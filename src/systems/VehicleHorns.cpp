#include "VehicleHorns.hpp"
#include "maszyna/utilities.h"

namespace godot {

    void VehicleHorns::_bind_methods() {
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, low_horn_enabled);
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, high_horn_enabled);
        BIND_PROPERTY(VehicleHorns, Variant::BOOL, whistle_enabled);
        ClassDB::bind_method(D_METHOD("set_horn_low", "state"), &VehicleHorns::set_horn_low);
        ClassDB::bind_method(D_METHOD("set_horn_high", "state"), &VehicleHorns::set_horn_high);
        ClassDB::bind_method(D_METHOD("set_whistle", "state"), &VehicleHorns::set_whistle);
        ClassDB::bind_method(D_METHOD("set_horn", "position"), &VehicleHorns::set_horn);
    }

    void VehicleHorns::_register_commands() {
        register_command("horn_low", Callable(this, "set_horn_low"));
        register_command("horn_high", Callable(this, "set_horn_high"));
        register_command("whistle", Callable(this, "set_whistle"));
        register_command("horn", Callable(this, "set_horn"));
    }

    void VehicleHorns::_unregister_commands() {
        unregister_command("horn_low", Callable(this, "set_horn_low"));
        unregister_command("horn_high", Callable(this, "set_horn_high"));
        unregister_command("whistle", Callable(this, "set_whistle"));
        unregister_command("horn", Callable(this, "set_horn"));
    }

    void VehicleHorns::set_horn_low(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (!low_horn_enabled) {
            log_warning("Low horn button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 1;
        } else {
            mover->WarningSignal &= ~1;
        }
    }

    void VehicleHorns::set_horn_high(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (!high_horn_enabled) {
            log_warning("High horn button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 2;
        } else {
            mover->WarningSignal &= ~2;
        }
    }

    void VehicleHorns::set_whistle(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (!whistle_enabled) {
            log_warning("Whistle button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 4;
        } else {
            mover->WarningSignal &= ~4;
        }
    }

    void VehicleHorns::set_horn(const double p_position) {
        set_horn_low(p_position > 0.0);
        set_horn_high(p_position < 0.0);
    }

    void VehicleHorns::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("horn_low_pressed", Variant::BOOL);
        declare_state_property("horn_high_pressed", Variant::BOOL);
        declare_state_property("whistle_pressed", Variant::BOOL);
        declare_state_property("horn_low_active", Variant::BOOL);
        declare_state_property("horn_high_active", Variant::BOOL);
        declare_state_property("whistle_active", Variant::BOOL);
        declare_state_property("horn", Variant::INT);
    }

    Variant VehicleHorns::_get_state_property(const int p_local_index) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        // Mirrors DynObj.cpp's per-frame horn combination: while moving with the manual emergency
        // brake (alarm chain) pulled, the emergency signal overrides the manually commanded one
        // for whichever bits it carries.
        const int combined =
                ((mover->Vel > 0.5) && mover->AlarmChainFlag ? mover->EmergencyBrakeWarningSignal : 0) |
                mover->WarningSignal;
        switch (p_local_index - state_base_index) {
            case STATE_HORN_LOW_PRESSED:
                return TestFlag(mover->WarningSignal, 1);
            case STATE_HORN_HIGH_PRESSED:
                return TestFlag(mover->WarningSignal, 2);
            case STATE_WHISTLE_PRESSED:
                return TestFlag(mover->WarningSignal, 4);
            case STATE_HORN_LOW_ACTIVE:
                return TestFlag(combined, 1);
            case STATE_HORN_HIGH_ACTIVE:
                return TestFlag(combined, 2);
            case STATE_WHISTLE_ACTIVE:
                return TestFlag(combined, 4);
            // Mirrors set_horn()'s signed input shape, for a widget driven by a single
            // bidirectional lever to sync its position.
            case STATE_HORN:
                return TestFlag(mover->WarningSignal, 1) ? 1 : (TestFlag(mover->WarningSignal, 2) ? -1 : 0);
            default:
                return Variant();
        }
    }
} // namespace godot
