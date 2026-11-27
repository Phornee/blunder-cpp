#include <string>
#include <algorithm>

std::string to_lower(const std::string& string_) {
    std::string result = string_;
    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
};  
