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

        ClassDB::bind_method(D_METHOD("get_low_pressed"), &VehicleHorns::get_low_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "low_pressed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_low_pressed");
        ClassDB::bind_method(D_METHOD("get_high_pressed"), &VehicleHorns::get_high_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "high_pressed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_high_pressed");
        ClassDB::bind_method(D_METHOD("get_whistle_pressed"), &VehicleHorns::get_whistle_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "whistle_pressed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_whistle_pressed");
        ClassDB::bind_method(D_METHOD("get_combined_signal"), &VehicleHorns::get_combined_signal);
        ClassDB::bind_method(D_METHOD("get_low_active"), &VehicleHorns::get_low_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "low_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_low_active");
        ClassDB::bind_method(D_METHOD("get_high_active"), &VehicleHorns::get_high_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "high_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_high_active");
        ClassDB::bind_method(D_METHOD("get_whistle_active"), &VehicleHorns::get_whistle_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "whistle_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_whistle_active");
        ClassDB::bind_method(D_METHOD("get_horn"), &VehicleHorns::get_horn);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "horn", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_horn");
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


    bool VehicleHorns::get_low_pressed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(mover->WarningSignal, 1) : false;
    }

    bool VehicleHorns::get_high_pressed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(mover->WarningSignal, 2) : false;
    }

    bool VehicleHorns::get_whistle_pressed() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(mover->WarningSignal, 4) : false;
    }

    int VehicleHorns::get_combined_signal() const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return 0;
        }
        return ((mover->Vel > HORN_EMERGENCY_MIN_SPEED) && mover->AlarmChainFlag
                        ? mover->EmergencyBrakeWarningSignal
                        : 0) |
                mover->WarningSignal;
    }

    bool VehicleHorns::get_low_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(get_combined_signal(), 1) : false;
    }

    bool VehicleHorns::get_high_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(get_combined_signal(), 2) : false;
    }

    bool VehicleHorns::get_whistle_active() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(get_combined_signal(), 4) : false;
    }

    int VehicleHorns::get_horn() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? TestFlag(mover->WarningSignal, 1) ? 1 : (TestFlag(mover->WarningSignal, 2) ? -1 : 0) : 0;
    }

    void VehicleHorns::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (get_mover() == nullptr) {
            return;
        }
        p_state["horn_low_pressed"] = get_low_pressed();
        p_state["horn_high_pressed"] = get_high_pressed();
        p_state["whistle_pressed"] = get_whistle_pressed();
        p_state["horn_low_active"] = get_low_active();
        p_state["horn_high_active"] = get_high_active();
        p_state["whistle_active"] = get_whistle_active();
        p_state["horn"] = get_horn();
    }
} // namespace godot
