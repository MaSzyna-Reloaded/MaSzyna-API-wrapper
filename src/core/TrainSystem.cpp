#include "../core/TrainController.hpp"
#include "./TrainSystem.hpp"

namespace godot {
    const char *TrainSystem::TRAIN_LOG_UPDATED_SIGNAL = "train_log_updated";

    void TrainSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("register_train", "train_id", "train"), &TrainSystem::register_train);
        ClassDB::bind_method(D_METHOD("unregister_train", "train_id"), &TrainSystem::unregister_train);
        ClassDB::bind_method(D_METHOD("get_train_count"), &TrainSystem::get_train_count);
        ClassDB::bind_method(
                D_METHOD("get_config_property", "train_id", "property_name"), &TrainSystem::get_config_property);
        ClassDB::bind_method(
                D_METHOD("get_all_config_properties", "train_id"), &TrainSystem::get_all_config_properties);
        ClassDB::bind_method(
                D_METHOD("get_supported_config_properties", "train_id"), &TrainSystem::get_supported_config_properties);
        ClassDB::bind_method(
                D_METHOD("send_command", "train_id", "command", "p1", "p2"), &TrainSystem::send_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainSystem::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("is_command_supported", "command"), &TrainSystem::is_command_supported);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainSystem::get_supported_commands);
        ClassDB::bind_method(D_METHOD("get_registered_trains"), &TrainSystem::get_registered_trains);
        ClassDB::bind_method(
                D_METHOD("register_command", "train_id", "command", "callable"), &TrainSystem::register_command);
        ClassDB::bind_method(
                D_METHOD("unregister_command", "train_id", "command", "callable"), &TrainSystem::unregister_command);
        ClassDB::bind_method(D_METHOD("get_train_state", "train_id"), &TrainSystem::get_train_state);
        ClassDB::bind_method(D_METHOD("log", "train_id", "loglevel", "line"), &TrainSystem::log);
        ADD_SIGNAL(MethodInfo(
                TRAIN_LOG_UPDATED_SIGNAL, PropertyInfo(Variant::STRING, "train"),
                PropertyInfo(Variant::INT, "loglevel"), PropertyInfo(Variant::STRING, "line")));
    }

    int TrainSystem::get_train_count() const {
        return static_cast<int>(trains.size());
    }

    bool TrainSystem::is_train_registered(const String &train_id) const {
        const std::map<String, TrainController *>::const_iterator it = trains.find(train_id);

        return it != trains.end();
    }

    TrainController *TrainSystem::get_train(const String &train_id) {
        const std::map<String, TrainController *>::iterator it = trains.find(train_id);

        if (it == trains.end()) {
            return nullptr;
        }

        return it->second;
    }

    Dictionary TrainSystem::get_train_state(const String &train_id) {
        TrainController *train = get_train(train_id);

        if (train == nullptr) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            Dictionary empty;
            return empty;
        }
        return train->get_state();
    }

    Array TrainSystem::get_supported_config_properties(const String &train_id) {
        return get_all_config_properties(train_id).keys();
    }

    Dictionary TrainSystem::get_all_config_properties(const String &train_id) {
        const std::map<String, TrainController *>::iterator it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered in");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            return {};
        }
        const TrainController *train = it->second;
        return train->get_config();
    }

    Variant TrainSystem::get_config_property(const String &train_id, const String &property_name) {
        const Dictionary props = get_all_config_properties(train_id);
        return props.get(property_name, "");
    }

    void TrainSystem::log(const String &train_id, const LogSystem::LogLevel level, const String &line) {
        emit_signal("train_log_updated", train_id, level, line);
        LogSystem::get_instance()->log(level, vformat(String("%s: %s"), train_id, line));
    }

    void TrainSystem::register_train(const String &train_id, TrainController *train) {
        if (is_train_registered(train_id)) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is already registered!");
            UtilityFunctions::push_error("Train is already registered: ", train_id);
        } else {
            trains[train_id] = train;
            DEBUG("Registered train %s", train_id);
        }
    }

    void TrainSystem::register_command(const String &train_id, const String &command, const Callable &callback) {
        if (!is_train_registered(train_id)) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered in");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            return;
        }

        if (!commands.has(command)) {
            const Dictionary _trains;
            commands[command] = _trains;
        }


        if ((static_cast<Dictionary>(commands[command])).has(train_id)) {
            log(train_id, LogSystem::LogLevel::ERROR, "Command is already registered: " + command);
            UtilityFunctions::push_error("Command ", command, " is already registered for train ", train_id);
            return;
        }

        Dictionary _trains = commands[command];
        _trains[train_id] = callback;
    }

    void TrainSystem::unregister_command(const String &train_id, const String &command, const Callable &callback) {
        if (!is_train_registered(train_id)) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            return;
        }

        if (!is_command_supported(command)) {
            LogSystem::get_instance()->error("Cannot unregister unknown command: " + command);
            UtilityFunctions::push_error("Cannot unregister unknown command: ", command);
        }

        if (commands.has(command)) {
            Dictionary _trains = static_cast<Dictionary>(commands[command]);
            if (!_trains.has(train_id)) {
                log(train_id, LogSystem::LogLevel::ERROR, "Command is not registered: " + command);
                UtilityFunctions::push_error("Command ", command, " is not registered for train ", train_id);
                return;
            }
            _trains.erase(train_id);
            if (_trains.size() == 0) {
                commands.erase(command);
            }
        }
    }

    void TrainSystem::unregister_train(const String &train_id) {
        if (!is_train_registered(train_id)) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            return;
        }

        Array command_keys = commands.keys();
        Array commands_to_remove;

        for (int i = 0; i < command_keys.size(); i++) {
            String command = command_keys[i];
            Dictionary _trains = commands[command];
            if (_trains.has(train_id)) {
                _trains.erase(train_id);
            }
            if (_trains.size() == 0) {
                commands_to_remove.append(command);
            }
        }

        for (int i = 0; i < commands_to_remove.size(); i++) {
            commands.erase(commands_to_remove[i]);
        }

        trains.erase(train_id);
        DEBUG("Unregistered train %s", train_id);
    }

    bool TrainSystem::is_command_supported(const String &command) {
        return commands.has(command);
    }

    Array TrainSystem::get_supported_commands() {
        return commands.keys();
    }

    Array TrainSystem::get_registered_trains() {
        Array train_names;
        for (const auto &[first, second]: trains) {
            train_names.append(first);
        }
        return train_names;
    }

    void
    TrainSystem::send_command(const String &train_id, const String &command, const Variant &p1, const Variant &p2) {
        const std::map<String, TrainController *>::iterator it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, LogSystem::LogLevel::ERROR, "Train is not registered");
            UtilityFunctions::push_error("Train is not registered: ", train_id);
            return;
        }
        TrainController *train = it->second;
        if (is_command_supported(command)) {

            Dictionary _trains = static_cast<Dictionary>(commands[command]);

            if (!_trains.has(train_id)) {
                log(train_id, LogSystem::LogLevel::WARNING, "train cannot handle command: " + command);
                UtilityFunctions::push_warning("Train \"", train_id, "\" cannot handle command \"", command, "\"");
                return;
            }

            if (Callable c = _trains[train_id]; c.is_valid()) {
                Array args;
                int argc = static_cast<int>(c.get_argument_count());
                if (argc > 0) {
                    args.append(p1);
                }
                if (argc > 1) {
                    args.append(p2);
                }
                const Variant _call = c.callv(args);
#if DEBUG_MODE
                int arg_required = 0;
                if (p1.get_type() != Variant::NIL) {
                    arg_required++;
                }

                if (p2.get_type() != Variant::NIL) {
                    arg_required++;
                }
                log(train_id, LogSystem::LogLevel::DEBUG,
                    "received command " + command + "(" + String(", ").join(args) + ")");
                if (arg_required != argc) {
                    UtilityFunctions::push_warning(
                            "Method ", c.get_object(), "::", c.get_method(), " should handle ", arg_required,
                            " arguments, but it has ", argc);
                }
#endif
            } else {
                UtilityFunctions::push_error("Callable ", c, " is invalid");
            }
        } else {
            log(train_id, LogSystem::LogLevel::ERROR, "Unknown command: " + command);
            ERR_PRINT("[" + train_id + "] Unknown command: " + command);
        }

        train->update_state();
        train->emit_command_received_signal(command, p1, p2);
    }

    void TrainSystem::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        if (!is_command_supported(command)) {
            LogSystem::get_instance()->error("Unknown command: " + command);
            ERR_PRINT("Unknown command: " + command);
            return;
        }
        for (auto &[train_id, train]: trains) {
            send_command(train_id, command, p1, p2);
        }
    }
} // namespace godot
