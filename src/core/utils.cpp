#include <sstream>
#include <string>
#include <vector>

#include "utils.hpp"

namespace libmaszyna {
    namespace utils {
        template<typename EnumType>
        std::string enum_to_string() {
            std::vector<std::string> enum_names;
            enum_names.reserve(static_cast<int>(EnumType::COUNT)); //Pre allocating the required capacity before the loop
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
    } // namespace utils



    std::vector<std::string> Utils::split(const std::string& s, const std::string& delimiter) {
        size_t pos_start = 0, pos_end;
        const size_t delim_len = delimiter.length();
        std::vector<std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            std::string token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back (token);
        }

        res.push_back (s.substr (pos_start));
        return res;
    }
} // namespace libmaszyna
