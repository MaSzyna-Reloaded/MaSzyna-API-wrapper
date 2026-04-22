#include "../core/TrainController.hpp"
#include "../core/RailVehicleModule.hpp"
#include "../core/TrainSystem.hpp"
#include <godot_cpp/core/math.hpp>

namespace godot {
    const char *TrainController::POWER_CHANGED_SIGNAL = "power_changed";
    const char *TrainController::COMMAND_RECEIVED = "command_received";
    const char *TrainController::RADIO_TOGGLED = "radio_toggled";
    const char *TrainController::RADIO_CHANNEL_CHANGED = "radio_channel_changed";

    void TrainController::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &TrainController::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainController::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &TrainController::register_command);
        ClassDB::bind_method(D_METHOD("unregister_command", "command", "callable"), &TrainController::unregister_command);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainController::get_supported_commands);
        ClassDB::bind_method(D_METHOD("refresh_command_registry"), &TrainController::refresh_command_registry);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &TrainController::battery);
        ClassDB::bind_method(D_METHOD("main_controller_increase", "step"), &TrainController::main_controller_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("main_controller_decrease", "step"), &TrainController::main_controller_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &TrainController::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &TrainController::direction_decrease);
        ClassDB::bind_method(D_METHOD("decouple", "relative_index"), &TrainController::decouple);
        ClassDB::bind_method(D_METHOD("radio", "enabled"), &TrainController::radio);
        ClassDB::bind_method(D_METHOD("radio_channel_set", "channel"), &TrainController::radio_channel_set);
        ClassDB::bind_method(D_METHOD("radio_channel_increase", "step"), &TrainController::radio_channel_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("radio_channel_decrease", "step"), &TrainController::radio_channel_decrease, DEFVAL(1));

        BIND_PROPERTY(Variant::STRING, "train_id", "train_id", &TrainController::set_train_id, &TrainController::get_train_id, "train_id");
        BIND_PROPERTY(Variant::FLOAT, "power", "power", &TrainController::set_power, &TrainController::get_power, "power");
        BIND_PROPERTY_W_HINT(Variant::FLOAT, "battery_voltage", "battery_voltage", &TrainController::set_battery_voltage, &TrainController::get_battery_voltage, "battery_voltage", PROPERTY_HINT_RANGE, "0,500,1");
        BIND_PROPERTY(Variant::INT, "radio_channel_min", "radio_channel/min", &TrainController::set_radio_channel_min, &TrainController::get_radio_channel_min, "radio_channel_min");
        BIND_PROPERTY(Variant::INT, "radio_channel_max", "radio_channel/max", &TrainController::set_radio_channel_max, &TrainController::get_radio_channel_max, "radio_channel_max");

        ADD_SIGNAL(MethodInfo(POWER_CHANGED_SIGNAL, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(RADIO_TOGGLED, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(RADIO_CHANNEL_CHANGED, PropertyInfo(Variant::INT, "channel")));
        ADD_SIGNAL(MethodInfo(
                COMMAND_RECEIVED, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"),
                PropertyInfo(Variant::NIL, "p2")));

        BIND_ENUM_CONSTANT(POWER_SOURCE_NOT_DEFINED);
        BIND_ENUM_CONSTANT(POWER_SOURCE_INTERNAL);
        BIND_ENUM_CONSTANT(POWER_SOURCE_TRANSDUCER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_GENERATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_ACCUMULATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_CURRENTCOLLECTOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_POWERCABLE);
        BIND_ENUM_CONSTANT(POWER_SOURCE_HEATER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_MAIN);

        BIND_ENUM_CONSTANT(POWER_TYPE_NONE);
        BIND_ENUM_CONSTANT(POWER_TYPE_BIO);
        BIND_ENUM_CONSTANT(POWER_TYPE_MECH);
        BIND_ENUM_CONSTANT(POWER_TYPE_ELECTRIC);
        BIND_ENUM_CONSTANT(POWER_TYPE_STEAM);
    }

    bool TrainController::can_host_commands() const {
        return true;
    }

    String TrainController::get_command_target_id() const {
        return train_id;
    }

    void TrainController::_do_update_internal_mover(TMoverParameters *mover) const {
        LegacyRailVehicle::_do_update_internal_mover(mover);
        mover->Power = power;
        mover->BatteryVoltage = battery_voltage;
        mover->NominalBatteryVoltage = static_cast<float>(battery_voltage);
    }

    void TrainController::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        LegacyRailVehicle::_do_fetch_state_from_mover(mover, state);
        state["cabin"] = mover->CabActive;
        state["cabin_controleable"] = mover->IsCabMaster();
        state["cabin_occupied"] = mover->CabOccupied;
        state["battery_enabled"] = mover->Battery;
        state["battery_voltage"] = mover->BatteryVoltage;
        state["power24_voltage"] = mover->Power24vVoltage;
        state["power24_available"] = mover->Power24vIsAvailable;
        state["power110_available"] = mover->Power110vIsAvailable;
        state["current0"] = mover->ShowCurrent(0);
        state["current1"] = mover->ShowCurrent(1);
        state["current2"] = mover->ShowCurrent(2);
        state["relay_novolt"] = mover->NoVoltRelay;
        state["relay_overvoltage"] = mover->OvervoltageRelay;
        state["relay_ground"] = mover->GroundRelay;
        state["controller_second_position"] = mover->ScndCtrlPos;
        state["controller_main_position"] = mover->MainCtrlPos;
        state["radio_enabled"] = mover->Radio;
        state["radio_powered"] = mover->Radio && (mover->Power24vIsAvailable || mover->Power110vIsAvailable);
        state["radio_channel"] = radio_channel;
    }

    void TrainController::refresh_runtime_signals() {
        Dictionary state = LegacyRailVehicle::get_state();
        const bool new_is_powered = (state.get("power24_available", false) || state.get("power110_available", false));
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered;
            emit_signal(POWER_CHANGED_SIGNAL, prev_is_powered);
        }

        const bool new_radio_enabled = state.get("radio_enabled", false) && new_is_powered;
        if (prev_radio_enabled != new_radio_enabled) {
            prev_radio_enabled = new_radio_enabled;
            emit_signal(RADIO_TOGGLED, new_radio_enabled);
        }

        const int new_radio_channel = state.get("radio_channel", 0);
        if (prev_radio_channel != new_radio_channel) {
            prev_radio_channel = new_radio_channel;
            emit_signal(RADIO_CHANNEL_CHANGED, new_radio_channel);
        }
    }

    void TrainController::_initialize() {
        LegacyRailVehicle::_initialize();
        TrainSystem::get_instance()->register_train(train_id, this);
    }

    void TrainController::_deinitialize() {
        TrainSystem::get_instance()->unregister_train(train_id);
        LegacyRailVehicle::_deinitialize();
    }

    void TrainController::_update(const double delta) {
        LegacyRailVehicle::_update(delta);
        refresh_runtime_signals();
    }

    void TrainController::_notification_after_mover_ready() {
        refresh_runtime_signals();
    }

    void TrainController::register_command(const String &command, const Callable &callable) {
        TrainSystem::get_instance()->register_command(train_id, command, callable);
    }

    void TrainController::unregister_command(const String &command, const Callable &callable) {
        TrainSystem::get_instance()->unregister_command(train_id, command, callable);
    }

    Dictionary TrainController::get_supported_commands() {
        Dictionary commands;
        commands["battery"] = Callable(this, "battery");
        commands["main_controller_increase"] = Callable(this, "main_controller_increase");
        commands["main_controller_decrease"] = Callable(this, "main_controller_decrease");
        commands["direction_increase"] = Callable(this, "direction_increase");
        commands["direction_decrease"] = Callable(this, "direction_decrease");
        commands["decouple"] = Callable(this, "decouple");
        commands["radio"] = Callable(this, "radio");
        commands["radio_channel_set"] = Callable(this, "radio_channel_set");
        commands["radio_channel_increase"] = Callable(this, "radio_channel_increase");
        commands["radio_channel_decrease"] = Callable(this, "radio_channel_decrease");

        const Array vehicle_modules = get_rail_vehicle_modules();
        for (int index = 0; index < vehicle_modules.size(); ++index) {
            auto *module = Object::cast_to<RailVehicleModule>(vehicle_modules[index]);
            if (module == nullptr) {
                continue;
            }

            const Dictionary module_commands = module->get_supported_commands();
            const Array keys = module_commands.keys();
            for (int key_index = 0; key_index < keys.size(); ++key_index) {
                const String command_name = keys[key_index];
                if (commands.has(command_name)) {
                    TrainSystem::get_instance()->log(
                            train_id, GameLog::LogLevel::ERROR, "Duplicate command ignored: " + command_name);
                    continue;
                }
                commands[command_name] = module_commands[command_name];
            }
        }

        return commands;
    }

    void TrainController::refresh_command_registry() {
        TrainSystem::get_instance()->refresh_train_commands(train_id);
    }

    void TrainController::emit_command_received_signal(const String &command, const Variant &p1, const Variant &p2) {
        emit_signal(COMMAND_RECEIVED, command, p1, p2);
    }

    void TrainController::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        TrainSystem::get_instance()->broadcast_command(command, p1, p2);
    }

    void TrainController::send_command(const StringName &command, const Variant &p1, const Variant &p2) const {
        TrainSystem::get_instance()->send_command(train_id, String(command), p1, p2);
    }

    void TrainController::battery(const bool p_enabled) const {
        get_mover()->BatterySwitch(p_enabled);
    }

    void TrainController::main_controller_increase(const int p_step) const {
        get_mover()->IncMainCtrl(p_step > 0 ? p_step : 1);
    }

    void TrainController::main_controller_decrease(const int p_step) const {
        get_mover()->DecMainCtrl(p_step > 0 ? p_step : 1);
    }

    void TrainController::direction_increase() const {
        get_mover()->DirectionForward();
    }

    void TrainController::direction_decrease() const {
        get_mover()->DirectionBackward();
    }

    void TrainController::decouple(const int relative_index) {
        if (RailVehicle::decouple(relative_index) == nullptr) {
            TrainSystem::get_instance()->log(
                    train_id,
                    GameLog::LogLevel::ERROR,
                    "decouple failed for relative_index=" + String::num_int64(relative_index));
        }
    }

    void TrainController::radio(const bool p_enabled) {
        get_mover()->Radio = p_enabled;
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_set(const int p_channel) {
        radio_channel = Math::clamp(p_channel, radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_increase(const int p_step) {
        radio_channel = Math::clamp(radio_channel + (p_step > 0 ? p_step : 1), radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }

    void TrainController::radio_channel_decrease(const int p_step) {
        radio_channel = Math::clamp(radio_channel - (p_step != 0 ? p_step : 1), radio_channel_min, radio_channel_max);
        refresh_runtime_signals();
    }
} // namespace godot
