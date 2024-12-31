#include "TrainSecuritySystem.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainSecuritySystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_aware_system_active"), &TrainSecuritySystem::get_aware_system_active);
        ClassDB::bind_method(D_METHOD("set_aware_system_active"), &TrainSecuritySystem::set_aware_system_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "aware_system/active"), "set_aware_system_active",
                "get_aware_system_active");

        ClassDB::bind_method(D_METHOD("get_aware_system_cabsignal"), &TrainSecuritySystem::get_aware_system_cabsignal);
        ClassDB::bind_method(D_METHOD("set_aware_system_cabsignal"), &TrainSecuritySystem::set_aware_system_cabsignal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "aware_system/cabsignal"), "set_aware_system_cabsignal",
                "get_aware_system_cabsignal");

        ClassDB::bind_method(
                D_METHOD("get_aware_system_separate_acknowledge"),
                &TrainSecuritySystem::get_aware_system_separate_acknowledge);
        ClassDB::bind_method(
                D_METHOD("set_aware_system_separate_acknowledge"),
                &TrainSecuritySystem::set_aware_system_separate_acknowledge);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "aware_system/separate_acknowledge"),
                "set_aware_system_separate_acknowledge", "get_aware_system_separate_acknowledge");

        ClassDB::bind_method(D_METHOD("get_aware_system_sifa"), &TrainSecuritySystem::get_aware_system_sifa);
        ClassDB::bind_method(D_METHOD("set_aware_system_sifa"), &TrainSecuritySystem::set_aware_system_sifa);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "aware_system/sifa"), "set_aware_system_sifa", "get_aware_system_sifa");

        ClassDB::bind_method(D_METHOD("get_aware_delay"), &TrainSecuritySystem::get_aware_delay);
        ClassDB::bind_method(D_METHOD("set_aware_delay"), &TrainSecuritySystem::set_aware_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "aware_delay"), "set_aware_delay", "get_aware_delay");

        ClassDB::bind_method(D_METHOD("get_emergency_brake_delay"), &TrainSecuritySystem::get_emergency_brake_delay);
        ClassDB::bind_method(D_METHOD("set_emergency_brake_delay"), &TrainSecuritySystem::set_emergency_brake_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "emergency_brake_delay"), "set_emergency_brake_delay",
                "get_emergency_brake_delay");

        ClassDB::bind_method(
                D_METHOD("get_emergency_brake_warning_signal"),
                &TrainSecuritySystem::get_emergency_brake_warning_signal);
        ClassDB::bind_method(
                D_METHOD("set_emergency_brake_warning_signal"),
                &TrainSecuritySystem::set_emergency_brake_warning_signal);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "emergency_brake_warning_signal", PROPERTY_HINT_ENUM,
                        "SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE"),
                "set_emergency_brake_warning_signal", "get_emergency_brake_warning_signal");

        ClassDB::bind_method(D_METHOD("get_radio_stop"), &TrainSecuritySystem::get_radio_stop);
        ClassDB::bind_method(D_METHOD("set_radio_stop"), &TrainSecuritySystem::set_radio_stop);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "radio_stop"), "set_radio_stop", "get_radio_stop");

        ClassDB::bind_method(D_METHOD("get_sound_signal_delay"), &TrainSecuritySystem::get_sound_signal_delay);
        ClassDB::bind_method(D_METHOD("set_sound_signal_delay", "value"), &TrainSecuritySystem::set_sound_signal_delay);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "sound_signal_delay"), "set_sound_signal_delay", "get_sound_signal_delay");

        ClassDB::bind_method(D_METHOD("get_shp_magnet_distance"), &TrainSecuritySystem::get_shp_magnet_distance);
        ClassDB::bind_method(D_METHOD("set_shp_magnet_distance"), &TrainSecuritySystem::set_shp_magnet_distance);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "shp_magnet_distance"), "set_shp_magnet_distance",
                "get_shp_magnet_distance");

        ClassDB::bind_method(D_METHOD("get_ca_max_hold_time"), &TrainSecuritySystem::get_ca_max_hold_time);
        ClassDB::bind_method(D_METHOD("set_ca_max_hold_time"), &TrainSecuritySystem::set_ca_max_hold_time);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ca_max_hold_time"), "set_ca_max_hold_time", "get_ca_max_hold_time");
        ClassDB::bind_method(D_METHOD("security_acknowledge", "enabled"), &TrainSecuritySystem::security_acknowledge);
        ClassDB::bind_method(
                D_METHOD("_on_train_controller_state_changed"),
                &TrainSecuritySystem::_on_train_controller_state_changed);

        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_SIREN_LOWTONE);
        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE);
        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_WHISTLE);
    }
    // Getters
    bool TrainSecuritySystem::get_aware_system_active() const {
        return aware_system_active;
    }
    bool TrainSecuritySystem::get_aware_system_cabsignal() const {
        return aware_system_cabsignal;
    }
    bool TrainSecuritySystem::get_aware_system_separate_acknowledge() const {
        return aware_system_separate_acknowledge;
    }
    bool TrainSecuritySystem::get_aware_system_sifa() const {
        return aware_system_sifa;
    }
    double TrainSecuritySystem::get_aware_delay() const {
        return aware_delay;
    }
    double TrainSecuritySystem::get_emergency_brake_delay() const {
        return emergency_brake_delay;
    }
    TrainSecuritySystem::EmergencyBrakeWarningSignal TrainSecuritySystem::get_emergency_brake_warning_signal() const {
        return emergency_brake_warning_signal;
    }
    bool TrainSecuritySystem::get_radio_stop() const {
        return radio_stop;
    }
    double TrainSecuritySystem::get_sound_signal_delay() const {
        return sound_signal_delay;
    }
    double TrainSecuritySystem::get_shp_magnet_distance() const {
        return shp_magnet_distance;
    }
    double TrainSecuritySystem::get_ca_max_hold_time() const {
        return ca_max_hold_time;
    }

    // Setters
    void TrainSecuritySystem::set_aware_system_active(const bool p_state) {
        aware_system_active = p_state;
        _dirty = true;
    }
    void TrainSecuritySystem::set_aware_system_cabsignal(const bool p_state) {
        aware_system_cabsignal = p_state;
        _dirty = true;
    }
    void TrainSecuritySystem::set_aware_system_separate_acknowledge(const bool p_state) {
        aware_system_separate_acknowledge = p_state;
        _dirty = true;
    }
    void TrainSecuritySystem::set_aware_system_sifa(const bool p_state) {
        aware_system_sifa = p_state;
        _dirty = true;
    }
    void TrainSecuritySystem::set_aware_delay(const double value) {
        aware_delay = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_emergency_brake_delay(const double value) {
        emergency_brake_delay = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_emergency_brake_warning_signal(const EmergencyBrakeWarningSignal value) {
        emergency_brake_warning_signal = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_radio_stop(const bool value) {
        radio_stop = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_sound_signal_delay(const double value) {
        sound_signal_delay = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_shp_magnet_distance(const double value) {
        shp_magnet_distance = value;
        _dirty = true;
    }
    void TrainSecuritySystem::set_ca_max_hold_time(const double value) {
        ca_max_hold_time = value;
        _dirty = true;
    }

    void TrainSecuritySystem::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["beeping"] = mover->SecuritySystem.is_beeping();
        state["blinking"] = mover->SecuritySystem.is_blinking();
        state["radiostop_available"] = mover->SecuritySystem.radiostop_available();
        state["vigilance_blinking"] = mover->SecuritySystem.is_vigilance_blinking();
        state["cabsignal_blinking"] = mover->SecuritySystem.is_cabsignal_blinking();
        state["cabsignal_beeping"] = mover->SecuritySystem.is_cabsignal_beeping();
        state["braking"] = mover->SecuritySystem.is_braking();
        state["engine_blocked"] = mover->SecuritySystem.is_engine_blocked();
        state["separate_acknowledge"] = mover->SecuritySystem.has_separate_acknowledge();
    }

    void TrainSecuritySystem::_do_update_internal_mover(TMoverParameters *mover) {
        mover->SecuritySystem.set_enabled(enabled);

        mover->SecuritySystem.vigilance_enabled = aware_system_active;
        mover->SecuritySystem.cabsignal_enabled = aware_system_cabsignal;
        mover->SecuritySystem.separate_acknowledge = aware_system_separate_acknowledge;
        mover->SecuritySystem.is_sifa = aware_system_sifa;

        mover->SecuritySystem.AwareDelay = aware_delay;
        mover->SecuritySystem.EmergencyBrakeDelay = emergency_brake_delay;
        mover->SecuritySystem.radiostop_enabled = radio_stop;
        mover->SecuritySystem.SoundSignalDelay = sound_signal_delay;
        mover->SecuritySystem.MagnetLocation = shp_magnet_distance;
        mover->SecuritySystem.MaxHoldTime = ca_max_hold_time;

        switch (emergency_brake_warning_signal) {
            case BRAKE_WARNINGSIGNAL_SIREN_LOWTONE:
                mover->EmergencyBrakeWarningSignal = 1;
                break;
            case BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE:
                mover->EmergencyBrakeWarningSignal = 2;
                break;
            case BRAKE_WARNINGSIGNAL_WHISTLE:
                mover->EmergencyBrakeWarningSignal = 4;
                break;
            default:
                mover->EmergencyBrakeWarningSignal = 1;
                break;
        }
    }

    void TrainSecuritySystem::_register_commands() {
        register_command("security_acknowledge", Callable(this, "security_acknowledge"));
    }

    void TrainSecuritySystem::_unregister_commands() {
        unregister_command("security_acknowledge", Callable(this, "security_acknowledge"));
    }

    void TrainSecuritySystem::security_acknowledge(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (p_enabled) {
            mover->SecuritySystem.acknowledge_press();
        } else {
            mover->SecuritySystem.acknowledge_release();
        }
    }

    void TrainSecuritySystem::_do_initialize_train_controller(TrainController *train_controller) {
        train_controller->connect(TrainController::STATE_CHANGED, Callable(this, "_on_train_controller_state_changed"));
    }

    void TrainSecuritySystem::_on_train_controller_state_changed() {
        Dictionary state = train_controller_node->get_state();

        if (prev_blinking != static_cast<bool>(state["blinking"])) {
            emit_signal("blinking_changed", state["blinking"]);
        }
        if (prev_beeping != static_cast<bool>(state["beeping"])) {
            emit_signal("beeping_changed", state["beeping"]);
        }

        prev_beeping = state["beeping"];
        prev_blinking = state["blinking"];
    }
} // namespace godot
