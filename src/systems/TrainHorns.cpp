#include "TrainHorns.hpp"
#include "maszyna/utilities.h"

namespace godot {

    void TrainHorns::_bind_methods() {
        BIND_PROPERTY(
                Variant::BOOL, "low_horn_enabled", "low_horn_enabled", &TrainHorns::set_low_horn_enabled,
                &TrainHorns::get_low_horn_enabled, "state");
        BIND_PROPERTY(
                Variant::BOOL, "high_horn_enabled", "high_horn_enabled", &TrainHorns::set_high_horn_enabled,
                &TrainHorns::get_high_horn_enabled, "state");
        BIND_PROPERTY(
                Variant::BOOL, "whistle_enabled", "whistle_enabled", &TrainHorns::set_whistle_enabled,
                &TrainHorns::get_whistle_enabled, "state");
        ClassDB::bind_method(D_METHOD("set_horn_low", "state"), &TrainHorns::set_horn_low);
        ClassDB::bind_method(D_METHOD("set_horn_high", "state"), &TrainHorns::set_horn_high);
        ClassDB::bind_method(D_METHOD("set_whistle", "state"), &TrainHorns::set_whistle);
        ClassDB::bind_method(D_METHOD("set_horn", "position"), &TrainHorns::set_horn);
    }

    void TrainHorns::_register_commands() {
        register_command("horn_low", Callable(this, "set_horn_low"));
        register_command("horn_high", Callable(this, "set_horn_high"));
        register_command("whistle", Callable(this, "set_whistle"));
        register_command("horn", Callable(this, "set_horn"));
    }

    void TrainHorns::_unregister_commands() {
        unregister_command("horn_low", Callable(this, "set_horn_low"));
        unregister_command("horn_high", Callable(this, "set_horn_high"));
        unregister_command("whistle", Callable(this, "set_whistle"));
        unregister_command("horn", Callable(this, "set_horn"));
    }

    void TrainHorns::set_horn_low(const bool p_state) {
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

    void TrainHorns::set_horn_high(const bool p_state) {
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

    void TrainHorns::set_whistle(const bool p_state) {
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

    void TrainHorns::set_horn(const double p_position) {
        set_horn_low(p_position > 0.0);
        set_horn_high(p_position < 0.0);
    }

    void TrainHorns::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        // Mirrors DynObj.cpp's per-frame horn combination: while moving with the manual
        // emergency brake (alarm chain) pulled, the emergency signal overrides the
        // manually commanded one for whichever bits it carries.
        int const combined =
                ((p_mover->Vel > 0.5) && p_mover->AlarmChainFlag ? p_mover->EmergencyBrakeWarningSignal : 0) |
                p_mover->WarningSignal;

        p_state["horn_low_pressed"] = TestFlag(p_mover->WarningSignal, 1);
        p_state["horn_high_pressed"] = TestFlag(p_mover->WarningSignal, 2);
        p_state["whistle_pressed"] = TestFlag(p_mover->WarningSignal, 4);

        p_state["horn_low_active"] = TestFlag(combined, 1);
        p_state["horn_high_active"] = TestFlag(combined, 2);
        p_state["whistle_active"] = TestFlag(combined, 4);

        // Mirrors set_horn()'s signed input shape, for widgets driven by a single
        // bidirectional lever (e.g. CabinSwitch's state_property) to sync their position.
        if (TestFlag(p_mover->WarningSignal, 1)) {
            p_state["horn"] = 1;
        } else if (TestFlag(p_mover->WarningSignal, 2)) {
            p_state["horn"] = -1;
        } else {
            p_state["horn"] = 0;
        }
    }
} // namespace godot
