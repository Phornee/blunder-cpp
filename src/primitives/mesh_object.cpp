#include <blunder/primitives/mesh_object.hpp>

#include <blunder/aabb.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <geom3/line3.hpp>

namespace {
std::string trim(const std::string& text)
{
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(begin, end - begin);
}

void strip_comment(std::string& line)
{
    const std::size_t comment = line.find('#');
    if (comment != std::string::npos) {
        line.resize(comment);
    }
}

bool is_inside(const Vector3& barycentric)
{
    return barycentric.x >= -epsilon && barycentric.x <= 1.0 + epsilon
        && barycentric.y >= -epsilon && barycentric.y <= 1.0 + epsilon
        && barycentric.z >= -epsilon && barycentric.z <= 1.0 + epsilon;
}

bool starts_section(const std::string& line, const std::string& name)
{
    const std::string key = trim(line);
    return key == name + ':' || key.rfind(name + ":", 0) == 0;
}

std::vector<std::string> parse_list(const std::string& line)
{
    std::string text = trim(line);
    if (!text.empty() && text.front() == '-') {
        text = trim(text.substr(1));
    }

    if (text.size() >= 2 && text.front() == '[' && text.back() == ']') {
        text = text.substr(1, text.size() - 2);
    }

    const std::string separators = ", ";
    std::vector<std::string> values;
    std::size_t start = 0;
    while (start < text.size()) {
        while (start < text.size()
               && (text[start] == ',' || std::isspace(static_cast<unsigned char>(text[start])))) {
            ++start;
        }
        if (start >= text.size()) {
            break;
        }

        const std::size_t comma = text.find_first_of(separators, start);
        if (comma == std::string::npos) {
            values.push_back(trim(text.substr(start)));
            break;
        }
        values.push_back(trim(text.substr(start, comma - start)));
        start = comma + 1;
    }
    return values;
}

bool parse_inline_numbers(const std::string& line, std::vector<double>& numbers)
{
    std::string text = trim(line);
    if (!text.empty() && text.front() == '-') {
        text = trim(text.substr(1));
    }
    if (text.size() >= 2 && text.front() == '[' && text.back() == ']') {
        text = text.substr(1, text.size() - 2);
    }

    std::size_t position = 0;
    while (position < text.size()) {
        while (position < text.size()
               && (text[position] == ',' || std::isspace(static_cast<unsigned char>(text[position])))) {
            ++position;
        }
        if (position >= text.size()) {
            break;
        }

        char* end = nullptr;
        const double value = std::strtod(text.c_str() + position, &end);
        if (end == text.c_str() + position) {
            return false;
        }
        numbers.push_back(value);
        position = static_cast<std::size_t>(end - text.c_str());
    }
    return !numbers.empty();
}

std::size_t point_index(const std::string& text)
{
    std::size_t parsed = 0;
    try {
        parsed = std::stoull(trim(text));
    } catch (...) {
        throw std::invalid_argument("A mesh face contains a non-integer point index");
    }
    return parsed;
}

std::vector<Vector3> parse_points(const std::vector<std::string>& lines)
{
    std::vector<Vector3> points;
    bool in_section = false;

    for (std::string raw : lines) {
        strip_comment(raw);
        const std::string line = trim(raw);
        if (line.empty()) {
            continue;
        }
        if (starts_section(line, "points")) {
            in_section = true;
            continue;
        }
        if (starts_section(line, "faces")) {
            break;
        }
        if (in_section) {
            std::string data = line;
            if (!data.empty() && data.front() == '-') {
                data = trim(data.substr(1));
            }

            const std::size_t colon = data.find(':');
            if (colon != std::string::npos) {
                data = trim(data.substr(colon + 1));
            }

            std::vector<double> point;
            if (parse_inline_numbers(data, point) && point.size() >= 3) {
                points.emplace_back(point[0], point[1], point[2]);
            }
        }
    }
    return points;
}

std::vector<std::array<std::size_t, 3>> parse_faces(const std::vector<std::string>& lines)
{
    std::vector<std::array<std::size_t, 3>> faces;
    bool in_section = false;

    for (std::string raw : lines) {
        strip_comment(raw);
        const std::string line = trim(raw);
        if (line.empty()) {
            continue;
        }
        if (starts_section(line, "faces")) {
            in_section = true;
            continue;
        }
        if (in_section) {
            std::vector<std::string> values = parse_list(line);
            if (values.size() >= 3) {
                faces.push_back({
                    point_index(values[0]),
                    point_index(values[1]),
                    point_index(values[2])});
            }
        }
    }
    return faces;
}

struct ParsedMesh
{
    std::vector<Vector3> points;
    std::vector<std::array<std::size_t, 3>> faces;
};

std::size_t obj_vertex_index(
    const std::string& reference,
    std::size_t vertex_count,
    std::size_t line_number)
{
    const std::size_t slash = reference.find('/');
    const std::string index_text = reference.substr(0, slash);
    long long index = 0;
    std::size_t parsed_length = 0;
    try {
        index = std::stoll(index_text, &parsed_length);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument(
            "Invalid OBJ vertex index at line " + std::to_string(line_number));
    } catch (const std::out_of_range&) {
        throw std::invalid_argument(
            "OBJ vertex index is out of range at line " + std::to_string(line_number));
    }

    if (parsed_length != index_text.size() || index == 0) {
        throw std::invalid_argument(
            "Invalid OBJ vertex index at line " + std::to_string(line_number));
    }

    const long long count = static_cast<long long>(vertex_count);
    const long long resolved_index = index > 0 ? index - 1 : count + index;
    if (resolved_index < 0 || resolved_index >= count) {
        throw std::invalid_argument(
            "OBJ vertex index is out of range at line " + std::to_string(line_number));
    }
    return static_cast<std::size_t>(resolved_index);
}

ParsedMesh parse_obj(const std::vector<std::string>& lines)
{
    ParsedMesh mesh;

    for (std::size_t line_index = 0; line_index < lines.size(); ++line_index) {
        std::string raw = lines[line_index];
        strip_comment(raw);
        std::istringstream input(raw);
        std::string directive;
        input >> directive;

        if (directive == "v") {
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            if (!(input >> x >> y >> z)) {
                throw std::invalid_argument(
                    "Invalid OBJ vertex at line " + std::to_string(line_index + 1));
            }
            mesh.points.emplace_back(x, y, z);
        } else if (directive == "f") {
            std::vector<std::size_t> face;
            std::string reference;
            while (input >> reference) {
                face.push_back(
                    obj_vertex_index(reference, mesh.points.size(), line_index + 1));
            }

            if (face.size() < 3) {
                throw std::invalid_argument(
                    "OBJ face has fewer than three vertices at line "
                    + std::to_string(line_index + 1));
            }
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                mesh.faces.push_back({face[0], face[i], face[i + 1]});
            }
        }
    }

