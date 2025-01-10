#include "utils.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace libmaszyna::utils {
    template<typename EnumType>
    std::string enum_to_string() {
        std::vector<std::string> enum_names;
        enum_names.reserve(static_cast<int>(EnumType::COUNT)); // Pre allocating the required capacity before the loop
        for (int i = 0; i < static_cast<int>(EnumType::COUNT); ++i) {
            enum_names.push_back(std::to_string(i)); // Placeholder, replace it with actual enum name extraction
        }
        std::stringstream ss;
        for (size_t i = 0; i < enum_names.size(); ++i) {
            ss << enum_names[i];
            if (i < enum_names.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }
} // namespace libmaszyna::utils
