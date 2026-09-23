#include "MoverVehicleHorns.hpp"
#include "../mover/MoverBackend.hpp"
#include "maszyna/utilities.h"

namespace godot {
    void MoverVehicleHorns::_bind_methods() {}





    void MoverVehicleHorns::set_horn_low(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        if (!get_low_horn_enabled()) {
            log_warning("Low horn button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 1;
        } else {
            mover->WarningSignal &= ~1;
        }
    }

    void MoverVehicleHorns::set_horn_high(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        if (!get_high_horn_enabled()) {
            log_warning("High horn button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 2;
        } else {
            mover->WarningSignal &= ~2;
        }
    }

    void MoverVehicleHorns::set_whistle(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        if (!get_whistle_enabled()) {
            log_warning("Whistle button is missing, or wasn't defined");
            return;
        }
        if (p_state) {
            mover->WarningSignal |= 4;
        } else {
            mover->WarningSignal &= ~4;
        }
    }

    void MoverVehicleHorns::set_horn(const double p_position) {
        set_horn_low(p_position > 0.0);
        set_horn_high(p_position < 0.0);
    }


    bool MoverVehicleHorns::get_low_pressed() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(mover->WarningSignal, 1) : false;
    }

    bool MoverVehicleHorns::get_high_pressed() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(mover->WarningSignal, 2) : false;
    }

    bool MoverVehicleHorns::get_whistle_pressed() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(mover->WarningSignal, 4) : false;
    }

    int MoverVehicleHorns::get_combined_signal() const {
        const TMoverParameters *mover = mover_of(this);
        if (mover == nullptr) {
            return 0;
        }
        return ((mover->Vel > HORN_EMERGENCY_MIN_SPEED) && mover->AlarmChainFlag
                        ? mover->EmergencyBrakeWarningSignal
                        : 0) |
                mover->WarningSignal;
    }

    bool MoverVehicleHorns::get_low_active() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(get_combined_signal(), 1) : false;
    }

    bool MoverVehicleHorns::get_high_active() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(get_combined_signal(), 2) : false;
    }

    bool MoverVehicleHorns::get_whistle_active() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(get_combined_signal(), 4) : false;
    }

    int MoverVehicleHorns::get_horn() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? TestFlag(mover->WarningSignal, 1) ? 1 : (TestFlag(mover->WarningSignal, 2) ? -1 : 0) : 0;
    }

    void MoverVehicleHorns::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (mover_of(this) == nullptr) {
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
