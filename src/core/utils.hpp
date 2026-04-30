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
    template<typename Type_>
    Type_ interpolate(Type_ const &First, Type_ const &Second, double const Factor) {

        return static_cast<Type_>((First * (1.0 - Factor)) + (Second * Factor));
    }

    double random(double a, double b);
} // namespace libmaszyna::utils
