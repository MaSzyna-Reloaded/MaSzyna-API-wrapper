#include "TrainCommand.hpp"

namespace godot {
    void TrainCommand::_bind_methods() {
        ClassDB::bind_method(D_METHOD("_init", "name", "callback"), &TrainCommand::_init, DEFVAL(StringName()), DEFVAL(Callable()));
        ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "name", "callback"), &TrainCommand::create);
        ClassDB::bind_method(D_METHOD("set_name", "name"), &TrainCommand::set_name);
        ClassDB::bind_method(D_METHOD("get_name"), &TrainCommand::get_name);
        ClassDB::bind_method(D_METHOD("set_callback", "callback"), &TrainCommand::set_callback);
        ClassDB::bind_method(D_METHOD("get_callback"), &TrainCommand::get_callback);

        ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "set_name", "get_name");
        ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "callback"), "set_callback", "get_callback");
    }

    void TrainCommand::_init(const StringName &p_name, const Callable &p_callback) {
        name = p_name;
        callback = p_callback;
    }

    Ref<TrainCommand> TrainCommand::create(const StringName &p_name, const Callable &p_callback) {
        Ref<TrainCommand> command;
        command.instantiate();
        command->_init(p_name, p_callback);
        return command;
    }

    void TrainCommand::set_name(const StringName &p_name) {
        name = p_name;
    }

    StringName TrainCommand::get_name() const {
        return name;
    }

    void TrainCommand::set_callback(const Callable &p_callback) {
        callback = p_callback;
    }

    Callable TrainCommand::get_callback() const {
        return callback;
    }
} // namespace godot
