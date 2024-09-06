#include "TrainDoor.hpp"

namespace godot {
    void TrainDoor::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_close_control", "close_control"), &TrainDoor::set_close_control);
        ClassDB::bind_method(D_METHOD("get_close_control"), &TrainDoor::get_close_control);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "close/control", PROPERTY_HINT_ENUM, "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_close_control", "get_close_control");
        ClassDB::bind_method(D_METHOD("set_open_control", "open_control"), &TrainDoor::set_open_control);
        ClassDB::bind_method(D_METHOD("get_open_control"), &TrainDoor::get_open_control);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "open/control", PROPERTY_HINT_ENUM, "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_open_control", "get_open_control");
        ClassDB::bind_method(D_METHOD("set_door_stay_open", "open_control"), &TrainDoor::set_door_stay_open);
        ClassDB::bind_method(D_METHOD("get_door_stay_open"), &TrainDoor::get_door_stay_open);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "stay_open"), "set_door_stay_open", "get_door_stay_open");
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
        ADD_PROPERTY(PropertyInfo(Variant::INT, "open/method"), "set_door_open_method", "get_door_open_method");
        ClassDB::bind_method(D_METHOD("set_door_voltage", "door_voltage"), &TrainDoor::set_door_voltage);
        ClassDB::bind_method(D_METHOD("get_door_voltage"), &TrainDoor::get_door_voltage);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "voltage"), "set_door_voltage", "get_door_voltage");
        ClassDB::bind_method(D_METHOD("set_door_closure_warning", "door_closure_warning"), &TrainDoor::set_door_closure_warning);
        ClassDB::bind_method(D_METHOD("get_door_closure_warning"), &TrainDoor::get_door_closure_warning);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "closure/warning"), "set_door_closure_warning", "get_door_closure_warning");
        ClassDB::bind_method(D_METHOD("set_auto_door_closure_warning", "auto_door_closure_warning"), &TrainDoor::set_auto_door_closure_warning);
        ClassDB::bind_method(D_METHOD("get_auto_door_closure_warning"), &TrainDoor::get_auto_door_closure_warning);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_closure/warning"), "auto_set_door_closure_warning", "get_auto_door_closure_warning");
        ClassDB::bind_method(D_METHOD("set_door_open_delay", "door_open_delay"), &TrainDoor::set_door_open_delay);
        ClassDB::bind_method(D_METHOD("get_door_open_delay"), &TrainDoor::get_door_open_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/delay"), "set_door_open_delay", "get_door_open_delay");
        ClassDB::bind_method(D_METHOD("set_door_close_delay", "door_close_delay"), &TrainDoor::set_door_close_delay);
        ClassDB::bind_method(D_METHOD("get_door_close_delay"), &TrainDoor::get_door_close_delay);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close/delay"), "set_door_close_delay", "get_door_close_delay");
        ClassDB::bind_method(D_METHOD("set_door_open_with_permit", "holding_time"), &TrainDoor::set_door_open_with_permit);
        ClassDB::bind_method(D_METHOD("get_door_open_with_permit"), &TrainDoor::get_door_open_with_permit);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open/with_permit"), "set_door_open_with_permit", "get_door_open_with_permit");
        ClassDB::bind_method(D_METHOD("set_door_blocked", "blocked"), &TrainDoor::set_door_blocked);
        ClassDB::bind_method(D_METHOD("get_door_blocked"), &TrainDoor::get_door_blocked);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocked"), "set_door_blocked", "get_door_blocked");
        ClassDB::bind_method(D_METHOD("set_door_max_shift_plug", "door_max_shift_plug"), &TrainDoor::set_door_max_shift_plug);
        ClassDB::bind_method(D_METHOD("get_door_max_shift_plug"), &TrainDoor::get_door_max_shift_plug);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_shift_plug"), "set_door_max_shift_plug", "get_door_max_shift_plug");
        ClassDB::bind_method(D_METHOD("set_door_permit_list", "door_permit_list"), &TrainDoor::set_door_permit_list);
        ClassDB::bind_method(D_METHOD("get_door_permit_list"), &TrainDoor::get_door_permit_list);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "permit/list"), "set_door_permit_list", "get_door_permit_list");
        ClassDB::bind_method(D_METHOD("set_door_permit_list_default", "door_permit_list_default"), &TrainDoor::set_door_permit_list_default);
        ClassDB::bind_method(D_METHOD("get_door_permit_list_default"), &TrainDoor::get_door_permit_list_default);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "permit/default"), "set_door_permit_list_default", "get_door_permit_list_default");
        ClassDB::bind_method(D_METHOD("set_door_auto_close_remote", "auto_close_remote"), &TrainDoor::set_door_auto_close_remote);
        ClassDB::bind_method(D_METHOD("get_door_auto_close_remote"), &TrainDoor::get_door_auto_close_remote);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_close/remote"), "set_door_auto_close_remote", "get_door_auto_close_remote");
        ClassDB::bind_method(D_METHOD("set_door_auto_close_vel", "auto_close_vel"), &TrainDoor::set_door_auto_close_vel);
        ClassDB::bind_method(D_METHOD("get_door_auto_close_vel"), &TrainDoor::get_door_auto_close_vel);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "auto_close/vel"), "set_door_auto_close_vel", "get_door_auto_close_vel");
        ClassDB::bind_method(D_METHOD("set_door_platform_max_speed", "platform_max_speed"), &TrainDoor::set_door_platform_max_speed);
        ClassDB::bind_method(D_METHOD("get_door_platform_max_speed"), &TrainDoor::get_door_platform_max_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "platform/max_speed"), "set_door_platform_max_speed", "get_door_platform_max_speed");
        ClassDB::bind_method(D_METHOD("set_door_platform_max_shift", "platform_max_shift"), &TrainDoor::set_door_platform_max_shift);
        ClassDB::bind_method(D_METHOD("get_door_platform_max_shift"), &TrainDoor::get_door_platform_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "platform/max_shift"), "set_door_platform_max_shift", "get_door_platform_max_shift");
        ClassDB::bind_method(D_METHOD("set_door_platform_speed", "platform_speed"), &TrainDoor::set_door_platform_speed);
        ClassDB::bind_method(D_METHOD("get_door_platform_speed"), &TrainDoor::get_door_platform_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "platform/speed"), "set_door_platform_speed", "get_door_platform_speed");
        ClassDB::bind_method(D_METHOD("set_mirror_max_shift", "mirror_max_shift"), &TrainDoor::set_mirror_max_shift);
        ClassDB::bind_method(D_METHOD("get_mirror_max_shift"), &TrainDoor::get_mirror_max_shift);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mirror/max_shift"), "set_mirror_max_shift", "get_mirror_max_shift");
        ClassDB::bind_method(D_METHOD("set_mirror_vel_close", "mirror_vel_close"), &TrainDoor::set_mirror_vel_close);
        ClassDB::bind_method(D_METHOD("get_mirror_vel_close"), &TrainDoor::get_mirror_vel_close);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mirror/vel_close"), "set_mirror_vel_close", "get_mirror_vel_close");
        ClassDB::bind_method(D_METHOD("set_door_needs_permit", "needs_permit"), &TrainDoor::set_door_needs_permit);
        ClassDB::bind_method(D_METHOD("get_door_needs_permit"), &TrainDoor::get_door_needs_permit);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "needs_permit"), "set_door_needs_permit", "get_door_needs_permit");
        ClassDB::bind_method(D_METHOD("set_door_permit_light_blinking", "blinking_mode"), &TrainDoor::set_door_permit_light_blinking);
        ClassDB::bind_method(D_METHOD("get_door_permit_light_blinking"), &TrainDoor::get_door_permit_light_blinking);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "permit/light_blinking"), "set_door_permit_light_blinking", "get_door_permit_light_blinking");
    }
    void TrainDoor::set_close_control(const int p_value) {
        close_control = p_value;
    }

    int TrainDoor::get_close_control() const {
        return close_control;
    }

    void TrainDoor::set_open_control(const int p_value) {
        open_control = p_value;
    }

    int TrainDoor::get_open_control() const {
        return open_control;
    }

    void TrainDoor::set_door_stay_open(const double p_value) {
        door_stay_open = p_value;
    }

    double TrainDoor::get_door_stay_open() const {
        return door_stay_open;
    }

    void TrainDoor::set_open_speed(const double p_value) {
        open_speed = p_value;
    }

    double TrainDoor::get_open_speed() const {
        return open_speed;
    }

    void TrainDoor::set_close_speed(const double p_value) {
        close_speed = p_value;
    }

    double TrainDoor::get_close_speed() const {
        return close_speed;
    }

    void TrainDoor::set_door_max_shift(const double p_max_shift) {
        door_max_shift = p_max_shift;
    }

    double TrainDoor::get_door_max_shift() const {
        return door_max_shift;
    }

    void TrainDoor::set_door_open_method(const int p_open_method) {
        door_open_method = p_open_method;
    }

    int TrainDoor::get_door_open_method() const {
        return door_open_method;
    }

    void TrainDoor::set_door_voltage(const double p_voltage) {
        door_voltage = p_voltage;
    }

    double TrainDoor::get_door_voltage() const {
        return door_voltage;
    }

    void TrainDoor::set_door_closure_warning(const bool p_closure_warning) {
        door_closure_warning = p_closure_warning;
    }

    bool TrainDoor::get_door_closure_warning() const {
        return door_closure_warning;
    }

    void TrainDoor::set_auto_door_closure_warning(const bool p_auto_closure_warning) {
        auto_door_closure_warning = p_auto_closure_warning;
    }

    bool TrainDoor::get_auto_door_closure_warning() const {
        return auto_door_closure_warning;
    }

    void TrainDoor::set_door_open_delay(const double p_open_delay) {
        door_open_delay = p_open_delay;
    }

    double TrainDoor::get_door_open_delay() const {
        return door_open_delay;
    }

    void TrainDoor::set_door_close_delay(const double p_close_delay) {
        door_close_delay = p_close_delay;
    }

    double TrainDoor::get_door_close_delay() const {
        return door_close_delay;
    }

    void TrainDoor::set_door_open_with_permit(const double p_holding_time) {
        door_open_with_permit = p_holding_time;
    }

    double TrainDoor::get_door_open_with_permit() const {
        return door_open_with_permit;
    }

    void TrainDoor::set_door_blocked(const bool p_blocked) {
        door_blocked = p_blocked;
    }

    bool TrainDoor::get_door_blocked() const {
        return door_blocked;
    }

    void TrainDoor::set_door_max_shift_plug(const double p_max_shift_plug) {
        door_max_shift = p_max_shift_plug;
    }

    double TrainDoor::get_door_max_shift_plug() const {
        return door_max_shift;
    }

    void TrainDoor::set_door_permit_list(const int p_permit_list) {
        door_permit_list = p_permit_list;
    }

    int TrainDoor::get_door_permit_list() const {
        return door_permit_list;
    }

    void TrainDoor::set_door_permit_list_default(const int p_permit_list_default) {
        door_permit_list_default = p_permit_list_default;
    }

    int TrainDoor::get_door_permit_list_default() const {
        return door_permit_list_default;
    }

    void TrainDoor::set_door_auto_close_remote(const bool p_auto_close) {
        door_auto_close_remote = p_auto_close;
    }

    bool TrainDoor::get_door_auto_close_remote() const {
        return door_auto_close_remote;
    }

    void TrainDoor::set_door_auto_close_vel(const double p_vel) {
        door_auto_close_vel = p_vel;
    }

    double TrainDoor::get_door_auto_close_vel() const {
        return door_auto_close_vel;
    }

    void TrainDoor::set_door_platform_max_speed(const double p_max_speed) {
        platform_max_speed = p_max_speed;
    }

    double TrainDoor::get_door_platform_max_speed() const {
        return platform_max_speed;
    }

    void TrainDoor::set_door_platform_max_shift(const double p_max_shift) {
        platform_max_shift = p_max_shift;
    }

    double TrainDoor::get_door_platform_max_shift() const {
        return platform_max_shift;
    }

    void TrainDoor::set_door_platform_speed(const double p_speed) {
        platform_speed = p_speed;
    }

    double TrainDoor::get_door_platform_speed() const {
        return platform_speed;
    }

    void TrainDoor::set_platform_shift_method(const int p_shift_method) {
        platform_shift_method = p_shift_method;
    }

    int TrainDoor::get_platform_shift_method() const {
        return platform_shift_method;
    }

    void TrainDoor::set_mirror_max_shift(const double p_max_shift) {
        mirror_max_shift = p_max_shift;
    }

    double TrainDoor::get_mirror_max_shift() const {
        return mirror_max_shift;
    }

    void TrainDoor::set_mirror_vel_close(const double p_vel_close) {
        mirror_vel_close = p_vel_close;
    }

    double TrainDoor::get_mirror_vel_close() const {
        return mirror_vel_close;
    }

    void TrainDoor::set_door_needs_permit(const bool p_needs_permit) {
        door_needs_permit = p_needs_permit;
    }

    bool TrainDoor::get_door_needs_permit() const {
        return door_needs_permit;
    }

    void TrainDoor::set_door_permit_light_blinking(const int p_blinking_mode) {
        door_permit_light_blinking = p_blinking_mode;
    }

    int TrainDoor::get_door_permit_light_blinking() const {
        return door_permit_light_blinking;
    }
} // namespace godot
