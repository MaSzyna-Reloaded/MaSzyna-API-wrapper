#include "VehicleSecuritySystem.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleSecuritySystem::_bind_methods() {
        BIND_PROPERTY(VehicleSecuritySystem, Variant::BOOL, aware_system_active, "aware_system");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::BOOL, aware_system_cabsignal, "aware_system");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::BOOL, aware_system_separate_acknowledge, "aware_system");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::BOOL, aware_system_sifa, "aware_system");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::FLOAT, aware_delay);
        BIND_PROPERTY(VehicleSecuritySystem, Variant::FLOAT, emergency_brake_delay, "emergency_brake");
        BIND_PROPERTY_W_HINT(
                VehicleSecuritySystem, Variant::INT, emergency_signal, PROPERTY_HINT_ENUM,
                "SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::BOOL, radio_stop_enabled, "radio_stop");
        BIND_PROPERTY(VehicleSecuritySystem, Variant::FLOAT, sound_signal_delay);
        BIND_PROPERTY(VehicleSecuritySystem, Variant::FLOAT, shp_magnet_distance);
        BIND_PROPERTY(VehicleSecuritySystem, Variant::FLOAT, ca_max_hold_time);
        ClassDB::bind_method(D_METHOD("security_acknowledge", "enabled"), &VehicleSecuritySystem::security_acknowledge);
        ClassDB::bind_method(
                D_METHOD("security_cabsignal_acknowledge"), &VehicleSecuritySystem::security_cabsignal_acknowledge);
        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_LOW_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_HIGH_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_WHISTLE);
    }

    // Detected once per tick against this part's own members - these used to be compared against
    // the state dictionary while that dictionary was being filled, so the signals fired on a read
    // rather than on a change.
    void VehicleSecuritySystem::_do_process_mover(TMoverParameters *p_mover, double p_delta) {
        if (const bool blinking = p_mover->SecuritySystem.is_blinking(); previous_blinking != blinking) {
            previous_blinking = blinking;
            emit_signal("blinking_changed", blinking);
        }
        if (const bool beeping = p_mover->SecuritySystem.is_beeping(); previous_beeping != beeping) {
            previous_beeping = beeping;
            emit_signal("beeping_changed", beeping);
        }
    }

    void VehicleSecuritySystem::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("beeping", Variant::BOOL);
        declare_state_property("blinking", Variant::BOOL);
        declare_state_property("radiostop_available", Variant::BOOL);
        declare_state_property("vigilance_blinking", Variant::BOOL);
        declare_state_property("cabsignal_blinking", Variant::BOOL);
        declare_state_property("cabsignal_beeping", Variant::BOOL);
        declare_state_property("braking", Variant::BOOL);
        declare_state_property("engine_blocked", Variant::BOOL);
        declare_state_property("separate_acknowledge", Variant::BOOL);
    }

    Variant VehicleSecuritySystem::_get_state_property(const int p_local_index) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_BEEPING:
                return mover->SecuritySystem.is_beeping();
            case STATE_BLINKING:
                return mover->SecuritySystem.is_blinking();
            case STATE_RADIOSTOP_AVAILABLE:
                return mover->SecuritySystem.radiostop_available();
            case STATE_VIGILANCE_BLINKING:
                return mover->SecuritySystem.is_vigilance_blinking();
            case STATE_CABSIGNAL_BLINKING:
                return mover->SecuritySystem.is_cabsignal_blinking();
            case STATE_CABSIGNAL_BEEPING:
                return mover->SecuritySystem.is_cabsignal_beeping();
            case STATE_BRAKING:
                return mover->SecuritySystem.is_braking();
            case STATE_ENGINE_BLOCKED:
                return mover->SecuritySystem.is_engine_blocked();
            case STATE_SEPARATE_ACKNOWLEDGE:
                return mover->SecuritySystem.has_separate_acknowledge();
            default:
                return Variant();
        }
    }

    void VehicleSecuritySystem::_do_update_internal_mover(TMoverParameters *p_mover) {
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

    void VehicleSecuritySystem::_register_commands() {
        register_command("security_acknowledge", Callable(this, "security_acknowledge"));
        register_command("security_cabsignal_acknowledge", Callable(this, "security_cabsignal_acknowledge"));
    }

    void VehicleSecuritySystem::_unregister_commands() {
        unregister_command("security_acknowledge", Callable(this, "security_acknowledge"));
        unregister_command("security_cabsignal_acknowledge", Callable(this, "security_cabsignal_acknowledge"));
    }

    // Train.cpp:2876 OnCommand_cabsignalacknowledge - the cab signalling of a vehicle with a separate
    // acknowledge button (FIZ SeparateAcknowledge) is not reset by the vigilance button
    void VehicleSecuritySystem::security_cabsignal_acknowledge() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (mover->SecuritySystem.has_separate_acknowledge()) {
            mover->SecuritySystem.cabsignal_reset();
        }
    }

    void VehicleSecuritySystem::security_acknowledge(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (p_enabled) {
            mover->SecuritySystem.acknowledge_press();
        } else {
            mover->SecuritySystem.acknowledge_release();
        }
    }
} // namespace godot
