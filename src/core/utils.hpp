#pragma once
#include <string>

#define ENUM_WITH_COUNT(name, ...) enum class name { __VA_ARGS__, COUNT };

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif
#ifndef M_1_PI
#define M_1_PI 0.318309886183790671538
#endif
#ifndef M_2_PI
#define M_2_PI 0.636619772367581343076
#endif

namespace libmaszyna::utils {
    template<typename EnumType>
    std::string enum_to_string();
    template<typename Type>
    Type interpolate(Type const &p_first, Type const &p_second, double const p_factor) {
        return static_cast<Type>((p_first * (1.0 - p_factor)) + (p_second * p_factor));
    }

    double random(double p_a, double p_b);
} // namespace libmaszyna::utils
