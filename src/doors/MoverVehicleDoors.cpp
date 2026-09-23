#include "MoverVehicleDoors.hpp"
#include "../mover/MoverBackend.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleDoors::_bind_methods() {}






    bool MoverVehicleDoors::get_locked() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.is_locked : false;
    }

    bool MoverVehicleDoors::get_lock_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.lock_enabled : false;
    }

    bool MoverVehicleDoors::get_step_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.step_enabled : false;
    }

    int MoverVehicleDoors::get_open_control() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.open_control : 0;
    }

    bool MoverVehicleDoors::get_left_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].is_open : false;
    }

    bool MoverVehicleDoors::get_left_open_permit() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].open_permit : false;
    }

    bool MoverVehicleDoors::get_left_local_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].local_open : false;
    }

    bool MoverVehicleDoors::get_left_remote_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].remote_open : false;
    }

    double MoverVehicleDoors::get_left_position() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].position : 0.0;
    }

    double MoverVehicleDoors::get_left_position_normalized() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].position / get_max_shift() : 0.0;
    }

    bool MoverVehicleDoors::get_left_operating() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].is_opening || mover->Doors.instances[side::left].is_closing : false;
    }

    double MoverVehicleDoors::get_left_step_position() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].step_position : 0.0;
    }

    bool MoverVehicleDoors::get_left_step_operating() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::left].step_folding || mover->Doors.instances[side::left].step_unfolding : false;
    }

    bool MoverVehicleDoors::get_right_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].is_open : false;
    }

    bool MoverVehicleDoors::get_right_open_permit() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].open_permit : false;
    }

    bool MoverVehicleDoors::get_right_local_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].local_open : false;
    }

    bool MoverVehicleDoors::get_right_remote_open() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].remote_open : false;
    }

    double MoverVehicleDoors::get_right_position() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].position : 0.0;
    }

    double MoverVehicleDoors::get_right_position_normalized() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].position / get_max_shift() : 0.0;
    }

    bool MoverVehicleDoors::get_right_operating() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].is_opening || mover->Doors.instances[side::right].is_closing : false;
    }

    double MoverVehicleDoors::get_right_step_position() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].step_position : 0.0;
    }

    bool MoverVehicleDoors::get_right_step_operating() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Doors.instances[side::right].step_folding || mover->Doors.instances[side::right].step_unfolding : false;
    }

    void MoverVehicleDoors::_fill_state_dictionary(Dictionary &p_state) const {
        TMoverParameters *mover = mover_of(this);
        if (mover == nullptr) {
            return;
        }
        p_state["doors_locked"] = get_locked();
        p_state["doors_lock_enabled"] = get_lock_enabled();
        p_state["doors_step_enabled"] = get_step_enabled();
        p_state["doors_open_control"] = get_open_control();
        p_state["doors_left_open"] = get_left_open();
        p_state["doors_left_open_permit"] = get_left_open_permit();
        p_state["doors_left_local_open"] = get_left_local_open();
        p_state["doors_left_remote_open"] = get_left_remote_open();
        p_state["doors_left_position"] = get_left_position();
        p_state["doors_left_position_normalized"] = get_left_position_normalized();
        p_state["doors_left_operating"] = get_left_operating();
        p_state["doors_left_step_position"] = get_left_step_position();
        p_state["doors_left_step_operating"] = get_left_step_operating();
        p_state["doors_right_open"] = get_right_open();
        p_state["doors_right_open_permit"] = get_right_open_permit();
        p_state["doors_right_local_open"] = get_right_local_open();
        p_state["doors_right_remote_open"] = get_right_remote_open();
        p_state["doors_right_position"] = get_right_position();
        p_state["doors_right_position_normalized"] = get_right_position_normalized();
        p_state["doors_right_operating"] = get_right_operating();
        p_state["doors_right_step_position"] = get_right_step_position();
        p_state["doors_right_step_operating"] = get_right_step_operating();
    }

    void MoverVehicleDoors::_do_process_component(const double p_delta) {
        TMoverParameters *p_mover = mover_of(this);
        ASSERT_MOVER(p_mover);
        p_mover->update_doors(p_delta);
    }

    void MoverVehicleDoors::next_permit_preset() {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->ChangeDoorPermitPreset(1);
    }

    void MoverVehicleDoors::previous_permit_preset() {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->ChangeDoorPermitPreset(-1);
    }

    void MoverVehicleDoors::permit_step(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->PermitDoorStep(p_state);
    }

    void MoverVehicleDoors::permit_doors(const Side p_side, const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->PermitDoors(p_side == Side::SIDE_LEFT ? side::left : side::right, p_state);
    }

    void MoverVehicleDoors::permit_left_doors(const bool p_state) {
        this->permit_doors(Side::SIDE_LEFT, p_state);
    }

    void MoverVehicleDoors::permit_right_doors(const bool p_state) {
        this->permit_doors(Side::SIDE_RIGHT, p_state);
    }

    void MoverVehicleDoors::operate_doors(const Side p_side, const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->OperateDoors(p_side == Side::SIDE_LEFT ? side::left : side::right, p_state);
    }

    void MoverVehicleDoors::operate_left_doors(const bool p_state) {
        this->operate_doors(Side::SIDE_LEFT, p_state);
    }

    void MoverVehicleDoors::operate_right_doors(const bool p_state) {
        this->operate_doors(Side::SIDE_RIGHT, p_state);
    }

    void MoverVehicleDoors::door_lock(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->LockDoors(p_state);
    }

    void MoverVehicleDoors::door_remote_control(const bool p_state) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->ChangeDoorControlMode(p_state);
    }

    void MoverVehicleDoors::_apply_configuration() {
        TMoverParameters *p_mover = mover_of(this);
        ASSERT_MOVER(p_mover);
        if (door_controls_map.find(get_open_method()) != door_controls_map.end()) {
            p_mover->Doors.open_control = door_controls_map.at(get_open_method());
        } else {
            log_error("Unhandled door open controls position: " + String::num(get_open_method()));
        }

        if (door_controls_map.find(get_close_method()) != door_controls_map.end()) {
            p_mover->Doors.close_control = door_controls_map.at(get_close_method());
        } else {
            log_error("Unhandled door close controls position: " + String::num(get_close_method()));
        }

        p_mover->Doors.auto_duration = get_open_time();
        p_mover->Doors.auto_velocity = get_close_auto_close_velocity();
        p_mover->Doors.auto_include_remote = get_close_auto_close_remote();
        p_mover->Doors.permit_needed = get_permit_required();
        p_mover->Doors.permit_presets.clear();
        for (int i = 0; i < get_permit_list().size(); i++) {
            if (get_permit_list()[i] != Variant()) {
                p_mover->Doors.permit_presets.emplace_back(static_cast<int>(get_permit_list()[i]));
            }
        }

        if (!p_mover->Doors.permit_presets.empty()) {
            p_mover->Doors.permit_preset = get_permit_default();
            p_mover->Doors.permit_preset =
                    std::min<int>(
                            static_cast<int>(p_mover->Doors.permit_presets.size()), p_mover->Doors.permit_preset) -
                    1;
        }

        p_mover->Doors.open_rate = get_open_speed();
        p_mover->Doors.open_delay = get_open_delay();
        p_mover->Doors.close_rate = get_close_speed();
        p_mover->Doors.close_delay = get_close_delay();
        p_mover->Doors.range = get_max_shift();
        p_mover->Doors.range_out = get_max_shift_plug();

        if (door_type_map.find(get_type()) != door_type_map.end()) {
            p_mover->Doors.type = door_type_map.at(get_type());
        } else {
            log_error("Unhandled door get_type(): " + String::num(get_type()));
        }

        p_mover->Doors.has_warning = get_close_warning();
        p_mover->Doors.has_autowarning = get_close_auto_close_warning();
        p_mover->Doors.has_lock = get_has_lock();
        bool const remote_control = {
                (get_open_method() == CONTROLS_DRIVER || get_open_method() == CONTROLS_CONDUCTOR || get_open_method() == CONTROLS_MIXED)};

        if (voltage_map.find(get_voltage()) != voltage_map.end()) {
            p_mover->Doors.voltage = voltage_map.at(get_voltage());
        } else {
            p_mover->Doors.voltage = remote_control ? 24 : 0;
        }
        p_mover->Doors.step_rate = get_platform_speed();
        p_mover->Doors.step_range = get_platform_max_shift();

        if (door_platform_type_map.find(get_platform_type()) != door_platform_type_map.end()) {
            p_mover->Doors.step_type = door_platform_type_map.at(get_platform_type());
        }

        p_mover->MirrorMaxShift = get_mirror_max_shift();
        p_mover->MirrorVelClose = get_mirror_close_velocity();
        p_mover->DoorsOpenWithPermitAfter = get_open_with_permit();
        p_mover->DoorsPermitLightBlinking = get_permit_light_blinking();
    }
} // namespace godot
