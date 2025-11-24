#include "../core/TrainNode.hpp"
#include "../core/TrainPart.hpp"
#include "../core/TrainSystem.hpp"
#include "../engines/TrainEngine.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *TrainNode::MOVER_CONFIG_CHANGED_SIGNAL = "mover_config_changed";
    const char *TrainNode::MOVER_INITIALIZED_SIGNAL = "mover_initialized";
    const char *TrainNode::POWER_CHANGED_SIGNAL = "power_changed";
    const char *TrainNode::COMMAND_RECEIVED = "command_received";
    const char *TrainNode::RADIO_TOGGLED = "radio_toggled";
    const char *TrainNode::RADIO_CHANNEL_CHANGED = "radio_channel_changed";
    const char *TrainNode::CONFIG_CHANGED = "config_changed";

    void TrainNode::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &TrainNode::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &TrainNode::get_config);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_state");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "config", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_config");

        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &TrainNode::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));

        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainNode::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));


        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &TrainNode::register_command);
        ClassDB::bind_method(
                D_METHOD("unregister_command", "command", "callable"), &TrainNode::unregister_command);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &TrainNode::battery);
        ClassDB::bind_method(
                D_METHOD("main_controller_increase", "step"), &TrainNode::main_controller_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("main_controller_decrease", "step"), &TrainNode::main_controller_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &TrainNode::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &TrainNode::direction_decrease);
        ClassDB::bind_method(D_METHOD("radio", "enabled"), &TrainNode::radio);
        ClassDB::bind_method(
                D_METHOD("radio_channel_increase", "step"), &TrainNode::radio_channel_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("radio_channel_decrease", "step"), &TrainNode::radio_channel_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainNode::update_mover);
        ClassDB::bind_method(D_METHOD("update_state"), &TrainNode::update_state);
        ClassDB::bind_method(D_METHOD("update_config"), &TrainNode::update_config);

        BIND_PROPERTY(Variant::STRING, "train_id", "train_id", &TrainNode::set_train_id, &TrainNode::get_train_id, "train_id");
        BIND_PROPERTY(Variant::STRING, "type_name", "type_name", &TrainNode::set_type_name, &TrainNode::get_type_name, "type_name");
        BIND_PROPERTY(Variant::FLOAT, "mass", "mass", &TrainNode::set_mass, &TrainNode::get_mass, "mass");
        BIND_PROPERTY(Variant::FLOAT, "power", "power", &TrainNode::set_power, &TrainNode::get_power, "power");
        BIND_PROPERTY(Variant::FLOAT, "max_velocity", "max_velocity", &TrainNode::set_max_velocity, &TrainNode::get_max_velocity, "max_velocity");
        BIND_PROPERTY(Variant::STRING, "axle_arrangement", "axle_arrangement", &TrainNode::set_axle_arrangement, &TrainNode::get_axle_arrangement, "axle_arrangement");
        BIND_PROPERTY(Variant::INT, "radio_channel_min", "radio_channel/min", &TrainNode::set_radio_channel_min, &TrainNode::get_radio_channel_min, "radio_channel_min");
        BIND_PROPERTY(Variant::INT, "radio_channel_max", "radio_channel/max", &TrainNode::set_radio_channel_max, &TrainNode::get_radio_channel_max, "radio_channel_max");
        /* FIXME: move to TrainPower section? */
        BIND_PROPERTY_W_HINT(Variant::FLOAT, "battery_voltage", "battery_voltage", &TrainNode::set_battery_voltage, &TrainNode::get_battery_voltage, "battery_voltage", PROPERTY_HINT_RANGE, "0,500,1");

        ADD_SIGNAL(MethodInfo(MOVER_CONFIG_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(MOVER_INITIALIZED_SIGNAL));
        ADD_SIGNAL(MethodInfo(POWER_CHANGED_SIGNAL, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(RADIO_TOGGLED, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(RADIO_CHANNEL_CHANGED, PropertyInfo(Variant::INT, "channel")));
        ADD_SIGNAL(MethodInfo(CONFIG_CHANGED));
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

    TMoverParameters *TrainNode::get_mover() const {
        return mover;
    }

    void TrainNode::initialize_mover() {
        const auto initial_vel = this->initial_velocity;
        const auto _type_name = std::string(type_name.utf8().ptr());
        const auto name = std::string(this->get_name().left(this->get_name().length()).utf8().ptr());
        mover = std::make_unique<TMoverParameters>(initial_vel, _type_name, name, this->cabin_number).release();

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(true, 0); // FIXME: brakujace parametery

        /* CheckLocomotiveParameters() will reset some parameters, so the changes
         * must be applied second time */

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        /* FIXME: remove test data */
        mover->CabActive = 1;
        mover->CabMaster = true;
        mover->CabOccupied = 1;
        mover->AutomaticCabActivation = true;
        mover->CabActivisationAuto();
        mover->CabActivisation();

        /* switch_physics() raczej trzeba zostawic */
        mover->switch_physics(true);

        DEBUG("[MaSzyna::TMoverParameters] Mover initialized successfully");
        emit_signal(MOVER_INITIALIZED_SIGNAL);
    }

    void TrainNode::register_command(const String &command, const Callable &callable) {
        TrainSystem::get_instance()->register_command(train_id, command, callable);
    }

    void TrainNode::unregister_command(const String &command, const Callable &callable) {
        TrainSystem::get_instance()->unregister_command(train_id, command, callable);
    }

    void TrainNode::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE:
                TrainSystem::get_instance()->register_train(train_id, this);
                register_command("battery", Callable(this, "battery"));
                register_command("main_controller_increase", Callable(this, "main_controller_increase"));
                register_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
                register_command("direction_increase", Callable(this, "direction_increase"));
                register_command("direction_decrease", Callable(this, "direction_decrease"));
                register_command("radio", Callable(this, "radio"));
                register_command("radio_channel_set", Callable(this, "radio_channel_set"));
                register_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
                register_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
                break;
            case NOTIFICATION_EXIT_TREE:
                unregister_command("battery", Callable(this, "battery"));
                unregister_command("main_controller_increase", Callable(this, "main_controller_increase"));
                unregister_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
                unregister_command("direction_increase", Callable(this, "direction_increase"));
                unregister_command("direction_decrease", Callable(this, "direction_decrease"));
                unregister_command("radio", Callable(this, "radio"));
                unregister_command("radio_channel_set", Callable(this, "radio_channel_set"));
                unregister_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
                unregister_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
                TrainSystem::get_instance()->unregister_train(train_id);
                break;
            case NOTIFICATION_READY:
                initialize_mover();
                update_state();
                DEBUG("TrainNode::_ready() signals connected to train parts");

                emit_signal(POWER_CHANGED_SIGNAL, prev_is_powered);
                emit_signal(RADIO_CHANNEL_CHANGED, prev_radio_channel);
                break;
            default:;
        }
    }

    void TrainNode::_update_mover_config_if_dirty() {
        if (_dirty) {
            /* update all train parts
             */
            emit_signal(MOVER_CONFIG_CHANGED_SIGNAL);

            _dirty = false;
            _dirty_prop = true; // sforsowanie odswiezenia stanu lokalnych propsow
        }

        if (_dirty_prop) {
            update_mover();
            _dirty_prop = false;
        }
    }

    void TrainNode::_process_mover(const double delta) {
        TLocation mock_location;
        TRotation mock_rotation;
        mover->ComputeTotalForce(delta);
        // mover->compute_movement_(delta);
        mover->ComputeMovement(
                delta, delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mock_location,
                mock_rotation);

        _handle_mover_update();
    }

    void TrainNode::update_state() {
        _handle_mover_update();
    }

    void TrainNode::_handle_mover_update() {
        state.merge(get_mover_state(), true);

        const bool new_is_powered = (state.get("power24_available", false) || state.get("power110_available", false));
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered; // FIXME: I don't like this
            emit_signal(POWER_CHANGED_SIGNAL, prev_is_powered);
        }

        if (const bool new_radio_enabled = state.get("radio_enabled", false) && new_is_powered;
            prev_radio_enabled != new_radio_enabled) {
            prev_radio_enabled = new_radio_enabled; // FIXME: I don't like this
            emit_signal(RADIO_TOGGLED, new_radio_enabled);
        }

        if (const int new_radio_channel = state.get("radio_channel", 0); prev_radio_channel != new_radio_channel) {
            prev_radio_channel = new_radio_channel; // FIXME: I don't like this
            emit_signal(RADIO_CHANNEL_CHANGED, new_radio_channel);
        }
    }

    void TrainNode::_process(const double delta) {
        /* nie daj borze w edytorze */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        _update_mover_config_if_dirty();
        _process_mover(delta);
    }

    void TrainNode::_do_update_internal_mover(TMoverParameters *mover) const {
        mover->Mass = mass;
        mover->Power = power;
        mover->Vmax = max_velocity;

        mover->ComputeMass();
        mover->NPoweredAxles = Maszyna::s2NPW(axle_arrangement.ascii().get_data());
        mover->NAxles = mover->NPoweredAxles + Maszyna::s2NNW(axle_arrangement.ascii().get_data());

        // FIXME: move to TrainPower
        mover->BatteryVoltage = battery_voltage;
        mover->NominalBatteryVoltage = static_cast<float>(battery_voltage); // LoadFIZ_Light
    }

    void TrainNode::_do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const {
        config["axles_powered_count"] = mover->NPoweredAxles;
        config["axles_count"] = mover->NAxles;
    }

    void TrainNode::update_mover() {
        if (TMoverParameters *mover = get_mover(); mover != nullptr) {
            _do_update_internal_mover(mover);
            Dictionary new_config;
            _do_fetch_config_from_mover(mover, new_config);
            update_config(new_config);

            /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
            mover->CheckLocomotiveParameters(true, 0); // FIXME: brakujace parametery
        } else {
            UtilityFunctions::push_warning("TrainNode::update_mover() failed: internal mover not initialized");
        }
    }

    Dictionary TrainNode::get_mover_state() {
        if (TMoverParameters *mover = get_mover(); mover != nullptr) {
            _do_fetch_state_from_mover(mover, state);
        } else {
            UtilityFunctions::push_warning("TrainNode::get_mover_state() failed: internal mover not initialized");
        }
        return internal_state;
    }

    void TrainNode::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        internal_state["mass_total"] = mover->TotalMass;
        internal_state["velocity"] = mover->V;
        internal_state["speed"] = mover->Vel;
        internal_state["total_distance"] = mover->DistCounter;
        internal_state["direction"] = mover->DirActive;
        internal_state["cabin"] = mover->CabActive;
        internal_state["cabin_controleable"] = mover->IsCabMaster();
        internal_state["cabin_occupied"] = mover->CabOccupied;

        /* FIXME: move to TrainPower section? */
        internal_state["battery_enabled"] = mover->Battery;
        internal_state["battery_voltage"] = mover->BatteryVoltage;

        /* FIXME: move to TrainRadio section? */
        internal_state["radio_enabled"] = mover->Radio;
        internal_state["radio_powered"] = mover->Radio && (mover->Power24vIsAvailable || mover->Power110vIsAvailable);
        internal_state["radio_channel"] = radio_channel;

        /* FIXME: move to TrainPower section */
        internal_state["power24_voltage"] = mover->Power24vVoltage;
        internal_state["power24_available"] = mover->Power24vIsAvailable;
        internal_state["power110_available"] = mover->Power110vIsAvailable;
        internal_state["current0"] = mover->ShowCurrent(0);
        internal_state["current1"] = mover->ShowCurrent(1);
        internal_state["current2"] = mover->ShowCurrent(2);
        internal_state["relay_novolt"] = mover->NoVoltRelay;
        internal_state["relay_overvoltage"] = mover->OvervoltageRelay;
        internal_state["relay_ground"] = mover->GroundRelay;
        internal_state["train_damage"] = mover->DamageFlag;
        internal_state["controller_second_position"] = mover->ScndCtrlPos;
        internal_state["controller_main_position"] = mover->MainCtrlPos;
    }

    Dictionary TrainNode::get_config() const {
        return config;
    }

    void TrainNode::update_config(const Dictionary &p_config) {
        config.merge(p_config, true);
        emit_signal(CONFIG_CHANGED);
    }

    Dictionary TrainNode::get_state() {
        return state;
    }

    void TrainNode::emit_command_received_signal(const String &command, const Variant &p1, const Variant &p2) {
        emit_signal(COMMAND_RECEIVED, command, p1, p2);
    }

    void TrainNode::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        TrainSystem::get_instance()->broadcast_command(command, p1, p2);
    }

    void TrainNode::send_command(const StringName &command, const Variant &p1, const Variant &p2) const {
        TrainSystem::get_instance()->send_command(train_id, String(command), p1, p2);
    }

    void TrainNode::battery(const bool p_enabled) const {
        mover->BatterySwitch(p_enabled);
    }

    void TrainNode::main_controller_increase(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->IncMainCtrl(step);
    }

    void TrainNode::main_controller_decrease(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->DecMainCtrl(step);
    }

    void TrainNode::direction_increase() const {
        mover->DirectionForward();
    }

    void TrainNode::direction_decrease() const {
        mover->DirectionBackward();
    }

    void TrainNode::radio_channel_increase(const int p_step) {
        const int step = p_step > 0 ? p_step : 1;
        radio_channel = Math::clamp(radio_channel + step, radio_channel_min, radio_channel_max);
    }

    void TrainNode::radio_channel_decrease(const int p_step) {
        const int step = (p_step != 0) ? p_step : 1;
        radio_channel = Math::clamp(radio_channel - step, radio_channel_min, radio_channel_max);
    }

    void TrainNode::radio_channel_set(const int p_channel) {
        radio_channel = Math::clamp(p_channel, radio_channel_min, radio_channel_max);
    }

    void TrainNode::radio(const bool p_enabled) {
        mover->Radio = p_enabled;
    }
} // namespace godot