    if (mesh.points.empty() || mesh.faces.empty()) {
        throw std::invalid_argument("OBJ mesh must contain vertices and faces");
    }
    return mesh;
}
}

namespace {
struct Vertex
{
    explicit Vertex(const Vector3& position)
        : position(position)
    {
    }

    Vector3 position;
};

class MeshTriangle
{
public:
    MeshTriangle(Vertex& a, Vertex& b, Vertex& c)
        : a_(&a),
          b_(&b),
          c_(&c),
          ab_(b.position - a.position),
          ac_(c.position - a.position),
          d00_(ab_.dot(ab_)),
          d01_(ab_.dot(ac_)),
          d11_(ac_.dot(ac_)),
          denom_(d00_ * d11_ - d01_ * d01_),
          normal_{0.0, 0.0, 0.0}
    {
        const Vector3 cross = ab_.cross(ac_);
        if (cross.is_zero()) {
            throw std::invalid_argument("A triangle must have three non-collinear vertices");
        }
        normal_ = cross.normalized();
    }

    const Vertex& a() const { return *a_; }
    const Vertex& b() const { return *b_; }
    const Vertex& c() const { return *c_; }

    Vector3 barycentric(const Vector3& point) const
    {
        const Vector3 ap = point - a_->position;
        const double d20 = ap.dot(ab_);
        const double d21 = ap.dot(ac_);
        const double bar_b = (d11_ * d20 - d01_ * d21) / denom_;
        const double bar_c = (d00_ * d21 - d01_ * d20) / denom_;
        return {1.0 - bar_b - bar_c, bar_b, bar_c};
    }

    bool contains(const Vector3& point) const
    {
        return std::abs(normal_.dot(point - a_->position)) <= epsilon
            && is_inside(barycentric(point));
    }

    Vector3 normal() const
    {
        return normal_;
    }

