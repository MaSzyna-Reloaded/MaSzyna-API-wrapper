#pragma once
#include <functional>
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

    /**
     * @brief A templated class that wraps a value and calls a callback function
     * whenever the value is changed.
     *
     * @tparam T The type of the variable to be observed.
     */
    template<typename T>
    class ObservableValue {
        private:
            T value;
            std::function<void(const T&)> callback;

        public:
            /**
             * ObservableValue watches any change to the variable it declares and calls the provided callback as it changes
             * @param p_initial_value Initial value
             * @param p_callback_fn Callback function taking no arguments
             */
            ObservableValue(const T& p_initial_value, std::function<void()> p_callback_fn)
                : value(p_initial_value), callback([fn = std::move(p_callback_fn)](const T&) { if (fn) { fn(); } }) {}

            /**
             * ObservableValue watches any change to the variable it declares and calls the provided callback as it changes
             * @param p_initial_value Initial value
             * @param p_callback_fn Callback function taking the new value (const T&)
             */
            ObservableValue(const T& p_initial_value, std::function<void(const T&)> p_callback_fn)
                : value(p_initial_value), callback(std::move(p_callback_fn)) {}

            // Core logic: Overload assignment operator - calls callback with one argument (new value)
            ObservableValue<T>& operator=(const T& p_new_value) {
                if (value != p_new_value) {
                    value = p_new_value;
                    if (callback) {
                        callback(p_new_value); // Call callback with the new value
                    }
                }
                return *this;
            }

            // Implicit conversion operator for easy reading
            explicit operator T() const {
                return value;
            }

            // Explicit getter
            const T& get() const {
                return value;
            }
    };
} // namespace libmaszyna::utils