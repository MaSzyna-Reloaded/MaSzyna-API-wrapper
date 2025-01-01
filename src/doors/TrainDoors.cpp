#include "TrainDoors.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void TrainDoors::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_type", "type"), &TrainDoors::set_type);
        ClassDB::bind_method(D_METHOD("get_type"), &TrainDoors::get_type);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, "Shift,Rotate,Fold,Plug"), "set_type",
                "get_type");
        ClassDB::bind_method(D_METHOD("set_open_time", "open_time"), &TrainDoors::set_open_time);
        ClassDB::bind_method(D_METHOD("get_open_time"), &TrainDoors::get_open_time);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/time"), "set_open_time", "get_open_time");
        ClassDB::bind_method(D_METHOD("set_open_speed", "open_speed"), &TrainDoors::set_open_speed);
        ClassDB::bind_method(D_METHOD("get_open_speed"), &TrainDoors::get_open_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/speed"), "set_open_speed", "get_open_speed");
        ClassDB::bind_method(D_METHOD("set_close_speed", "close_speed"), &TrainDoors::set_close_speed);
        ClassDB::bind_method(D_METHOD("get_close_speed"), &TrainDoors::get_close_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close/speed"), "set_close_speed", "get_close_speed");
        ClassDB::bind_method(D_METHOD("set_max_shift", "max_shift"), &TrainDoors::set_max_shift);
        ClassDB::bind_method(D_METHOD("get_max_shift"), &TrainDoors::get_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_shift"), "set_max_shift", "get_max_shift");
        ClassDB::bind_method(D_METHOD("set_open_method", "open_method"), &TrainDoors::set_open_method);
        ClassDB::bind_method(D_METHOD("get_open_method"), &TrainDoors::get_open_method);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "open/method", PROPERTY_HINT_ENUM, "Passenger,Automatic,Driver,Conductor,Mixed"),
                "set_open_method", "get_open_method");
        ClassDB::bind_method(D_METHOD("set_close_method", "close_method"), &TrainDoors::set_close_method);
        ClassDB::bind_method(D_METHOD("get_close_method"), &TrainDoors::get_close_method);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "close/method", PROPERTY_HINT_ENUM, "Passenger,Automatic,Driver,Conductor,Mixed"),
                "set_close_method", "get_close_method");
        ClassDB::bind_method(D_METHOD("set_voltage", "voltage"), &TrainDoors::set_voltage);
        ClassDB::bind_method(D_METHOD("get_voltage"), &TrainDoors::get_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "voltage", PROPERTY_HINT_ENUM, "Automatic,0V,12V,24V,112V"), "set_voltage",
                "get_voltage");
        ClassDB::bind_method(D_METHOD("set_close_warning", "close_warning"), &TrainDoors::set_close_warning);
        ClassDB::bind_method(D_METHOD("get_close_warning"), &TrainDoors::get_close_warning);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "close/warning"), "set_close_warning", "get_close_warning");
        ClassDB::bind_method(
                D_METHOD("set_auto_close_warning", "auto_close_warning"), &TrainDoors::set_auto_close_warning);
        ClassDB::bind_method(D_METHOD("get_auto_close_warning"), &TrainDoors::get_auto_close_warning);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "close/auto_close_warning"), "set_auto_close_warning",
                "get_auto_close_warning");
        ClassDB::bind_method(D_METHOD("set_open_delay", "open_delay"), &TrainDoors::set_open_delay);
        ClassDB::bind_method(D_METHOD("get_open_delay"), &TrainDoors::get_open_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/delay"), "set_open_delay", "get_open_delay");
        ClassDB::bind_method(D_METHOD("set_close_delay", "close_delay"), &TrainDoors::set_close_delay);
        ClassDB::bind_method(D_METHOD("get_close_delay"), &TrainDoors::get_close_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close/delay"), "set_close_delay", "get_close_delay");
        ClassDB::bind_method(D_METHOD("set_open_with_permit", "holding_time"), &TrainDoors::set_open_with_permit);
        ClassDB::bind_method(D_METHOD("get_open_with_permit"), &TrainDoors::get_open_with_permit);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/with_permit"), "set_open_with_permit", "get_open_with_permit");
        ClassDB::bind_method(D_METHOD("set_has_lock", "lock"), &TrainDoors::set_has_lock);
        ClassDB::bind_method(D_METHOD("get_has_lock"), &TrainDoors::get_has_lock);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "has_lock"), "set_has_lock", "get_has_lock");
        ClassDB::bind_method(D_METHOD("set_max_shift_plug", "max_shift_plug"), &TrainDoors::set_max_shift_plug);
        ClassDB::bind_method(D_METHOD("get_max_shift_plug"), &TrainDoors::get_max_shift_plug);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_shift_plug"), "set_max_shift_plug", "get_max_shift_plug");
        ClassDB::bind_method(D_METHOD("set_permit_list", "permit_list"), &TrainDoors::set_permit_list);
        ClassDB::bind_method(D_METHOD("get_permit_list"), &TrainDoors::get_permit_list);
        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "permit/list", PROPERTY_HINT_ARRAY_TYPE), "set_permit_list",
                "get_permit_list");
        ClassDB::bind_method(
                D_METHOD("set_permit_list_default", "permit_list_default"), &TrainDoors::set_permit_list_default);
        ClassDB::bind_method(D_METHOD("get_permit_list_default"), &TrainDoors::get_permit_list_default);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "permit/list_default"), "set_permit_list_default",
                "get_permit_list_default");
        ClassDB::bind_method(
                D_METHOD("set_auto_close_remote", "auto_close_remote"), &TrainDoors::set_auto_close_remote);
        ClassDB::bind_method(D_METHOD("get_auto_close_remote"), &TrainDoors::get_auto_close_remote);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "close/auto_close_remote"), "set_auto_close_remote",
                "get_auto_close_remote");
        ClassDB::bind_method(
                D_METHOD("set_auto_close_velocity", "auto_close_velocity"), &TrainDoors::set_auto_close_velocity);
        ClassDB::bind_method(D_METHOD("get_auto_close_velocity"), &TrainDoors::get_auto_close_velocity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "close/auto_close_velocity"), "set_auto_close_velocity",
                "get_auto_close_velocity");
        ClassDB::bind_method(
                D_METHOD("set_platform_max_speed", "platform_max_speed"), &TrainDoors::set_platform_max_speed);
        ClassDB::bind_method(D_METHOD("get_platform_max_speed"), &TrainDoors::get_platform_max_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "platform/max_speed"), "set_platform_max_speed", "get_platform_max_speed");
        ClassDB::bind_method(D_METHOD("set_platform_type", "platform_type"), &TrainDoors::set_platform_type);
        ClassDB::bind_method(D_METHOD("get_platform_type"), &TrainDoors::get_platform_type);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "platform/type", PROPERTY_HINT_ENUM, "Shift,Rotate"), "set_platform_type",
                "get_platform_type");
        ClassDB::bind_method(
                D_METHOD("set_platform_max_shift", "platform_max_shift"), &TrainDoors::set_platform_max_shift);
        ClassDB::bind_method(D_METHOD("get_platform_max_shift"), &TrainDoors::get_platform_max_shift);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "platform/max_shift"), "set_platform_max_shift", "get_platform_max_shift");
        ClassDB::bind_method(D_METHOD("set_platform_speed", "platform_speed"), &TrainDoors::set_platform_speed);
        ClassDB::bind_method(D_METHOD("get_platform_speed"), &TrainDoors::get_platform_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "platform/speed"), "set_platform_speed", "get_platform_speed");
        ClassDB::bind_method(D_METHOD("set_mirror_max_shift", "mirror_max_shift"), &TrainDoors::set_mirror_max_shift);
        ClassDB::bind_method(D_METHOD("get_mirror_max_shift"), &TrainDoors::get_mirror_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mirror/max_shift"), "set_mirror_max_shift", "get_mirror_max_shift");
        ClassDB::bind_method(
                D_METHOD("set_mirror_close_velocity", "mirror_close_velocity"), &TrainDoors::set_mirror_close_velocity);
        ClassDB::bind_method(D_METHOD("get_mirror_close_velocity"), &TrainDoors::get_mirror_close_velocity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "mirror/close_velocity"), "set_mirror_close_velocity",
                "get_mirror_close_velocity");
        ClassDB::bind_method(D_METHOD("set_permit_required", "permit_required"), &TrainDoors::set_permit_required);
        ClassDB::bind_method(D_METHOD("get_permit_required"), &TrainDoors::get_permit_required);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "permit/required"), "set_permit_required", "get_permit_required");
        ClassDB::bind_method(
                D_METHOD("set_permit_light_blinking", "blinking_mode"), &TrainDoors::set_permit_light_blinking);
        ClassDB::bind_method(D_METHOD("get_permit_light_blinking"), &TrainDoors::get_permit_light_blinking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "permit/light_blinking", PROPERTY_HINT_ENUM,
                        "Continuous light,Flashing on permission w/step,Flashing on permission,Flashing always"),
                "set_permit_light_blinking", "get_permit_light_blinking");

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
            log_error("Unhandled door open controls position: " + String::num(static_cast<int>(open_method)));
        }

        if (doorControlsMap.find(close_method) != doorControlsMap.end()) {
            mover->Doors.close_control = doorControlsMap.at(close_method);
        } else {
            log_error("Unhandled door close controls position: " + String::num(static_cast<int>(close_method)));
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
            log_error("Unhandled door type: " + String::num(static_cast<int>(type)));
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

    void TrainDoors::set_type(const Type p_type) {
        type = p_type;
        _dirty = true;
    }

    TrainDoors::Type TrainDoors::get_type() const {
        return type;
    }

    void TrainDoors::set_open_time(const float p_value) {
        open_time = p_value;
        _dirty = true;
    }

    float TrainDoors::get_open_time() const {
        return open_time;
    }

    void TrainDoors::set_open_speed(const float p_value) {
        open_speed = p_value;
        _dirty = true;
    }

    float TrainDoors::get_open_speed() const {
        return open_speed;
    }

    void TrainDoors::set_close_speed(const float p_value) {
        close_speed = p_value;
        _dirty = true;
    }

    float TrainDoors::get_close_speed() const {
        return close_speed;
    }

    void TrainDoors::set_max_shift(const float p_max_shift) {
        max_shift = p_max_shift;
        _dirty = true;
    }

    float TrainDoors::get_max_shift() const {
        return max_shift;
    }

    void TrainDoors::set_open_method(const Controls p_open_method) {
        open_method = p_open_method;
        _dirty = true;
    }

    TrainDoors::Controls TrainDoors::get_open_method() const {
        return open_method;
    }

    void TrainDoors::set_close_method(const Controls p_close_method) {
        close_method = p_close_method;
        _dirty = true;
    }

    TrainDoors::Controls TrainDoors::get_close_method() const {
        return close_method;
    }

    void TrainDoors::set_voltage(const Voltage p_voltage) {
        voltage = p_voltage;
        _dirty = true;
    }

    TrainDoors::Voltage TrainDoors::get_voltage() const {
        return voltage;
    }

    void TrainDoors::set_close_warning(const bool p_close_warning) {
        close_warning = p_close_warning;
        _dirty = true;
    }

    bool TrainDoors::get_close_warning() const {
        return close_warning;
    }

    void TrainDoors::set_auto_close_warning(const bool p_auto_close_warning) {
        auto_close_warning = p_auto_close_warning;
        _dirty = true;
    }

    bool TrainDoors::get_auto_close_warning() const {
        return auto_close_warning;
    }

    void TrainDoors::set_open_delay(const float p_open_delay) {
        open_delay = p_open_delay;
        _dirty = true;
    }

    float TrainDoors::get_open_delay() const {
        return open_delay;
    }

    void TrainDoors::set_close_delay(const float p_close_delay) {
        close_delay = p_close_delay;
        _dirty = true;
    }

    float TrainDoors::get_close_delay() const {
        return close_delay;
    }

    void TrainDoors::set_open_with_permit(const float p_holding_time) {
        open_with_permit = p_holding_time;
        _dirty = true;
    }

    float TrainDoors::get_open_with_permit() const {
        return open_with_permit;
    }

    void TrainDoors::set_has_lock(const bool p_has_lock) {
        has_lock = p_has_lock;
        _dirty = true;
    }

    bool TrainDoors::get_has_lock() const {
        return has_lock;
    }

    void TrainDoors::set_max_shift_plug(const float p_max_shift_plug) {
        max_shift = p_max_shift_plug;
        _dirty = true;
    }

    float TrainDoors::get_max_shift_plug() const {
        return max_shift;
    }

    void TrainDoors::set_permit_list(const Array &p_permit_list) {
        permit_list = p_permit_list;
        _dirty = true;
    }

    Array TrainDoors::get_permit_list() const {
        return permit_list;
    }

    void TrainDoors::set_permit_list_default(const int p_permit_list_default) {
        permit_list_default = p_permit_list_default;
        _dirty = true;
    }

    int TrainDoors::get_permit_list_default() const {
        return permit_list_default;
    }

    void TrainDoors::set_auto_close_remote(const bool p_auto_close) {
        auto_close_remote = p_auto_close;
        _dirty = true;
    }

    bool TrainDoors::get_auto_close_remote() const {
        return auto_close_remote;
    }

    void TrainDoors::set_auto_close_velocity(const float p_vel) {
        auto_close_velocity = p_vel;
        _dirty = true;
    }

    float TrainDoors::get_auto_close_velocity() const {
        return auto_close_velocity;
    }

    void TrainDoors::set_platform_max_speed(const double p_max_speed) {
        platform_max_speed = p_max_speed;
        _dirty = true;
    }

    double TrainDoors::get_platform_max_speed() const {
        return platform_max_speed;
    }

    void TrainDoors::set_platform_max_shift(const float p_max_shift) {
        platform_max_shift = p_max_shift;
        _dirty = true;
    }

    float TrainDoors::get_platform_max_shift() const {
        return platform_max_shift;
    }

    void TrainDoors::set_platform_speed(const float p_speed) {
        platform_speed = p_speed;
        _dirty = true;
    }

    float TrainDoors::get_platform_speed() const {
        return platform_speed;
    }

    void TrainDoors::set_platform_type(const PlatformType p_type) {
        platform_type = p_type;
        _dirty = true;
    }

    TrainDoors::PlatformType TrainDoors::get_platform_type() const {
        return platform_type;
    }

    void TrainDoors::set_mirror_max_shift(const double p_max_shift) {
        mirror_max_shift = p_max_shift;
        _dirty = true;
    }

    double TrainDoors::get_mirror_max_shift() const {
        return mirror_max_shift;
    }

    void TrainDoors::set_mirror_close_velocity(const double p_close_velocity) {
        mirror_close_velocity = p_close_velocity;
        _dirty = true;
    }

    double TrainDoors::get_mirror_close_velocity() const {
        return mirror_close_velocity;
    }

    void TrainDoors::set_permit_required(const bool p_permit_required) {
        permit_required = p_permit_required;
        _dirty = true;
    }

    bool TrainDoors::get_permit_required() const {
        return permit_required;
    }

    void TrainDoors::set_permit_light_blinking(const PermitLight p_blinking_mode) {
        permit_light_blinking = p_blinking_mode;
        _dirty = true;
    }

    TrainDoors::PermitLight TrainDoors::get_permit_light_blinking() const {
        return permit_light_blinking;
    }
} // namespace godot
