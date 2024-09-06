#include "TrainDoor.hpp"

namespace godot {
    void TrainDoor::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_close_control", "close_control"), &TrainDoor::set_close_control);
        ClassDB::bind_method(D_METHOD("get_close_control"), &TrainDoor::get_close_control);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "close_control", PROPERTY_HINT_ENUM,
                        "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"),
                "set_close_control", "get_close_control");
        ClassDB::bind_method(D_METHOD("set_open_control", "open_control"), &TrainDoor::set_open_control);
        ClassDB::bind_method(D_METHOD("get_open_control"), &TrainDoor::get_open_control);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "open_control", PROPERTY_HINT_ENUM,
                        "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"),
                "set_open_control", "get_open_control");
        ClassDB::bind_method(D_METHOD("set_door_stay_open", "open_control"), &TrainDoor::set_door_stay_open);
        ClassDB::bind_method(D_METHOD("get_door_stay_open"), &TrainDoor::get_door_stay_open);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "door_stay_open"), "set_door_stay_open", "get_door_stay_open");
        ClassDB::bind_method(D_METHOD("set_open_speed", "open_speed"), &TrainDoor::set_open_speed);
        ClassDB::bind_method(D_METHOD("get_open_speed"), &TrainDoor::get_open_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "open_speed"), "set_open_speed", "get_open_speed");
        ClassDB::bind_method(D_METHOD("set_close_speed", "close_speed"), &TrainDoor::set_close_speed);
        ClassDB::bind_method(D_METHOD("get_close_speed"), &TrainDoor::get_close_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "close_speed"), "set_close_speed", "get_close_speed");
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

    void TrainDoor::set_door_auto_closure_warning(const bool p_auto_closure_warning) {
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

    void TrainDoor::set_door_max_shift_plug(const double p_max_shift) {
        door_max_shift = p_max_shift;
    }

    double TrainDoor::get_door_max_shift_plug() const {
        return door_max_shift;
    }
} // namespace godot
