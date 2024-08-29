#pragma once
#include <string>

#define ENUM_WITH_COUNT(name, ...) enum class name { __VA_ARGS__, COUNT };

namespace libmaszyna {
    namespace utils {
        template<typename EnumType>
        std::string enum_to_string();
    }
} // namespace libmaszyna
