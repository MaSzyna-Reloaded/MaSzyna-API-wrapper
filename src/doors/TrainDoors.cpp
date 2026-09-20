#include "TrainDoors.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void TrainDoors::_bind_methods() {
        BIND_PROPERTY_W_HINT(TrainDoors, Variant::INT, type, PROPERTY_HINT_ENUM, "Shift,Rotate,Fold,Plug");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, open_time, "open");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, open_speed, "open");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, close_speed, "close");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, max_shift);
        BIND_PROPERTY_W_HINT(
                TrainDoors, Variant::INT, open_method, "open", PROPERTY_HINT_ENUM,
                "Passenger,Automatic,Driver,Conductor,Mixed");
        BIND_PROPERTY_W_HINT(
                TrainDoors, Variant::INT, close_method, "close", PROPERTY_HINT_ENUM,
                "Passenger,Automatic,Driver,Conductor,Mixed");
        BIND_PROPERTY_W_HINT(TrainDoors, Variant::INT, voltage, PROPERTY_HINT_ENUM, "Automatic,0V,12V,24V,112V");
        BIND_PROPERTY(TrainDoors, Variant::BOOL, close_warning, "close");
        BIND_PROPERTY(TrainDoors, Variant::BOOL, close_auto_close_warning, "close");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, open_delay, "open");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, close_delay, "close");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, open_with_permit, "open");
        BIND_PROPERTY(TrainDoors, Variant::BOOL, has_lock);
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, max_shift_plug);
        BIND_PROPERTY_ARRAY(TrainDoors, permit_list, "permit");
        BIND_PROPERTY(TrainDoors, Variant::INT, permit_default, "permit");
        BIND_PROPERTY(TrainDoors, Variant::BOOL, close_auto_close_remote, "close");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, close_auto_close_velocity, "close");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, platform_max_speed, "platform");
        BIND_PROPERTY_W_HINT(TrainDoors, Variant::INT, platform_type, "platform", PROPERTY_HINT_ENUM, "Shift,Rotate");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, platform_max_shift, "platform");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, platform_speed, "platform");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, mirror_max_shift, "mirror");
        BIND_PROPERTY(TrainDoors, Variant::FLOAT, mirror_close_velocity, "mirror");
        BIND_PROPERTY(TrainDoors, Variant::BOOL, permit_required, "permit");
        BIND_PROPERTY_W_HINT(
                TrainDoors, Variant::INT, permit_light_blinking, "permit", PROPERTY_HINT_ENUM,
                "Continuous light,Flashing on permission w/step,Flashing on permission,Flashing always");
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

    void TrainDoors::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        const auto left_door = p_mover->Doors.instances[side::left];
        const auto right_door = p_mover->Doors.instances[side::right];

        p_state["doors_locked"] = p_mover->Doors.is_locked;
        p_state["doors_lock_enabled"] = p_mover->Doors.lock_enabled;
        p_state["doors_step_enabled"] = p_mover->Doors.step_enabled;
        p_state["doors_open_control"] = p_mover->Doors.open_control;

        p_state["doors_left_open"] = left_door.is_open;
        p_state["doors_left_open_permit"] = left_door.open_permit;
        p_state["doors_left_local_open"] = left_door.local_open;
        p_state["doors_left_remote_open"] = left_door.remote_open;
        p_state["doors_left_position"] = left_door.position;
        p_state["doors_left_position_normalized"] = left_door.position / max_shift;
        p_state["doors_left_operating"] = left_door.is_closing || left_door.is_opening;
        p_state["doors_left_step_position"] = left_door.step_position;
        p_state["doors_left_step_operating"] = left_door.step_folding || left_door.step_unfolding;

        p_state["doors_right_open"] = right_door.is_open;
        p_state["doors_right_open_permit"] = right_door.open_permit;
        p_state["doors_right_local_open"] = right_door.local_open;
        p_state["doors_right_remote_open"] = right_door.remote_open;
        p_state["doors_right_position"] = right_door.position;
        p_state["doors_right_position_normalized"] = right_door.position / max_shift;
        p_state["doors_right_operating"] = right_door.is_opening || right_door.is_closing;
        p_state["doors_right_step_position"] = right_door.step_position;
        p_state["doors_right_step_operating"] = right_door.step_folding || right_door.step_unfolding;
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

    void TrainDoors::_do_update_internal_mover(TMoverParameters *p_mover) {
        if (door_controls_map.find(open_method) != door_controls_map.end()) {
            p_mover->Doors.open_control = door_controls_map.at(open_method);
        } else {
            log_error("Unhandled door open controls position: " + String::num(open_method));
        }

        if (door_controls_map.find(close_method) != door_controls_map.end()) {
            p_mover->Doors.close_control = door_controls_map.at(close_method);
        } else {
            log_error("Unhandled door close controls position: " + String::num(close_method));
        }

        p_mover->Doors.auto_duration = open_time;
        p_mover->Doors.auto_velocity = close_auto_close_velocity;
        p_mover->Doors.auto_include_remote = close_auto_close_remote;
        p_mover->Doors.permit_needed = permit_required;
        p_mover->Doors.permit_presets.clear();
        for (int i = 0; i < permit_list.size(); i++) {
            if (permit_list[i] != Variant()) {
                p_mover->Doors.permit_presets.emplace_back(static_cast<int>(permit_list[i]));
            }
        }

        if (!p_mover->Doors.permit_presets.empty()) {
            p_mover->Doors.permit_preset = permit_default;
            p_mover->Doors.permit_preset =
                    std::min<int>(
                            static_cast<int>(p_mover->Doors.permit_presets.size()), p_mover->Doors.permit_preset) -
                    1;
        }

        p_mover->Doors.open_rate = open_speed;
        p_mover->Doors.open_delay = open_delay;
        p_mover->Doors.close_rate = close_speed;
        p_mover->Doors.close_delay = close_delay;
        p_mover->Doors.range = max_shift;
        p_mover->Doors.range_out = max_shift_plug;

        if (door_type_map.find(type) != door_type_map.end()) {
            p_mover->Doors.type = door_type_map.at(type);
        } else {
            log_error("Unhandled door type: " + String::num(type));
        }

        p_mover->Doors.has_warning = close_warning;
        p_mover->Doors.has_autowarning = close_auto_close_warning;
        p_mover->Doors.has_lock = has_lock;
        bool const remote_control = {
                (open_method == CONTROLS_DRIVER || open_method == CONTROLS_CONDUCTOR || open_method == CONTROLS_MIXED)};

        if (voltage_map.find(voltage) != voltage_map.end()) {
            p_mover->Doors.voltage = voltage_map.at(voltage);
        } else {
            p_mover->Doors.voltage = remote_control ? 24 : 0;
        }
        p_mover->Doors.step_rate = platform_speed;
        p_mover->Doors.step_range = platform_max_shift;

        if (door_platform_type_map.find(platform_type) != door_platform_type_map.end()) {
            p_mover->Doors.step_type = door_platform_type_map.at(platform_type);
        }

        p_mover->MirrorMaxShift = mirror_max_shift;
        p_mover->MirrorVelClose = mirror_close_velocity;
        p_mover->DoorsOpenWithPermitAfter = open_with_permit;
        p_mover->DoorsPermitLightBlinking = permit_light_blinking;
    }
} // namespace godot
