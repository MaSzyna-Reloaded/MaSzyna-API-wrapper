#include "TrainDoors.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    TrainDoors::TrainDoors() {
        set_capability("doors");
    }

    void TrainDoors::_bind_methods() {
        BIND_PROPERTY_W_HINT(Variant::INT, "type", "type", &TrainDoors::set_type, &TrainDoors::get_type, "type", PROPERTY_HINT_ENUM, "Shift,Rotate,Fold,Plug");
        BIND_PROPERTY(Variant::FLOAT, "open_time", "open/time", &TrainDoors::set_open_time, &TrainDoors::get_open_time, "open_time");
        BIND_PROPERTY(Variant::FLOAT, "open_speed", "open/speed", &TrainDoors::set_open_speed, &TrainDoors::get_open_speed, "open_speed");
        BIND_PROPERTY(Variant::FLOAT, "close_speed", "close/speed", &TrainDoors::set_close_speed, &TrainDoors::get_close_speed, "close_speed");
        BIND_PROPERTY(Variant::FLOAT, "max_shift", "max_shift", &TrainDoors::set_max_shift, &TrainDoors::get_max_shift, "max_shift");
        BIND_PROPERTY_W_HINT(Variant::INT, "open_method", "open/method", &TrainDoors::set_open_method, &TrainDoors::get_open_method, "open_method", PROPERTY_HINT_ENUM, "Passenger,Automatic,Driver,Conductor,Mixed");
        BIND_PROPERTY_W_HINT(Variant::INT, "close_method", "close/method", &TrainDoors::set_close_method, &TrainDoors::get_close_method, "close_method", PROPERTY_HINT_ENUM, "Passenger,Automatic,Driver,Conductor,Mixed");
        BIND_PROPERTY_W_HINT(Variant::INT, "voltage", "voltage", &TrainDoors::set_voltage, &TrainDoors::get_voltage, "voltage", PROPERTY_HINT_ENUM, "Automatic,0V,12V,24V,112V");
        BIND_PROPERTY(Variant::BOOL, "close_warning", "close/warning", &TrainDoors::set_close_warning, &TrainDoors::get_close_warning, "close_warning");
        BIND_PROPERTY(Variant::BOOL, "auto_close_warning", "close/auto_close_warning", &TrainDoors::set_auto_close_warning, &TrainDoors::get_auto_close_warning, "auto_close_warning");
        BIND_PROPERTY(Variant::FLOAT, "open_delay", "open/delay", &TrainDoors::set_open_delay, &TrainDoors::get_open_delay, "open_delay");
        BIND_PROPERTY(Variant::FLOAT, "close_delay", "close/delay", &TrainDoors::set_close_delay, &TrainDoors::get_close_delay, "close_delay");
        BIND_PROPERTY(Variant::FLOAT, "open_with_permit", "open/with_permit", &TrainDoors::set_open_with_permit, &TrainDoors::get_open_with_permit, "open_with_permit");
        BIND_PROPERTY(Variant::BOOL, "has_lock", "has_lock", &TrainDoors::set_has_lock, &TrainDoors::get_has_lock, "has_lock");
        BIND_PROPERTY(Variant::FLOAT, "max_shift_plug", "max_shift_plug", &TrainDoors::set_max_shift_plug, &TrainDoors::get_max_shift_plug, "max_shift_plug");
        BIND_PROPERTY_ARRAY("permit_list", "permit/list", &TrainDoors::set_permit_list, &TrainDoors::get_permit_list, "permit_list");
        BIND_PROPERTY(Variant::INT, "permit_default", "permit/default", &TrainDoors::set_permit_list_default, &TrainDoors::get_permit_list_default, "permit_default");
        BIND_PROPERTY(Variant::BOOL, "auto_close_remote", "close/auto_close_remote", &TrainDoors::set_auto_close_remote, &TrainDoors::get_auto_close_remote, "auto_close_remote");
        BIND_PROPERTY(Variant::FLOAT, "auto_close_velocity", "close/auto_close_velocity", &TrainDoors::set_auto_close_velocity, &TrainDoors::get_auto_close_velocity, "auto_close_velocity");
        BIND_PROPERTY(Variant::FLOAT, "platform_max_speed", "platform/max_speed", &TrainDoors::set_platform_max_speed, &TrainDoors::get_platform_max_speed, "platform_max_speed");
        BIND_PROPERTY_W_HINT(Variant::INT, "platform_type", "platform/type", &TrainDoors::set_platform_type, &TrainDoors::get_platform_type, "platform_type", PROPERTY_HINT_ENUM, "Shift,Rotate");
        BIND_PROPERTY(Variant::FLOAT, "platform_max_shift", "platform/max_shift", &TrainDoors::set_platform_max_shift, &TrainDoors::get_platform_max_shift, "platform_max_shift");
        BIND_PROPERTY(Variant::FLOAT, "platform_speed", "platform/speed", &TrainDoors::set_platform_speed, &TrainDoors::get_platform_speed, "platform_speed");
        BIND_PROPERTY(Variant::FLOAT, "mirror_max_shift", "mirror/max_shift", &TrainDoors::set_mirror_max_shift, &TrainDoors::get_mirror_max_shift, "mirror_max_shift");
        BIND_PROPERTY(Variant::FLOAT, "mirror_close_velocity", "mirror/close_velocity", &TrainDoors::set_mirror_close_velocity, &TrainDoors::get_mirror_close_velocity, "mirror_close_velocity");
        BIND_PROPERTY(Variant::BOOL, "permit_required", "permit/required", &TrainDoors::set_permit_required, &TrainDoors::get_permit_required, "permit_required");
        BIND_PROPERTY_W_HINT(Variant::INT, "permit_light_blinking", "permit/light_blinking", &TrainDoors::set_permit_light_blinking, &TrainDoors::get_permit_light_blinking, "permit_light_blinking", PROPERTY_HINT_ENUM, "Continuous light,Flashing on permission w/step,Flashing on permission,Flashing always");
        ClassDB::bind_method(D_METHOD("next_permit_preset"), &TrainDoors::next_permit_preset);
        ClassDB::bind_method(D_METHOD("previous_permit_preset"), &TrainDoors::previous_permit_preset);
        ClassDB::bind_method(D_METHOD("permit_step", "state"), &TrainDoors::permit_step);
        ClassDB::bind_method(D_METHOD("permit_left_doors", "state"), &TrainDoors::permit_left_doors);
        ClassDB::bind_method(D_METHOD("permit_right_doors", "state"), &TrainDoors::permit_right_doors);
        ClassDB::bind_method(D_METHOD("permit_doors", "side", "state"), &TrainDoors::permit_doors);
        ClassDB::bind_method(D_METHOD("operate_left_doors", "state"), &TrainDoors::operate_left_doors);
        ClassDB::bind_method(D_METHOD("operate_right_doors", "state"), &TrainDoors::operate_right_doors);
        ClassDB::bind_method(D_METHOD("operate_doors", "side", "state"), &TrainDoors::operate_doors);
        ClassDB::bind_method(D_METHOD("door_lock", "state"), &TrainDoors::door_lock);
        ClassDB::bind_method(D_METHOD("door_remote_control", "state"), &TrainDoors::door_remote_control);


        BIND_ENUM_CONSTANT(PERMIT_LIGHT_CONTINUOUS);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ON_PERMISSION);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ALWAYS);

        BIND_ENUM_CONSTANT(PLATFORM_TYPE_SHIFT);
        BIND_ENUM_CONSTANT(PLATFORM_TYPE_ROTATE);

        BIND_ENUM_CONSTANT(SIDE_RIGHT)
        BIND_ENUM_CONSTANT(SIDE_LEFT)

        BIND_ENUM_CONSTANT(CONTROLS_PASSENGER)
        BIND_ENUM_CONSTANT(CONTROLS_AUTOMATIC)
        BIND_ENUM_CONSTANT(CONTROLS_DRIVER)
        BIND_ENUM_CONSTANT(CONTROLS_CONDUCTOR)
        BIND_ENUM_CONSTANT(CONTROLS_MIXED)

        BIND_ENUM_CONSTANT(VOLTAGE_0)
        BIND_ENUM_CONSTANT(VOLTAGE_12)
        BIND_ENUM_CONSTANT(VOLTAGE_24)
        BIND_ENUM_CONSTANT(VOLTAGE_112)

        BIND_ENUM_CONSTANT(TYPE_SHIFT)
        BIND_ENUM_CONSTANT(TYPE_ROTATE)
        BIND_ENUM_CONSTANT(TYPE_FOLD)
        BIND_ENUM_CONSTANT(TYPE_PLUG)
    }

    void TrainDoors::_register_commands() {
        register_command("doors_next_permit_preset", Callable(this, "next_permit_preset"));
        register_command("doors_previous_permit_preset", Callable(this, "previous_permit_preset"));
        register_command("doors_permit_step", Callable(this, "permit_step"));
        register_command("doors_left_permit", Callable(this, "permit_left_doors"));
        register_command("doors_right_permit", Callable(this, "permit_right_doors"));
        register_command("doors_left", Callable(this, "operate_left_doors"));
        register_command("doors_right", Callable(this, "operate_right_doors"));
        register_command("doors_lock", Callable(this, "door_lock"));
        register_command("doors_remote_control", Callable(this, "door_remote_control"));
    }

    void TrainDoors::_unregister_commands() {
        unregister_command("doors_next_permit_preset", Callable(this, "next_permit_preset"));
        unregister_command("doors_previous_permit_preset", Callable(this, "previous_permit_preset"));
        unregister_command("doors_permit_step", Callable(this, "permit_step"));
        unregister_command("doors_left_permit", Callable(this, "permit_left_doors"));
        unregister_command("doors_right_permit", Callable(this, "permit_right_doors"));
        unregister_command("doors_left", Callable(this, "operate_left_doors"));
        unregister_command("doors_right", Callable(this, "operate_right_doors"));
        unregister_command("doors_lock", Callable(this, "door_lock"));
        unregister_command("doors_remote_control", Callable(this, "door_remote_control"));
    }

    void TrainDoors::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const auto left_door = mover->Doors.instances[side::left];
        const auto right_door = mover->Doors.instances[side::right];

        state["doors_locked"] = mover->Doors.is_locked;
        state["doors_lock_enabled"] = mover->Doors.lock_enabled;
        state["doors_step_enabled"] = mover->Doors.step_enabled;
        state["doors_open_control"] = mover->Doors.open_control;

        state["doors_left_open"] = left_door.is_open;
        state["doors_left_open_permit"] = left_door.open_permit;
        state["doors_left_local_open"] = left_door.local_open;
        state["doors_left_remote_open"] = left_door.remote_open;
        state["doors_left_position"] = left_door.position;
        state["doors_left_position_normalized"] = left_door.position / max_shift;
        state["doors_left_operating"] = left_door.is_closing || left_door.is_opening;
        state["doors_left_step_position"] = left_door.step_position;
        state["doors_left_step_operating"] = left_door.step_folding || left_door.step_unfolding;

        state["doors_right_open"] = right_door.is_open;
        state["doors_right_open_permit"] = right_door.open_permit;
        state["doors_right_local_open"] = right_door.local_open;
        state["doors_right_remote_open"] = right_door.remote_open;
        state["doors_right_position"] = right_door.position;
        state["doors_right_position_normalized"] = right_door.position / max_shift;
        state["doors_right_operating"] = right_door.is_opening || right_door.is_closing;
        state["doors_right_step_position"] = right_door.step_position;
        state["doors_right_step_operating"] = right_door.step_folding || right_door.step_unfolding;
    }

    void TrainDoors::_do_process_mover(TMoverParameters *mover, const double delta) {
        mover->update_doors(delta);
    }

    void TrainDoors::next_permit_preset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ChangeDoorPermitPreset(1);
    }

    void TrainDoors::previous_permit_preset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ChangeDoorPermitPreset(-1);
    }

    void TrainDoors::permit_step(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->PermitDoorStep(p_state);
    }

    void TrainDoors::permit_doors(const Side p_side, const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->PermitDoors(p_side == Side::SIDE_LEFT ? side::left : side::right, p_state);
    }

    void TrainDoors::permit_left_doors(const bool p_state) {
        this->permit_doors(Side::SIDE_LEFT, p_state);
    }

    void TrainDoors::permit_right_doors(const bool p_state) {
        this->permit_doors(Side::SIDE_RIGHT, p_state);
    }

    void TrainDoors::operate_doors(const Side p_side, const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OperateDoors(p_side == Side::SIDE_LEFT ? side::left : side::right, p_state);
    }

    void TrainDoors::operate_left_doors(const bool p_state) {
        this->operate_doors(Side::SIDE_LEFT, p_state);
    }

    void TrainDoors::operate_right_doors(const bool p_state) {
        this->operate_doors(Side::SIDE_RIGHT, p_state);
    }

    void TrainDoors::door_lock(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->LockDoors(p_state);
    }

    void TrainDoors::door_remote_control(const bool p_state) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ChangeDoorControlMode(p_state);
    }

    void TrainDoors::_do_update_internal_mover(TMoverParameters *mover) {
        if (doorControlsMap.find(open_method) != doorControlsMap.end()) {
            mover->Doors.open_control = doorControlsMap.at(open_method);
        } else {
            log_error("Unhandled door open controls position: " + String::num(open_method));
        }

        if (doorControlsMap.find(close_method) != doorControlsMap.end()) {
            mover->Doors.close_control = doorControlsMap.at(close_method);
        } else {
            log_error("Unhandled door close controls position: " + String::num(close_method));
        }

        mover->Doors.auto_duration = open_time;
        mover->Doors.auto_velocity = auto_close_velocity;
        mover->Doors.auto_include_remote = auto_close_remote;
        mover->Doors.permit_needed = permit_required;
        mover->Doors.permit_presets.clear();
        for (int i = 0; i < permit_list.size(); i++) {
            if (permit_list[i] != Variant()) {
                mover->Doors.permit_presets.emplace_back(static_cast<int>(permit_list[i]));
            }
        }

        if (!mover->Doors.permit_presets.empty()) {
            mover->Doors.permit_preset = permit_list_default;
            mover->Doors.permit_preset =
                    std::min<int>(static_cast<int>(mover->Doors.permit_presets.size()), mover->Doors.permit_preset) - 1;
        }

        mover->Doors.open_rate = open_speed;
        mover->Doors.open_delay = open_delay;
        mover->Doors.close_rate = close_speed;
        mover->Doors.close_delay = close_delay;
        mover->Doors.range = max_shift;
        mover->Doors.range_out = max_shift_plug;

        if (doorTypeMap.find(type) != doorTypeMap.end()) {
            mover->Doors.type = doorTypeMap.at(type);
        } else {
            log_error("Unhandled door type: " + String::num(type));
        }

        mover->Doors.has_warning = close_warning;
        mover->Doors.has_autowarning = auto_close_warning;
        mover->Doors.has_lock = has_lock;
        bool const remote_control = {
                (open_method == CONTROLS_DRIVER || open_method == CONTROLS_CONDUCTOR || open_method == CONTROLS_MIXED)};

        if (voltageMap.find(voltage) != voltageMap.end()) {
            mover->Doors.voltage = voltageMap.at(voltage);
        } else {
            mover->Doors.voltage = remote_control ? 24 : 0;
        }
        mover->Doors.step_rate = platform_speed;
        mover->Doors.step_range = platform_max_shift;

        if (doorPlatformTypeMap.find(platform_type) != doorPlatformTypeMap.end()) {
            mover->Doors.step_type = doorPlatformTypeMap.at(platform_type);
        }

        mover->MirrorMaxShift = mirror_max_shift;
        mover->MirrorVelClose = mirror_close_velocity;
        mover->DoorsOpenWithPermitAfter = open_with_permit;
        mover->DoorsPermitLightBlinking = permit_light_blinking;
    }
} // namespace godot
