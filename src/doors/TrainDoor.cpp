#include "TrainDoor.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    TrainDoor::TrainDoor() = default;

    void TrainDoor::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_door_type", "type"), &TrainDoor::set_door_type);
        ClassDB::bind_method(D_METHOD("get_door_type"), &TrainDoor::get_door_type);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM,
                        "Shift,Rotate,Fold,Plug"), "set_door_type", "get_door_type");
        ClassDB::bind_method(D_METHOD("set_door_open_time", "open_time"), &TrainDoor::set_door_open_time);
        ClassDB::bind_method(D_METHOD("get_door_open_time"), &TrainDoor::get_door_open_time);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/time"), "set_door_open_time", "get_door_open_time");
        ClassDB::bind_method(D_METHOD("set_open_speed", "open_speed"), &TrainDoor::set_open_speed);
        ClassDB::bind_method(D_METHOD("get_open_speed"), &TrainDoor::get_open_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/speed"), "set_open_speed", "get_open_speed");
        ClassDB::bind_method(D_METHOD("set_close_speed", "close_speed"), &TrainDoor::set_close_speed);
        ClassDB::bind_method(D_METHOD("get_close_speed"), &TrainDoor::get_close_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close/speed"), "set_close_speed", "get_close_speed");
        ClassDB::bind_method(D_METHOD("set_door_max_shift", "door_max_shift"), &TrainDoor::set_door_max_shift);
        ClassDB::bind_method(D_METHOD("get_door_max_shift"), &TrainDoor::get_door_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_shift"), "set_door_max_shift", "get_door_max_shift");
        ClassDB::bind_method(D_METHOD("set_door_open_method", "door_open_method"), &TrainDoor::set_door_open_method);
        ClassDB::bind_method(D_METHOD("get_door_open_method"), &TrainDoor::get_door_open_method);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "open/method", PROPERTY_HINT_ENUM,
                        "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_door_open_method", "get_door_open_method");
        ClassDB::bind_method(D_METHOD("set_door_close_method", "door_close_method"), &TrainDoor::set_door_close_method);
        ClassDB::bind_method(D_METHOD("get_door_close_method"), &TrainDoor::get_door_close_method);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "close/method", PROPERTY_HINT_ENUM,
                        "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_door_close_method", "get_door_close_method");
        ClassDB::bind_method(D_METHOD("set_door_voltage", "door_voltage"), &TrainDoor::set_door_voltage);
        ClassDB::bind_method(D_METHOD("get_door_voltage"), &TrainDoor::get_door_voltage);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "voltage", PROPERTY_HINT_ENUM,
                        "0V,12V,24V,112V"), "set_door_voltage", "get_door_voltage");
        ClassDB::bind_method(
                D_METHOD("set_door_close_warning", "door_close_warning"), &TrainDoor::set_door_close_warning);
        ClassDB::bind_method(D_METHOD("get_door_close_warning"), &TrainDoor::get_door_close_warning);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "close/warning"), "set_door_close_warning", "get_door_close_warning");
        ClassDB::bind_method(
                D_METHOD("set_auto_door_close_warning", "auto_door_close_warning"),
                &TrainDoor::set_auto_door_close_warning);
        ClassDB::bind_method(D_METHOD("get_auto_door_close_warning"), &TrainDoor::get_auto_door_close_warning);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "auto_close/warning"), "set_auto_door_close_warning",
                "get_auto_door_close_warning");
        ClassDB::bind_method(D_METHOD("set_door_open_delay", "door_open_delay"), &TrainDoor::set_door_open_delay);
        ClassDB::bind_method(D_METHOD("get_door_open_delay"), &TrainDoor::get_door_open_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/delay"), "set_door_open_delay", "get_door_open_delay");
        ClassDB::bind_method(D_METHOD("set_door_close_delay", "door_close_delay"), &TrainDoor::set_door_close_delay);
        ClassDB::bind_method(D_METHOD("get_door_close_delay"), &TrainDoor::get_door_close_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close/delay"), "set_door_close_delay", "get_door_close_delay");
        ClassDB::bind_method(
                D_METHOD("set_door_open_with_permit", "holding_time"), &TrainDoor::set_door_open_with_permit);
        ClassDB::bind_method(D_METHOD("get_door_open_with_permit"), &TrainDoor::get_door_open_with_permit);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "open/with_permit"), "set_door_open_with_permit",
                "get_door_open_with_permit");
        ClassDB::bind_method(D_METHOD("set_door_blocked", "blocked"), &TrainDoor::set_door_blocked);
        ClassDB::bind_method(D_METHOD("get_door_blocked"), &TrainDoor::get_door_blocked);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocked"), "set_door_blocked", "get_door_blocked");
        ClassDB::bind_method(
                D_METHOD("set_door_max_shift_plug", "door_max_shift_plug"), &TrainDoor::set_door_max_shift_plug);
        ClassDB::bind_method(D_METHOD("get_door_max_shift_plug"), &TrainDoor::get_door_max_shift_plug);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_shift_plug"), "set_door_max_shift_plug", "get_door_max_shift_plug");
        ClassDB::bind_method(D_METHOD("set_door_permit_list", "door_permit_list"), &TrainDoor::set_door_permit_list);
        ClassDB::bind_method(D_METHOD("get_door_permit_list"), &TrainDoor::get_door_permit_list);
        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "permit/list", PROPERTY_HINT_ARRAY_TYPE), "set_door_permit_list", "get_door_permit_list");
        ClassDB::bind_method(
                D_METHOD("set_door_permit_list_default", "door_permit_list_default"),
                &TrainDoor::set_door_permit_list_default);
        ClassDB::bind_method(D_METHOD("get_door_permit_list_default"), &TrainDoor::get_door_permit_list_default);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "permit/default"), "set_door_permit_list_default",
                "get_door_permit_list_default");
        ClassDB::bind_method(
                D_METHOD("set_door_auto_close_remote", "auto_close_remote"), &TrainDoor::set_door_auto_close_remote);
        ClassDB::bind_method(D_METHOD("get_door_auto_close_remote"), &TrainDoor::get_door_auto_close_remote);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "auto_close/remote"), "set_door_auto_close_remote",
                "get_door_auto_close_remote");
        ClassDB::bind_method(
                D_METHOD("set_door_auto_close_velocity", "auto_close_velocity"), &TrainDoor::set_door_auto_close_velocity);
        ClassDB::bind_method(D_METHOD("get_door_auto_close_velocity"), &TrainDoor::get_door_auto_close_velocity);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "auto_close/velocity"), "set_door_auto_close_velocity", "get_door_auto_close_velocity");
        ClassDB::bind_method(
                D_METHOD("set_door_auto_close_enabled", "auto_close_enabled"), &TrainDoor::set_door_auto_close_enabled);
        ClassDB::bind_method(D_METHOD("get_door_auto_close_enabled"), &TrainDoor::get_door_auto_close_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "auto_close/enabled"), "set_door_auto_close_enabled", "get_door_auto_close_enabled");
        ClassDB::bind_method(
                D_METHOD("set_door_platform_max_speed", "platform_max_speed"), &TrainDoor::set_door_platform_max_speed);
        ClassDB::bind_method(D_METHOD("get_door_platform_max_speed"), &TrainDoor::get_door_platform_max_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "platform/max_speed"), "set_door_platform_max_speed",
                "get_door_platform_max_speed");
        ClassDB::bind_method(D_METHOD("set_platform_open_method", "platform_open_method"), &TrainDoor::set_platform_open_method);
        ClassDB::bind_method(D_METHOD("get_platform_open_method"), &TrainDoor::get_platform_open_method);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "platform/open_method", PROPERTY_HINT_ENUM, "Shift,Rotate"), "set_platform_open_method", "get_platform_open_method");
        ClassDB::bind_method(
                D_METHOD("set_door_platform_max_shift", "platform_max_shift"), &TrainDoor::set_door_platform_max_shift);
        ClassDB::bind_method(D_METHOD("get_door_platform_max_shift"), &TrainDoor::get_door_platform_max_shift);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "platform/max_shift"), "set_door_platform_max_shift",
                "get_door_platform_max_shift");
        ClassDB::bind_method(
                D_METHOD("set_door_platform_speed", "platform_speed"), &TrainDoor::set_door_platform_speed);
        ClassDB::bind_method(D_METHOD("get_door_platform_speed"), &TrainDoor::get_door_platform_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "platform/speed"), "set_door_platform_speed", "get_door_platform_speed");
        ClassDB::bind_method(D_METHOD("set_mirror_max_shift", "mirror_max_shift"), &TrainDoor::set_mirror_max_shift);
        ClassDB::bind_method(D_METHOD("get_mirror_max_shift"), &TrainDoor::get_mirror_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mirror/max_shift"), "set_mirror_max_shift", "get_mirror_max_shift");
        ClassDB::bind_method(D_METHOD("set_mirror_vel_close", "mirror_vel_close"), &TrainDoor::set_mirror_vel_close);
        ClassDB::bind_method(D_METHOD("get_mirror_vel_close"), &TrainDoor::get_mirror_vel_close);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mirror/vel_close"), "set_mirror_vel_close", "get_mirror_vel_close");
        ClassDB::bind_method(D_METHOD("set_door_permit_required", "permit_required"), &TrainDoor::set_door_permit_required);
        ClassDB::bind_method(D_METHOD("get_door_permit_required"), &TrainDoor::get_door_permit_required);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "permit/required"), "set_door_permit_required", "get_door_permit_required");
        ClassDB::bind_method(
                D_METHOD("set_door_permit_light_blinking", "blinking_mode"),
                &TrainDoor::set_door_permit_light_blinking);
        ClassDB::bind_method(D_METHOD("get_door_permit_light_blinking"), &TrainDoor::get_door_permit_light_blinking);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "permit/light_blinking", PROPERTY_HINT_ENUM, "ContinuousLight,FlashingOnPermissionWithStep,FlashingOnPermission,FlashingAlways"), "set_door_permit_light_blinking",
                "get_door_permit_light_blinking");

        BIND_ENUM_CONSTANT(PERMIT_LIGHT_CONTINUOUS_LIGHT);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ON_PERMISSION);
        BIND_ENUM_CONSTANT(PERMIT_LIGHT_FLASHING_ALWAYS);

        BIND_ENUM_CONSTANT(PLATFORM_ANIMATION_TYPE_SHIFT);
        BIND_ENUM_CONSTANT(PLATFORM_ANIMATION_TYPE_ROTATE);

        BIND_ENUM_CONSTANT(DOOR_SIDE_RIGHT)
        BIND_ENUM_CONSTANT(DOOR_SIDE_LEFT)

        BIND_ENUM_CONSTANT(NOTIFICATION_RANGE_LOCAL)
        BIND_ENUM_CONSTANT(NOTIFICATION_RANGE_UNIT)
        BIND_ENUM_CONSTANT(NOTIFICATION_RANGE_CONSIST)

        BIND_ENUM_CONSTANT(DOOR_STATE_CLOSE)
        BIND_ENUM_CONSTANT(DOOR_STATE_OPEN)

        BIND_ENUM_CONSTANT(DOOR_CONTROLS_PASSENGER)
        BIND_ENUM_CONSTANT(DOOR_CONTROLS_AUTOMATIC)
        BIND_ENUM_CONSTANT(DOOR_CONTROLS_DRIVER)
        BIND_ENUM_CONSTANT(DOOR_CONTROLS_CONDUCTOR)
        BIND_ENUM_CONSTANT(DOOR_CONTROLS_MIXED)

        BIND_ENUM_CONSTANT(DOOR_VOLTAGE_0)
        BIND_ENUM_CONSTANT(DOOR_VOLTAGE_12)
        BIND_ENUM_CONSTANT(DOOR_VOLTAGE_24)
        BIND_ENUM_CONSTANT(DOOR_VOLTAGE_112)

        BIND_ENUM_CONSTANT(DOOR_TYPE_SHIFT)
        BIND_ENUM_CONSTANT(DOOR_TYPE_ROTATE)
        BIND_ENUM_CONSTANT(DOOR_TYPE_FOLD)
        BIND_ENUM_CONSTANT(DOOR_TYPE_PLUG)
    }

    void TrainDoor::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const auto left_door = mover->Doors.instances[side::left];
        const auto right_door = mover->Doors.instances[side::right];

        state["door/has_warning"] = mover->Doors.has_warning;
        state["door/has_auto_warning"] = mover->Doors.has_autowarning;
        state["door/has_lock"] = mover->Doors.has_lock;
        state["door/is_locked"] = mover->Doors.is_locked;
        state["door/voltage"] = mover->Doors.voltage;
        state["door/step_rate"] = mover->Doors.step_rate;
        state["door/step_range"] = mover->Doors.step_range;
        state["door/step_type"] = mover->Doors.step_type;
        state["door/step_enabled"] = mover->Doors.step_enabled;
        state["door/open_with_permit_after"] = mover->DoorsOpenWithPermitAfter;
        state["door/permit_light_blinking"] = mover->DoorsPermitLightBlinking;

        state["door/left/open_permit"] = left_door.open_permit;
        state["door/left/local_open"] = left_door.local_open;
        state["door/left/local_close"] = left_door.local_close;
        state["door/left/remote_open"] = left_door.remote_open;
        state["door/left/remote_close"] = left_door.remote_close;
        state["door/left/close_delay"] = left_door.close_delay;
        state["door/left/open_delay"] = left_door.open_delay;
        state["door/left/position"] = left_door.position;
        state["door/left/step_position"] = left_door.step_position;
        state["door/left/is_closed"] = left_door.is_closed;
        state["door/left/is_door_closed"] = left_door.is_door_closed;
        state["door/left/is_closing"] = left_door.is_closing;
        state["door/left/is_opening"] = left_door.is_opening;
        state["door/left/is_open"] = left_door.is_open;
        state["door/left/step_unfolding"] = left_door.step_unfolding;
        state["door/left/step_folding"] = left_door.step_folding;

        state["door/right/open_permit"] = right_door.open_permit;
        state["door/right/local_open"] = right_door.local_open;
        state["door/right/local_close"] = right_door.local_close;
        state["door/right/remote_open"] = right_door.remote_open;
        state["door/right/remote_close"] = right_door.remote_close;
        state["door/right/close_delay"] = right_door.close_delay;
        state["door/right/open_delay"] = right_door.open_delay;
        state["door/right/position"] = right_door.position;
        state["door/right/step_position"] = right_door.step_position;
        state["door/right/is_closed"] = right_door.is_closed;
        state["door/right/is_door_closed"] = right_door.is_door_closed;
        state["door/right/is_closing"] = right_door.is_closing;
        state["door/right/is_opening"] = right_door.is_opening;
        state["door/right/is_open"] = right_door.is_open;
        state["door/right/step_unfolding"] = right_door.step_unfolding;
        state["door/right/step_folding"] = right_door.step_folding;
    }

    void TrainDoor::_do_process_mover(TMoverParameters *mover, double delta) {
        mover->update_doors(delta); //działaj plz qwp
    }

    void TrainDoor::_on_command_received(const String &command, const Variant &p1, const Variant &p2) {
        TrainPart::_on_command_received(command, p1, p2);
        if (train_controller_node == nullptr) {
            return;
        }

        TMoverParameters *mover = train_controller_node->get_mover();

        if (command == "doors_change_control_mode") {
            //@TODO: add support for changing range_t param
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] State parameter (argument #1) is out of range");
                return;
            }
            mover->ChangeDoorControlMode(param_state);
        }

        if (command == "doors_change_permit_preset") {
            //@TODO: add support for changing range_t param
            const int param_change = p1;
            if (param_change > 1 || param_change < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] State parameter (argument #1) is out of range");
                return;
            }
            mover->ChangeDoorPermitPreset(param_change);
        }

        if (command == "doors_left_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->OperateDoors(side::left, param_state);
            return;
        }

        if (command == "doors_right_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->OperateDoors(side::right, param_state);
            return;
        }


        if (command == "doors_lock_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->LockDoors(param_state);
            return;
        }

        if (command == "doors_left_permit_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->PermitDoors(side::left, param_state);
            return;
        }

        if (command == "doors_right_permit_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->PermitDoors(side::right, param_state);
            return;
        }

        if (command == "doors_step_permit_toggle") {
            const int param_state = p1;
            if (param_state > 1 || param_state < 0) {
                UtilityFunctions::push_error("[MaSzyna::Doors] Side parameter (argument #1) is out of range");
                return;
            }

            mover->PermitDoorStep(param_state);
            return;
        }

        UtilityFunctions::push_error("[MaSzyna::Doors] Unknown command: " + command);
    }

    void TrainDoor::_do_update_internal_mover(TMoverParameters *mover) {
        mover->Doors.open_control = static_cast<control_t>(door_open_method);
        mover->Doors.close_control = static_cast<control_t>(door_close_method);
        mover->Doors.auto_duration = door_open_time;
        mover->Doors.auto_velocity = door_auto_close_enabled ? door_auto_close_velocity : -1.0f;
        mover->Doors.auto_include_remote = door_auto_close_remote;
        mover->Doors.permit_needed = door_permit_required;
        mover->Doors.permit_presets.clear();
        for (int i = 0; i < door_permit_list.size(); i++) {
            if (door_permit_list[i] != Variant()) {
                mover->Doors.permit_presets.emplace_back(static_cast<int>(door_permit_list[i]));
            }
        }

        if (!mover->Doors.permit_presets.empty()) {
            mover->Doors.permit_preset = door_permit_list_default;
            mover->Doors.permit_preset =
                    std::min<int>(mover->Doors.permit_presets.size(), mover->Doors.permit_preset) - 1;
        }

        mover->Doors.open_rate = open_speed;
        mover->Doors.open_delay = door_open_delay;
        mover->Doors.close_rate = close_speed;
        mover->Doors.close_delay = door_close_delay;
        mover->Doors.range = door_max_shift;
        mover->Doors.range_out = door_max_shift_plug;;
        mover->Doors.type = door_type;
        mover->Doors.has_warning = door_close_warning;
        mover->Doors.has_autowarning = auto_door_close_warning;
        mover->Doors.has_lock = door_blocked;
        bool const remote_door_control {
                mover->Doors.open_control == control_t::driver ||
                mover->Doors.open_control == control_t::conductor || mover->Doors.open_control == control_t::mixed };
        door_voltage = remote_door_control ? DoorVoltage::DOOR_VOLTAGE_24 : DoorVoltage::DOOR_VOLTAGE_0;
        mover->Doors.voltage = door_voltage;
        mover->Doors.step_rate = platform_speed;
        mover->Doors.step_range = platform_max_shift;
        if (platform_open_method == PlatformAnimationType::PLATFORM_ANIMATION_TYPE_SHIFT) {
            mover->Doors.step_type = 1;
        }

        mover->MirrorMaxShift = mirror_max_shift;
        mover->MirrorVelClose = mirror_vel_close;
        mover->DoorsOpenWithPermitAfter = door_open_with_permit;
        mover->DoorsPermitLightBlinking = door_permit_light_blinking;
    }

    void TrainDoor::set_door_type(const DoorType p_door_type) {
        door_type = p_door_type;
        _dirty = true;
    }

    TrainDoor::DoorType TrainDoor::get_door_type() const {
        return door_type;
    }

    void TrainDoor::set_door_open_time(const float p_value) {
        door_open_time = p_value;
        _dirty = true;
    }

    float TrainDoor::get_door_open_time() const {
        return door_open_time;
    }

    void TrainDoor::set_open_speed(const float p_value) {
        open_speed = p_value;
        _dirty = true;
    }

    float TrainDoor::get_open_speed() const {
        return open_speed;
    }

    void TrainDoor::set_close_speed(const float p_value) {
        close_speed = p_value;
        _dirty = true;
    }

    float TrainDoor::get_close_speed() const {
        return close_speed;
    }

    void TrainDoor::set_door_max_shift(const float p_max_shift) {
        door_max_shift = p_max_shift;
        _dirty = true;
    }

    float TrainDoor::get_door_max_shift() const {
        return door_max_shift;
    }

    void TrainDoor::set_door_open_method(const DoorControls p_open_method) {
        door_open_method = p_open_method;
        _dirty = true;
    }

    TrainDoor::DoorControls TrainDoor::get_door_open_method() const {
        return door_open_method;
    }

    void TrainDoor::set_door_close_method(const DoorControls p_close_method) {
        door_close_method = p_close_method;
        _dirty = true;
    }

    TrainDoor::DoorControls TrainDoor::get_door_close_method() const {
        return door_close_method;
    }

    void TrainDoor::set_door_voltage(const DoorVoltage p_voltage) {
        door_voltage = p_voltage;
        _dirty = true;
    }

    TrainDoor::DoorVoltage TrainDoor::get_door_voltage() const {
        return door_voltage;
    }

    void TrainDoor::set_door_close_warning(const bool p_close_warning) {
        door_close_warning = p_close_warning;
        _dirty = true;
    }

    bool TrainDoor::get_door_close_warning() const {
        return door_close_warning;
    }

    void TrainDoor::set_auto_door_close_warning(const bool p_auto_close_warning) {
        auto_door_close_warning = p_auto_close_warning;
        _dirty = true;
    }

    bool TrainDoor::get_auto_door_close_warning() const {
        return auto_door_close_warning;
    }

    void TrainDoor::set_door_open_delay(const float p_open_delay) {
        door_open_delay = p_open_delay;
        _dirty = true;
    }

    float TrainDoor::get_door_open_delay() const {
        return door_open_delay;
    }

    void TrainDoor::set_door_close_delay(const float p_close_delay) {
        door_close_delay = p_close_delay;
        _dirty = true;
    }

    float TrainDoor::get_door_close_delay() const {
        return door_close_delay;
    }

    void TrainDoor::set_door_open_with_permit(const float p_holding_time) {
        door_open_with_permit = p_holding_time;
        _dirty = true;
    }

    float TrainDoor::get_door_open_with_permit() const {
        return door_open_with_permit;
    }

    void TrainDoor::set_door_blocked(const bool p_blocked) {
        door_blocked = p_blocked;
        _dirty = true;
    }

    bool TrainDoor::get_door_blocked() const {
        return door_blocked;
    }

    void TrainDoor::set_door_max_shift_plug(const float p_max_shift_plug) {
        door_max_shift = p_max_shift_plug;
        _dirty = true;
    }

    float TrainDoor::get_door_max_shift_plug() const {
        return door_max_shift;
    }

    void TrainDoor::set_door_permit_list(const Array& p_permit_list) {
        door_permit_list = p_permit_list;
        _dirty = true;
    }

    Array TrainDoor::get_door_permit_list() const {
        return door_permit_list;
    }

    void TrainDoor::set_door_permit_list_default(const int p_permit_list_default) {
        door_permit_list_default = p_permit_list_default;
        _dirty = true;
    }

    int TrainDoor::get_door_permit_list_default() const {
        return door_permit_list_default;
    }

    void TrainDoor::set_door_auto_close_remote(const bool p_auto_close) {
        door_auto_close_remote = p_auto_close;
        _dirty = true;
    }

    bool TrainDoor::get_door_auto_close_remote() const {
        return door_auto_close_remote;
    }

    void TrainDoor::set_door_auto_close_velocity(const float p_vel) {
        door_auto_close_velocity = p_vel;
        _dirty = true;
    }

    float TrainDoor::get_door_auto_close_velocity() const {
        return door_auto_close_velocity;
    }

    void TrainDoor::set_door_auto_close_enabled(const bool p_enabled) {
        door_auto_close_enabled = p_enabled;
        _dirty = true;
    }

    bool TrainDoor::get_door_auto_close_enabled() const {
        return door_auto_close_enabled;
    }

    void TrainDoor::set_door_platform_max_speed(const double p_max_speed) {
        platform_max_speed = p_max_speed;
        _dirty = true;
    }

    double TrainDoor::get_door_platform_max_speed() const {
        return platform_max_speed;
    }

    void TrainDoor::set_door_platform_max_shift(const float p_max_shift) {
        platform_max_shift = p_max_shift;
        _dirty = true;
    }

    float TrainDoor::get_door_platform_max_shift() const {
        return platform_max_shift;
    }

    void TrainDoor::set_door_platform_speed(const float p_speed) {
        platform_speed = p_speed;
        _dirty = true;
    }

    float TrainDoor::get_door_platform_speed() const {
        return platform_speed;
    }

    void TrainDoor::set_platform_open_method(const PlatformAnimationType p_shift_method) {
        platform_open_method = p_shift_method;
        _dirty = true;
    }

    int TrainDoor::get_platform_open_method() const {
        return platform_open_method;
    }

    void TrainDoor::set_mirror_max_shift(const double p_max_shift) {
        mirror_max_shift = p_max_shift;
        _dirty = true;
    }

    double TrainDoor::get_mirror_max_shift() const {
        return mirror_max_shift;
    }

    void TrainDoor::set_mirror_vel_close(const double p_vel_close) {
        mirror_vel_close = p_vel_close;
        _dirty = true;
    }

    double TrainDoor::get_mirror_vel_close() const {
        return mirror_vel_close;
    }

    void TrainDoor::set_door_permit_required(const bool p_permit_required) {
        door_permit_required = p_permit_required;
        _dirty = true;
    }

    bool TrainDoor::get_door_permit_required() const {
        return door_permit_required;
    }

    void TrainDoor::set_door_permit_light_blinking(const PermitLights p_blinking_mode) {
        door_permit_light_blinking = p_blinking_mode;
        _dirty = true;
    }

    int TrainDoor::get_door_permit_light_blinking() const {
        return door_permit_light_blinking;
    }
} // namespace godot