    std::optional<Hit> closest_intersection(const Line3& line) const
    {
        const double denominator = normal_.dot(line.v);
        if (std::abs(denominator) < epsilon) {
            return std::nullopt;
        }

        const double lambda = normal_.dot(a_->position - line.a) / denominator;
        const Vector3 point = line.calc_point(lambda);
        const Vector3 coordinates = barycentric(point);
        if (coordinates.x < 0.0 || coordinates.x > 1.0
            || coordinates.y < 0.0 || coordinates.y > 1.0
            || coordinates.z < 0.0 || coordinates.z > 1.0) {
            return std::nullopt;
        }
        return Hit{lambda, nullptr, normal_, coordinates};
    }

private:
    Vertex* a_;
    Vertex* b_;
    Vertex* c_;
    Vector3 ab_;
    Vector3 ac_;
    double d00_;
    double d01_;
    double d11_;
    double denom_;
    Vector3 normal_;
};

class MeshShape final : public Shape
{
public:
    explicit MeshShape(const std::string& file)
    {
        std::ifstream stream(file);
        if (!stream) {
            throw std::runtime_error("Unable to open mesh file: " + file);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        std::vector<Vector3> points;
        std::vector<std::array<std::size_t, 3>> faces;
        std::string extension = std::filesystem::path(file).extension().string();
        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
        if (extension == ".obj") {
            ParsedMesh mesh = parse_obj(lines);
            points = std::move(mesh.points);
            faces = std::move(mesh.faces);
        } else {
            points = parse_points(lines);
            faces = parse_faces(lines);
        }

        vertices_.reserve(points.size());
        for (const Vector3& point : points) {
            vertices_.push_back(std::make_unique<Vertex>(point));
        }

        for (const auto& face : faces) {
            if (face[0] >= vertices_.size()
                || face[1] >= vertices_.size()
                || face[2] >= vertices_.size()) {
                continue;
            }

            try {
                triangles_.emplace_back(
                    *vertices_[face[0]],
                    *vertices_[face[1]],
                    *vertices_[face[2]]);
            } catch (const std::invalid_argument&) {
                // Skip degenerate triangles.
            }
        }
    }

    MeshShape(
        std::vector<std::unique_ptr<Vertex>> vertices,
        std::vector<MeshTriangle> triangles)
        : vertices_(std::move(vertices)),
          triangles_(std::move(triangles))
    {
    }

    std::vector<Vector3> vertex_positions() const
    {
        std::vector<Vector3> positions;
        positions.reserve(vertices_.size());
        for (const std::unique_ptr<Vertex>& vertex : vertices_) {
            positions.push_back(vertex->position);
        }
        return positions;
    }

    std::unique_ptr<MeshShape> transformed(const Matrix4& transform) const
    {
        std::vector<std::unique_ptr<Vertex>> transformed_vertices;
        transformed_vertices.reserve(vertices_.size());
        std::unordered_map<const Vertex*, Vertex*> vertex_map;
        vertex_map.reserve(vertices_.size());

        for (const std::unique_ptr<Vertex>& vertex : vertices_) {
            auto transformed_vertex =
                std::make_unique<Vertex>(transform * vertex->position);
            vertex_map.emplace(vertex.get(), transformed_vertex.get());
            transformed_vertices.push_back(std::move(transformed_vertex));
        }
transform the normals?
        std::vector<MeshTriangle> transformed_triangles;
        transformed_triangles.reserve(triangles_.size());
        for (const MeshTriangle& triangle : triangles_) {
            try {
                transformed_triangles.emplace_back(
                    *vertex_map.at(&triangle.a()),
                    *vertex_map.at(&triangle.b()),
                    *vertex_map.at(&triangle.c()));
            } catch (const std::invalid_argument&) {
                // Skip triangles that become degenerate after transformation.
            }
        }

        return std::make_unique<MeshShape>(
            std::move(transformed_vertices),
            std::move(transformed_triangles));
    }

    Vector3 barycentric(const Vector3& point) const override
    {
        for (const MeshTriangle& triangle : triangles_) {
            if (triangle.contains(point)) {
                return triangle.barycentric(point);
            }
        }
        return Vector3::zero;
    }

    Vector3 normal(const Vector3& point) const override
    {
        for (const MeshTriangle& triangle : triangles_) {
            if (triangle.contains(point)) {
                return triangle.normal();
            }
        }
        return Vector3::zero;
    }

    std::vector<Hit> intersects(const Line3& line) const override
    {
        std::vector<Hit> intersections;
        for (const MeshTriangle& triangle : triangles_) {
            const std::optional<Hit> hit = triangle.closest_intersection(line);
            if (hit && hit->lambda >= epsilon) {
                intersections.push_back(*hit);
            }
        }
        return intersections;
    }

    std::optional<Hit> closest_intersection(const Line3& line) const override
    {
        std::optional<Hit> closest;
        for (const MeshTriangle& triangle : triangles_) {
            const std::optional<Hit> hit = triangle.closest_intersection(line);
            if (hit && hit->lambda >= epsilon
                && (!closest || hit->lambda < closest->lambda)) {
                closest = hit;
            }
        }
        return closest;
    }

private:
    std::vector<std::unique_ptr<Vertex>> vertices_;
    std::vector<MeshTriangle> triangles_;
};
}

MeshObject::MeshObject(
    std::string name,
    const Material& material,
    const std::string& file)
    : Object(
          std::move(name),
          material,
          std::make_unique<MeshShape>(file)),
      file_(file)
{
    transform_shape();
}

const std::string& MeshObject::filename() const
{
    return file_;
}

void MeshObject::transform_shape() const
{
    MeshShape* mesh = dynamic_cast<MeshShape*>(shape_.get());
    if (mesh == nullptr) {
        throw std::logic_error("MeshObject must contain a MeshShape");
    }

    std::unique_ptr<MeshShape> transformed_mesh = mesh->transformed(transform_);
    const std::vector<Vector3> vertices = transformed_mesh->vertex_positions();
    bounding_box_.emplace(vertices.front(), vertices.front());
    bounding_box_->set_from_vertices(vertices);
    transformed_shape_ = std::move(transformed_mesh);
}

Color MeshObject::albedo_at(const Vector3& point) const
{
    (void)point;
    return material().color();
}