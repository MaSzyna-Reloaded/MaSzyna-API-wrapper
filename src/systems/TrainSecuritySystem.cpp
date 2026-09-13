#include "TrainSecuritySystem.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainSecuritySystem::_bind_methods() {
        BIND_PROPERTY(TrainSecuritySystem, Variant::BOOL, aware_system_active, "aware_system");
        BIND_PROPERTY(TrainSecuritySystem, Variant::BOOL, aware_system_cabsignal, "aware_system");
        BIND_PROPERTY(TrainSecuritySystem, Variant::BOOL, aware_system_separate_acknowledge, "aware_system");
        BIND_PROPERTY(TrainSecuritySystem, Variant::BOOL, aware_system_sifa, "aware_system");
        BIND_PROPERTY(TrainSecuritySystem, Variant::FLOAT, aware_delay);
        BIND_PROPERTY(TrainSecuritySystem, Variant::FLOAT, emergency_brake_delay, "emergency_brake");
        BIND_PROPERTY_W_HINT(
                TrainSecuritySystem, Variant::INT, emergency_signal, PROPERTY_HINT_ENUM,
                "SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE");
        BIND_PROPERTY(TrainSecuritySystem, Variant::BOOL, radio_stop_enabled, "radio_stop");
        BIND_PROPERTY(TrainSecuritySystem, Variant::FLOAT, sound_signal_delay);
        BIND_PROPERTY(TrainSecuritySystem, Variant::FLOAT, shp_magnet_distance);
        BIND_PROPERTY(TrainSecuritySystem, Variant::FLOAT, ca_max_hold_time);
        ClassDB::bind_method(D_METHOD("security_acknowledge", "enabled"), &TrainSecuritySystem::security_acknowledge);
        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_LOW_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_HIGH_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_WHISTLE);
    }

    void TrainSecuritySystem::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        const bool prev_beeping = p_state["beeping"];
        const bool prev_blinking = p_state["blinking"];
        p_state["beeping"] = p_mover->SecuritySystem.is_beeping();
        p_state["blinking"] = p_mover->SecuritySystem.is_blinking();
        p_state["radiostop_available"] = p_mover->SecuritySystem.radiostop_available();
        p_state["vigilance_blinking"] = p_mover->SecuritySystem.is_vigilance_blinking();
        p_state["cabsignal_blinking"] = p_mover->SecuritySystem.is_cabsignal_blinking();
        p_state["cabsignal_beeping"] = p_mover->SecuritySystem.is_cabsignal_beeping();
        p_state["braking"] = p_mover->SecuritySystem.is_braking();
        p_state["engine_blocked"] = p_mover->SecuritySystem.is_engine_blocked();
        p_state["separate_acknowledge"] = p_mover->SecuritySystem.has_separate_acknowledge();

        if (prev_blinking != static_cast<bool>(p_state["blinking"])) {
            emit_signal("blinking_changed", p_state["blinking"]);
        }

        if (prev_beeping != static_cast<bool>(p_state["beeping"])) {
            emit_signal("beeping_changed", p_state["beeping"]);
        }
    }

    void TrainSecuritySystem::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->SecuritySystem.set_enabled(enabled);

        p_mover->SecuritySystem.vigilance_enabled = aware_system_active;
        p_mover->SecuritySystem.cabsignal_enabled = aware_system_cabsignal;
        p_mover->SecuritySystem.separate_acknowledge = aware_system_separate_acknowledge;
        p_mover->SecuritySystem.is_sifa = aware_system_sifa;

        p_mover->SecuritySystem.AwareDelay = aware_delay;
        p_mover->SecuritySystem.EmergencyBrakeDelay = emergency_brake_delay;
        p_mover->SecuritySystem.radiostop_enabled = radio_stop_enabled;
        p_mover->SecuritySystem.SoundSignalDelay = sound_signal_delay;
        p_mover->SecuritySystem.MagnetLocation = shp_magnet_distance;
        p_mover->SecuritySystem.MaxHoldTime = ca_max_hold_time;

        switch (emergency_signal) {
            case EMERGENCY_SIGNAL_SIREN_LOW_TONE:
                p_mover->EmergencyBrakeWarningSignal = 1;
                break;
            case EMERGENCY_SIGNAL_SIREN_HIGH_TONE:
                p_mover->EmergencyBrakeWarningSignal = 2;
                break;
            case EMERGENCY_SIGNAL_WHISTLE:
                p_mover->EmergencyBrakeWarningSignal = 4;
                break;
            default:
                p_mover->EmergencyBrakeWarningSignal = 1;
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
