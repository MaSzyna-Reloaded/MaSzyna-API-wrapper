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

        ClassDB::bind_method(D_METHOD("get_beeping"), &VehicleSecuritySystem::get_beeping);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "beeping", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_beeping");
        ClassDB::bind_method(D_METHOD("get_blinking"), &VehicleSecuritySystem::get_blinking);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "blinking", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_blinking");
        ClassDB::bind_method(D_METHOD("get_radiostop_available"), &VehicleSecuritySystem::get_radiostop_available);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "radiostop_available", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_radiostop_available");
        ClassDB::bind_method(D_METHOD("get_vigilance_blinking"), &VehicleSecuritySystem::get_vigilance_blinking);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "vigilance_blinking", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_vigilance_blinking");
        ClassDB::bind_method(D_METHOD("get_cabsignal_blinking"), &VehicleSecuritySystem::get_cabsignal_blinking);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabsignal_blinking", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabsignal_blinking");
        ClassDB::bind_method(D_METHOD("get_cabsignal_beeping"), &VehicleSecuritySystem::get_cabsignal_beeping);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabsignal_beeping", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabsignal_beeping");
        ClassDB::bind_method(D_METHOD("get_braking"), &VehicleSecuritySystem::get_braking);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "braking", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_braking");
        ClassDB::bind_method(D_METHOD("get_engine_blocked"), &VehicleSecuritySystem::get_engine_blocked);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "engine_blocked", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_engine_blocked");
        ClassDB::bind_method(D_METHOD("get_separate_acknowledge"), &VehicleSecuritySystem::get_separate_acknowledge);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "separate_acknowledge", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_separate_acknowledge");
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


    bool VehicleSecuritySystem::get_beeping() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_beeping() : false;
    }

    bool VehicleSecuritySystem::get_blinking() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_blinking() : false;
    }

    bool VehicleSecuritySystem::get_radiostop_available() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.radiostop_available() : false;
    }

    bool VehicleSecuritySystem::get_vigilance_blinking() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_vigilance_blinking() : false;
    }

    bool VehicleSecuritySystem::get_cabsignal_blinking() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_cabsignal_blinking() : false;
    }

    bool VehicleSecuritySystem::get_cabsignal_beeping() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_cabsignal_beeping() : false;
    }

    bool VehicleSecuritySystem::get_braking() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_braking() : false;
    }

    bool VehicleSecuritySystem::get_engine_blocked() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.is_engine_blocked() : false;
    }

    bool VehicleSecuritySystem::get_separate_acknowledge() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->SecuritySystem.has_separate_acknowledge() : false;
    }

    void VehicleSecuritySystem::_fill_state_dictionary(Dictionary &p_state) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["beeping"] = get_beeping();
        p_state["blinking"] = get_blinking();
        p_state["radiostop_available"] = get_radiostop_available();
        p_state["vigilance_blinking"] = get_vigilance_blinking();
        p_state["cabsignal_blinking"] = get_cabsignal_blinking();
        p_state["cabsignal_beeping"] = get_cabsignal_beeping();
        p_state["braking"] = get_braking();
        p_state["engine_blocked"] = get_engine_blocked();
        p_state["separate_acknowledge"] = get_separate_acknowledge();
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
