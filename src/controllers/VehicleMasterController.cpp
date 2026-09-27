#include "VehicleMasterController.hpp"

namespace godot {
    void VehicleMasterController::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_main_position_count", "value"), &VehicleMasterController::set_main_position_count);
        ClassDB::bind_method(D_METHOD("get_main_position_count"), &VehicleMasterController::get_main_position_count);
        ClassDB::bind_method(
                D_METHOD("set_second_position_count", "value"), &VehicleMasterController::set_second_position_count);
        ClassDB::bind_method(
                D_METHOD("get_second_position_count"), &VehicleMasterController::get_second_position_count);
        ClassDB::bind_method(
                D_METHOD("set_direction_change_max_position", "value"),
                &VehicleMasterController::set_direction_change_max_position);
        ClassDB::bind_method(
                D_METHOD("get_direction_change_max_position"),
                &VehicleMasterController::get_direction_change_max_position);
        ClassDB::bind_method(
                D_METHOD("set_coupled_controllers", "value"), &VehicleMasterController::set_coupled_controllers);
        ClassDB::bind_method(D_METHOD("get_coupled_controllers"), &VehicleMasterController::get_coupled_controllers);
        ClassDB::bind_method(D_METHOD("set_initial_delay", "value"), &VehicleMasterController::set_initial_delay);
        ClassDB::bind_method(D_METHOD("get_initial_delay"), &VehicleMasterController::get_initial_delay);
        ClassDB::bind_method(D_METHOD("set_step_delay", "value"), &VehicleMasterController::set_step_delay);
        ClassDB::bind_method(D_METHOD("get_step_delay"), &VehicleMasterController::get_step_delay);
        ClassDB::bind_method(D_METHOD("set_step_down_delay", "value"), &VehicleMasterController::set_step_down_delay);
        ClassDB::bind_method(D_METHOD("get_step_down_delay"), &VehicleMasterController::get_step_down_delay);

        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "main_position_count"), "set_main_position_count",
                "get_main_position_count");
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "second_position_count"), "set_second_position_count",
                "get_second_position_count");
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "direction_change_max_position"), "set_direction_change_max_position",
                "get_direction_change_max_position");
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "coupled_controllers"), "set_coupled_controllers",
                "get_coupled_controllers");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "initial_delay"), "set_initial_delay", "get_initial_delay");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step_delay"), "set_step_delay", "get_step_delay");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step_down_delay"), "set_step_down_delay", "get_step_down_delay");
    }

    void VehicleMasterController::set_main_position_count(const int p_value) {
        main_position_count = p_value;
    }

    int VehicleMasterController::get_main_position_count() const {
        return main_position_count;
    }

    void VehicleMasterController::set_second_position_count(const int p_value) {
        second_position_count = p_value;
    }

    int VehicleMasterController::get_second_position_count() const {
        return second_position_count;
    }

    void VehicleMasterController::set_direction_change_max_position(const int p_value) {
        direction_change_max_position = p_value;
    }

    int VehicleMasterController::get_direction_change_max_position() const {
        return direction_change_max_position;
    }

    void VehicleMasterController::set_coupled_controllers(const bool p_value) {
        coupled_controllers = p_value;
    }

    bool VehicleMasterController::get_coupled_controllers() const {
        return coupled_controllers;
    }

    void VehicleMasterController::set_initial_delay(const double p_value) {
        initial_delay = p_value;
    }

    double VehicleMasterController::get_initial_delay() const {
        return initial_delay;
    }

    void VehicleMasterController::set_step_delay(const double p_value) {
        step_delay = p_value;
    }

    double VehicleMasterController::get_step_delay() const {
        return step_delay;
    }

    void VehicleMasterController::set_step_down_delay(const double p_value) {
        step_down_delay = p_value;
    }

    double VehicleMasterController::get_step_down_delay() const {
        return step_down_delay;
    }
} // namespace godot
