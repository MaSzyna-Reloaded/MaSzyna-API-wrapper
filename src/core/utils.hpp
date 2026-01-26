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
    template<typename Type_>
    Type_ interpolate(Type_ const &First, Type_ const &Second, double const Factor) {

        return static_cast<Type_>((First * (1.0 - Factor)) + (Second * Factor));
    }

    double random(double a, double b);

    /**
     * @brief A templated class that wraps a value and calls a callback function
     * whenever the value is changed.
     *
     * @tparam T The type of the variable to be observed.
     */
    template<typename T>
    class ObservableValue {
        private:
            T value_;
            std::function<void(const T&)> callback_;

        public:
            /**
             * ObservableValue watches any change to the variable it declares and calls the provided callback as it changes
             * @param initialValue Initial value
             * @param callbackFn Callback function taking no arguments
             */
            ObservableValue(const T& initialValue, std::function<void()> callbackFn)
                : value_(initialValue), callback_([fn = std::move(callbackFn)](const T&) { if (fn) fn(); }) {}

            /**
             * ObservableValue watches any change to the variable it declares and calls the provided callback as it changes
             * @param initialValue Initial value
             * @param callbackFn Callback function taking the new value (const T&)
             */
            ObservableValue(const T& initialValue, std::function<void(const T&)> callbackFn)
                : value_(initialValue), callback_(std::move(callbackFn)) {}

            // Core logic: Overload assignment operator - calls callback with one argument (new value)
            ObservableValue<T>& operator=(const T& newValue) {
                if (value_ != newValue) {
                    value_ = newValue;
                    if (callback_) {
                        callback_(newValue); // Call callback with the new value
                    }
                }
                return *this;
            }

            // Implicit conversion operator for easy reading
            explicit operator T() const {
                return value_;
            }

            // Explicit getter
            const T& get() const {
                return value_;
            }
    };
} // namespace libmaszyna::utils