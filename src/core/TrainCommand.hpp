#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {
    class TrainCommand : public RefCounted {
            GDCLASS(TrainCommand, RefCounted);

        private:
            StringName name;
            Callable callback;

        protected:
            static void _bind_methods();

        public:
            void _init(const StringName &p_name = StringName(), const Callable &p_callback = Callable());
            static Ref<TrainCommand> create(const StringName &p_name, const Callable &p_callback);

            void set_name(const StringName &p_name);
            StringName get_name() const;

            void set_callback(const Callable &p_callback);
            Callable get_callback() const;
    };

    inline Ref<TrainCommand> make_train_command(const StringName &name, const Callable &callback) {
        return TrainCommand::create(name, callback);
    }
} // namespace godot
