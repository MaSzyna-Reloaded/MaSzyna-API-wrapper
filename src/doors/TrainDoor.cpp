#include "TrainDoor.hpp"

namespace godot {
    void TrainDoor::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_close_control", "close_control"), &TrainDoor::set_close_control);
        ClassDB::bind_method(D_METHOD("get_close_control"), &TrainDoor::get_close_control);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "close_control", PROPERTY_HINT_ENUM, "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_close_control", "get_close_control");
        ClassDB::bind_method(D_METHOD("set_open_control", "open_control"), &TrainDoor::set_open_control);
        ClassDB::bind_method(D_METHOD("get_open_control"), &TrainDoor::get_open_control);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "open_control", PROPERTY_HINT_ENUM, "Passenger,AutomaticCtrl,DriverCtrl,Conductor,Mixed"), "set_open_control", "get_open_control");
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
}