#include <blunder/primitives/mesh_object.hpp>

#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <utility>

#include <geom3/line3.hpp>
#include <geom3/triangle3.hpp>

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

std::vector<std::vector<double>> parse_points(const std::vector<std::string>& lines)
{
    std::vector<std::vector<double>> points;
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
                points.push_back({point[0], point[1], point[2]});
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
}

namespace {
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

        const std::vector<std::vector<double>> points = parse_points(lines);
        const std::vector<std::array<std::size_t, 3>> faces = parse_faces(lines);

        for (const auto& face : faces) {
            if (face[0] >= points.size() || face[1] >= points.size() || face[2] >= points.size()) {
                continue;
            }

            try {
                triangles.emplace_back(
                    Vector3(points[face[0]][0], points[face[0]][1], points[face[0]][2]),
                    Vector3(points[face[1]][0], points[face[1]][1], points[face[1]][2]),
                    Vector3(points[face[2]][0], points[face[2]][1], points[face[2]][2]));
            } catch (const std::invalid_argument&) {
                // Skip degenerate triangles.
            }
        }
    }

    explicit MeshShape(std::vector<Triangle3> source)
        : triangles(std::move(source))
    {
    }

    std::vector<Triangle3> transformed_triangles(const Matrix4& transform) const
    {
        std::vector<Triangle3> transformed;
        transformed.reserve(triangles.size());

        for (const Triangle3& triangle : triangles) {
            try {
                transformed.emplace_back(
                    transform * triangle.a,
                    transform * triangle.b,
                    transform * triangle.c);
            } catch (const std::invalid_argument&) {
                // Skip triangles that become degenerate after transformation.
            }
        }
        return transformed;
    }

    Vector3 barycentric(const Vector3& point) const override
    {
        for (const Triangle3& triangle : triangles) {
            const Vector3 result = triangle.barycentric(point);
            if (is_inside(result)) {
                return result;
            }
        }
        return Vector3{};
    }

    Vector3 normal(const Vector3& point) const override
    {
        for (const Triangle3& triangle : triangles) {
            if (is_inside(triangle.barycentric(point))) {
                return triangle.normal(point);
            }
        }
        return Vector3{};
    }

    std::vector<double> intersects(const Line3& line) const override
    {
        std::vector<double> intersections;
        for (const Triangle3& triangle : triangles) {
            const std::vector<double> triangle_hits = triangle.intersects(line);
            for (double lambda : triangle_hits) {
                if (lambda >= epsilon) {
                    intersections.push_back(lambda);
                }
            }
        }
        return intersections;
    }

    std::optional<double> closest_intersection(const Line3& line) const override
    {
        std::optional<double> closest;
        for (const Triangle3& triangle : triangles) {
            const std::optional<double> hit = triangle.closest_intersection(line);
            if (hit && *hit >= epsilon && (!closest || *hit < *closest)) {
                closest = hit;
            }
        }
        return closest;
    }

private:
    std::vector<Triangle3> triangles;
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

    transformed_shape_ = std::make_unique<MeshShape>(
        mesh->transformed_triangles(transform_));
}

Color MeshObject::color_at(const Vector3& point) const
{
    (void)point;
    return material().color();
}