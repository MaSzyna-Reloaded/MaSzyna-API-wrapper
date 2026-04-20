#include "../core/TrainController.hpp"
#include "./TrainSystem.hpp"

namespace godot {
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
    }

    TrainSystem::~TrainSystem() {
        commands.clear();
        trains.clear();
    }

    int TrainSystem::get_train_count() const {
        return static_cast<int>(trains.size());
    }

    bool TrainSystem::is_train_registered(const String &p_train_id) const {
        const std::map<String, TrainController *>::const_iterator it = trains.find(p_train_id);

        return it != trains.end();
    }

    TrainController *TrainSystem::get_train(const String &p_train_id) {
        const std::map<String, TrainController *>::iterator it = trains.find(p_train_id);

        if (it == trains.end()) {
            return nullptr;
        }

        return it->second;
    }

    Dictionary TrainSystem::get_train_state(const String &p_train_id) {
        TrainController *train = get_train(p_train_id);

        if (train == nullptr) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered");
            UtilityFunctions::push_error("Train is not registered: ", p_train_id);
            Dictionary empty;
            return empty;
        }
        return train->get_state();
    }

    Array TrainSystem::get_supported_config_properties(const String &p_train_id) {
        return get_all_config_properties(p_train_id).keys();
    }

    Dictionary TrainSystem::get_all_config_properties(const String &p_train_id) {
        const std::map<String, TrainController *>::iterator it = trains.find(p_train_id);

        if (it == trains.end()) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered in");
            UtilityFunctions::push_error("Train is not registered: ", p_train_id);
            return {};
        }
        const TrainController *train = it->second;
        return train->get_config();
    }

    Variant TrainSystem::get_config_property(const String &p_train_id, const String &p_property_name) {
        const Dictionary props = get_all_config_properties(p_train_id);
        return props.get(p_property_name, "");
    }

    void TrainSystem::log(const String &p_train_id, const GameLog::LogLevel p_level, const String &p_line) {
        GameLog::get_instance()->log(p_level, vformat(String("%s: %s"), p_train_id, p_line));
    }

    void TrainSystem::register_train(const String &p_train_id, TrainController *p_train) {
        if (is_train_registered(p_train_id)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is already registered");
        } else {
            trains[p_train_id] = p_train;
            log(p_train_id, GameLog::DEBUG, "Registered train");
        }
    }

    void TrainSystem::register_command(const String &p_train_id, const String &p_command, const Callable &p_callback) {
        if (!is_train_registered(p_train_id)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered in");
            return;
        }

        if (!commands.has(p_command)) {
            const Dictionary trains;
            commands[p_command] = trains;
        }


        if ((static_cast<Dictionary>(commands[p_command])).has(p_train_id)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Command is already registered: " + p_command);
            return;
        }

        Dictionary trains = commands[p_command];
        trains[p_train_id] = p_callback;
    }

    void TrainSystem::unregister_command(const String &p_train_id, const String &p_command, const Callable &p_callback) {
        if (!is_train_registered(p_train_id)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered");
            return;
        }

        if (!is_command_supported(p_command)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Cannot unregister unknown command: " + p_command);
            return;
        }

        if (commands.has(p_command)) {
            Dictionary trains = static_cast<Dictionary>(commands[p_command]);
            if (!trains.has(p_train_id)) {
                log(p_train_id, GameLog::LogLevel::ERROR, "Command is not registered: " + p_command);
                return;
            }
            trains.erase(p_train_id);
            if (trains.size() == 0) {
                commands.erase(p_command);
            }
        }
    }

    void TrainSystem::unregister_train(const String &p_train_id) {
        if (!is_train_registered(p_train_id)) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered");
            return;
        }

        Array command_keys = commands.keys();
        Array commands_to_remove;

        for (const auto & command_key : command_keys) {
            String command = command_key;
            Dictionary _trains = commands[command];
            if (_trains.has(p_train_id)) {
                _trains.erase(p_train_id);
            }
            if (_trains.size() == 0) {
                commands_to_remove.append(command);
            }
        }

        for (const auto & i : commands_to_remove) {
            commands.erase(i);
        }

        trains.erase(p_train_id);
        DEBUG("Unregistered train %s", p_train_id);
    }

    bool TrainSystem::is_command_supported(const String &p_command) {
        return commands.has(p_command);
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

    void TrainSystem::send_command(const String &p_train_id, const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        const std::map<String, TrainController *>::iterator it = trains.find(p_train_id);

        if (it == trains.end()) {
            log(p_train_id, GameLog::LogLevel::ERROR, "Train is not registered");
            return;
        }
        TrainController *train = it->second;
        if (is_command_supported(p_command)) {

            Dictionary trains = static_cast<Dictionary>(commands[p_command]);

            if (!trains.has(p_train_id)) {
                log(p_train_id, GameLog::LogLevel::WARNING, "train cannot handle command: " + p_command);
                return;
            }

            if (const Callable c = trains[p_train_id]; c.is_valid()) {
                Array args;
                const int argc = static_cast<int>(c.get_argument_count());
                if (argc > 0) {
                    args.append(p_p1);
                }
                if (argc > 1) {
                    args.append(p_p2);
                }
                const Variant call = c.callv(args);
#if DEBUG_MODE
                int arg_required = 0;
                if (p_p1.get_type() != Variant::NIL) {
                    arg_required++;
                }

                if (p_p2.get_type() != Variant::NIL) {
                    arg_required++;
                }

                log(p_train_id, GameLog::LogLevel::DEBUG,
                    "Received command " + p_command + "(" + String(", ").join(args) + ")");
                if (arg_required != argc) {
                    log(p_train_id, GameLog::LogLevel::WARNING,
                            "Method " + String(c.get_object()->get_class()) + "::" + String(c.get_method().get_basename()) + " should handle " + String::num_int64(arg_required) +
                            " arguments, but it has " + String::num_int64(arg_required));
                }
#endif
            } else {
                log(p_train_id, GameLog::LogLevel::ERROR, "Callable " + String(c.get_method().get_basename()) + " is invalid");
            }
        } else {
            log(p_train_id, GameLog::LogLevel::ERROR, "Unknown command: " + p_command);
            ERR_PRINT("[" + p_train_id + "] Unknown command: " + p_command);
        }

        train->update_state();
        train->emit_command_received_signal(p_command, p_p1, p_p2);
    }

    void TrainSystem::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        if (!is_command_supported(p_command)) {
            GameLog::get_instance()->error("Unknown command: " + p_command);
            ERR_PRINT("Unknown command: " + p_command);
            return;
        }
        for (auto &[train_id, train]: trains) {
            send_command(train_id, p_command, p_p1, p_p2);
        }
    }
} // namespace godot
