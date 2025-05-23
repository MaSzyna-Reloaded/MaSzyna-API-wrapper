#include "TrainSecuritySystem.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    // clang-format off
    void TrainSecuritySystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_aware_system_active"), &TrainSecuritySystem::GetAwareSystemActive);
        ClassDB::bind_method(D_METHOD("set_aware_system_active"), &TrainSecuritySystem::SetAwareSystemActive);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "aware_system/active"), "set_aware_system_active","get_aware_system_active");

        ClassDB::bind_method(D_METHOD("get_aware_system_cabsignal"), &TrainSecuritySystem::GetAwareSystemCabinSignalEnabled);
        ClassDB::bind_method(D_METHOD("set_aware_system_cabsignal"), &TrainSecuritySystem::SetAwareSystemCabinSignalEnabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "aware_system/cabsignal"), "set_aware_system_cabsignal","get_aware_system_cabsignal");

        ClassDB::bind_method(D_METHOD("get_aware_system_separate_acknowledge"),&TrainSecuritySystem::GetAwareSystemSeparateAcknowledge);
        ClassDB::bind_method(D_METHOD("set_aware_system_separate_acknowledge"),&TrainSecuritySystem::SetAwareSystemSeparateAcknowledge);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "aware_system/separate_acknowledge"),"set_aware_system_separate_acknowledge", "get_aware_system_separate_acknowledge");

        ClassDB::bind_method(D_METHOD("get_aware_system_sifa"), &TrainSecuritySystem::GetAwareSystemSifaAvailable);
        ClassDB::bind_method(D_METHOD("set_aware_system_sifa"), &TrainSecuritySystem::SetAwareSystemSifaAvailable);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "aware_system/sifa"), "set_aware_system_sifa", "get_aware_system_sifa");

        ClassDB::bind_method(D_METHOD("get_aware_delay"), &TrainSecuritySystem::GetAwareDelay);
        ClassDB::bind_method(D_METHOD("set_aware_delay"), &TrainSecuritySystem::SetAwareDelay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "aware_delay"), "set_aware_delay", "get_aware_delay");

        ClassDB::bind_method(D_METHOD("get_emergency_brake_delay"), &TrainSecuritySystem::GetEmergencyBrakeDelay);
        ClassDB::bind_method(D_METHOD("set_emergency_brake_delay"), &TrainSecuritySystem::SetEmergencyBrakeDelay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emergency_brake_delay"), "set_emergency_brake_delay","get_emergency_brake_delay");

        ClassDB::bind_method(D_METHOD("get_emergency_brake_warning_signal"),&TrainSecuritySystem::GetBrakeWarningSignalType);
        ClassDB::bind_method(D_METHOD("set_emergency_brake_warning_signal"),&TrainSecuritySystem::SetBrakeWarningSignalType);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "emergency_brake_warning_signal", PROPERTY_HINT_ENUM,"SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE"),"set_emergency_brake_warning_signal", "get_emergency_brake_warning_signal");

        ClassDB::bind_method(D_METHOD("get_radio_stop"), &TrainSecuritySystem::GetRadioStopEnabled);
        ClassDB::bind_method(D_METHOD("set_radio_stop"), &TrainSecuritySystem::SetRadioStopEnabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "radio_stop"), "set_radio_stop", "get_radio_stop");

        ClassDB::bind_method(D_METHOD("get_sound_signal_delay"), &TrainSecuritySystem::GetSoundSignalDelay);
        ClassDB::bind_method(D_METHOD("set_sound_signal_delay", "value"), &TrainSecuritySystem::SetSoundSignalDelay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sound_signal_delay"), "set_sound_signal_delay", "get_sound_signal_delay");

        ClassDB::bind_method(D_METHOD("get_shp_magnet_distance"), &TrainSecuritySystem::GetShpMagnetDistance);
        ClassDB::bind_method(D_METHOD("set_shp_magnet_distance"), &TrainSecuritySystem::SetShpMagnetDistance);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shp_magnet_distance"), "set_shp_magnet_distance","get_shp_magnet_distance");

        ClassDB::bind_method(D_METHOD("get_ca_max_hold_time"), &TrainSecuritySystem::GetAwareSystemMaxHoldTime);
        ClassDB::bind_method(D_METHOD("set_ca_max_hold_time"), &TrainSecuritySystem::SetAwareSystemMaxHoldTime);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ca_max_hold_time"), "set_ca_max_hold_time", "get_ca_max_hold_time");

        ClassDB::bind_method(D_METHOD("security_acknowledge", "enabled"), &TrainSecuritySystem::SetSecurityAcknowledgeEnabled);

        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(SIREN_LOW_TONE);
        BIND_ENUM_CONSTANT(SIREN_HIGH_TONE);
        BIND_ENUM_CONSTANT(WHISTLE);
    }
    // clang-format on


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

        mover->SecuritySystem.vigilance_enabled = m_bAwareSystemActive;
        mover->SecuritySystem.cabsignal_enabled = m_bAwareSystemCabinSignalEnabled;
        mover->SecuritySystem.separate_acknowledge = m_bAwareSystemSeparateAcknowledge;
        mover->SecuritySystem.is_sifa = m_bAwareSystemSifaAvailable;

        mover->SecuritySystem.AwareDelay = m_dAwareDelay;
        mover->SecuritySystem.EmergencyBrakeDelay = m_CurrentBrakeWarningSignal;
        mover->SecuritySystem.radiostop_enabled = m_bRadioStopEnabled;
        mover->SecuritySystem.SoundSignalDelay = m_dSoundSignalDelay;
        mover->SecuritySystem.MagnetLocation = m_dShpMagnetDistance;
        mover->SecuritySystem.MaxHoldTime = m_dAwareSystemMaxHoldTime;

        switch (m_CurrentBrakeWarningSignal) {
            case SIREN_LOW_TONE:
                mover->EmergencyBrakeWarningSignal = 1;
                break;
            case SIREN_HIGH_TONE:
                mover->EmergencyBrakeWarningSignal = 2;
                break;
            case WHISTLE:
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

    void TrainSecuritySystem::SetSecurityAcknowledgeEnabled(const bool enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (enabled) {
            mover->SecuritySystem.acknowledge_press();
        } else {
            mover->SecuritySystem.acknowledge_release();
        }
    }
} // namespace godot
