#pragma once
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <functional>

// Helper to wrap C++ void() lambdas/functors into Godot Callables.
namespace godot {
    using LambdaType = std::function<void()>;

    class LambdaCallable : public CallableCustom {
        private:
            LambdaType lambda_;
            static bool _cmp_equal(const CallableCustom *p_a, const CallableCustom *p_b) {
                return p_a == p_b;
            }
            static bool _cmp_less(const CallableCustom *p_a, const CallableCustom *p_b) {
                return p_a < p_b;
            };

        public:
            explicit LambdaCallable(LambdaType p_lambda) : lambda_(std::move(p_lambda)) {}

            uint32_t hash() const override {
                return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this));
            }

            String get_as_text() const override {
                return String("LambdaCallable<void()>");
            }

            CompareEqualFunc get_compare_equal_func() const override {
                return &_cmp_equal;
            }

            CompareLessFunc get_compare_less_func() const override {
                return &_cmp_less;
            }

            bool is_valid() const override {
                return static_cast<bool>(lambda_);
            }

            ObjectID get_object() const override {
                return ObjectID(); // No bound object
            }

            void call(const Variant **p_arguments, const int p_argcount, Variant &r_return_value, GDExtensionCallError &r_call_error) const override {
                if (p_argcount != 0) {
                    r_call_error.error = GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS;
                    r_call_error.argument = 0;
                    r_call_error.expected = 0;
                    r_return_value = Variant();
                    return;
                }

                if (lambda_) {
                    lambda_();
                }

                r_return_value = Variant(); // void return
                r_call_error.error = GDEXTENSION_CALL_OK;
                r_call_error.argument = 0;
                r_call_error.expected = 0;
            }
    };

    // Factory helper to create a Godot Callable from a C++ lambda/functor of signature void().
    inline Callable make_lambda_callable(LambdaType p_lambda) {
        return Callable(memnew(LambdaCallable(std::move(p_lambda))));
    }

    // Convenience template to accept any invocable convertible to void().
    template <typename F>
    inline Callable make_callable(F &&f) {
        return make_lambda_callable(LambdaType(std::forward<F>(f)));
    }
} //namespace godot