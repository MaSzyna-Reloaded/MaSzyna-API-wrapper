#include "TrainSecuritySystem.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainSecuritySystem::_bind_methods() {
        BIND_PROPERTY(Variant::BOOL, "aware_system_active", "aware_system/active", &TrainSecuritySystem::set_aware_system_active, &TrainSecuritySystem::get_aware_system_active, "state");
        BIND_PROPERTY(Variant::BOOL, "aware_system_cabsignal", "aware_system/cabsignal", &TrainSecuritySystem::set_aware_system_cab_signal, &TrainSecuritySystem::get_aware_system_cab_signal, "state");
        BIND_PROPERTY(Variant::BOOL, "aware_system_separate_acknowledge", "aware_system/separate_acknowledge", &TrainSecuritySystem::set_aware_system_separate_acknowledge, &TrainSecuritySystem::get_aware_system_separate_acknowledge, "state");
        BIND_PROPERTY(Variant::BOOL, "aware_system_sifa", "aware_system/sifa", &TrainSecuritySystem::set_aware_system_sifa, &TrainSecuritySystem::get_aware_system_sifa, "state");
        BIND_PROPERTY(Variant::FLOAT, "aware_delay", "aware_delay", &TrainSecuritySystem::set_aware_delay, &TrainSecuritySystem::get_aware_delay, "delay");
        BIND_PROPERTY(Variant::FLOAT, "emergency_brake_delay", "emergency_brake/delay", &TrainSecuritySystem::set_emergency_brake_delay, &TrainSecuritySystem::get_emergency_brake_delay, "delay");
        BIND_PROPERTY_W_HINT(Variant::INT, "emergency_brake_warning_signal", "emergency_brake/warning_signal", &TrainSecuritySystem::set_brake_warning_signal, &TrainSecuritySystem::get_brake_warning_signal, "signal", PROPERTY_HINT_ENUM, "SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE");
        BIND_PROPERTY(Variant::BOOL, "radio_stop_enabled", "radio_stop/enabled", &TrainSecuritySystem::set_radio_stop_enabled, &TrainSecuritySystem::get_radio_stop_enabled, "state");
        BIND_PROPERTY(Variant::FLOAT, "sound_signal_delay", "sound_signal_delay", &TrainSecuritySystem::set_sound_signal_delay, &TrainSecuritySystem::get_sound_signal_delay, "delay");
        BIND_PROPERTY(Variant::FLOAT, "shp_magnet_distance", "shp_magnet_distance", &TrainSecuritySystem::set_shp_magnet_distance, &TrainSecuritySystem::get_shp_magnet_distance, "distance");
        BIND_PROPERTY(Variant::FLOAT, "ca_max_hold_time", "ca_max_hold_time", &TrainSecuritySystem::set_ca_max_hold_time, &TrainSecuritySystem::get_ca_max_hold_time, "delay");
        ClassDB::bind_method(D_METHOD("security_acknowledge", "enabled"), &TrainSecuritySystem::security_acknowledge);
        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_SIREN_LOWTONE);
        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE);
        BIND_ENUM_CONSTANT(BRAKE_WARNINGSIGNAL_WHISTLE);
    }

    void TrainSecuritySystem::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const bool prev_beeping = state["beeping"];
        const bool prev_blinking = state["blinking"];
        state["beeping"] = mover->SecuritySystem.is_beeping();
        state["blinking"] = mover->SecuritySystem.is_blinking();
        state["radiostop_available"] = mover->SecuritySystem.radiostop_available();
        state["vigilance_blinking"] = mover->SecuritySystem.is_vigilance_blinking();
        state["cabsignal_blinking"] = mover->SecuritySystem.is_cabsignal_blinking();
        state["cabsignal_beeping"] = mover->SecuritySystem.is_cabsignal_beeping();
        state["braking"] = mover->SecuritySystem.is_braking();
        state["engine_blocked"] = mover->SecuritySystem.is_engine_blocked();
        state["separate_acknowledge"] = mover->SecuritySystem.has_separate_acknowledge();

        if (prev_blinking != static_cast<bool>(state["blinking"])) {
            emit_signal("blinking_changed", state["blinking"]);
        }
        if (prev_beeping != static_cast<bool>(state["beeping"])) {
            emit_signal("beeping_changed", state["beeping"]);
        }
    }

    void TrainSecuritySystem::_do_update_internal_mover(TMoverParameters *mover) {
        mover->SecuritySystem.set_enabled(enabled);

        mover->SecuritySystem.vigilance_enabled = aware_system_active;
        mover->SecuritySystem.cabsignal_enabled = aware_system_cab_signal;
        mover->SecuritySystem.separate_acknowledge = aware_system_separate_acknowledge;
        mover->SecuritySystem.is_sifa = aware_system_sifa;

        mover->SecuritySystem.AwareDelay = aware_delay;
        mover->SecuritySystem.EmergencyBrakeDelay = emergency_brake_delay;
        mover->SecuritySystem.radiostop_enabled = radio_stop_enabled;
        mover->SecuritySystem.SoundSignalDelay = sound_signal_delay;
        mover->SecuritySystem.MagnetLocation = shp_magnet_distance;
        mover->SecuritySystem.MaxHoldTime = ca_max_hold_time;

        switch (brake_warning_signal) {
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
} // namespace godot
