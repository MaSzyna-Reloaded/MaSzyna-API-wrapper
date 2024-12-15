#include <godot_cpp/variant/utility_functions.hpp>
#include "RailVehicle.hpp"
#include "TrainSet.hpp"

namespace godot {

    void RailVehicle::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple", "other_vehicle", "self_side", "other_side"), &RailVehicle::couple);
        ClassDB::bind_method(D_METHOD("couple_front", "other_vehicle", "other_side"), &RailVehicle::couple_front);
        ClassDB::bind_method(D_METHOD("couple_back", "other_vehicle", "other_side"), &RailVehicle::couple_back);
        ClassDB::bind_method(D_METHOD("uncouple_front"), &RailVehicle::uncouple_front);
        ClassDB::bind_method(D_METHOD("uncouple_back"), &RailVehicle::uncouple_back);
        ClassDB::bind_method(D_METHOD("get_trainset"), &RailVehicle::get_trainset);
        ClassDB::bind_method(D_METHOD("_to_string"), &RailVehicle::_to_string);

        /*
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "trainset", PROPERTY_HINT_RESOURCE_TYPE, "TrainSet",
                        PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_NO_INSTANCE_STATE),
                "", "get_trainset");
        */

        BIND_ENUM_CONSTANT(FRONT);
        BIND_ENUM_CONSTANT(BACK);
    }

    RailVehicle::RailVehicle() {
        trainset.instantiate();
        trainset->_init(this);
    }

    String RailVehicle::_to_string() const {
        return String("<RailVehicle({0})>").format(Array::make(get_name()));
    }

    void RailVehicle::couple(RailVehicle *other_vehicle, Side self_side, Side other_side) {
        if (self_side == Side::FRONT && other_side == Side::BACK) {
            if (front != nullptr || other_vehicle->back != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            front = other_vehicle;
            other_vehicle->back = this;
        } else if (self_side == Side::BACK && other_side == Side::FRONT) {
            if (back != nullptr || other_vehicle->front != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            back = other_vehicle;
            other_vehicle->front = this;
        } else {
            UtilityFunctions::push_error("Invalid coupling sides specified.");
        }
    }

    void RailVehicle::couple_front(RailVehicle *other_vehicle, Side other_side) {
        couple(other_vehicle, Side::FRONT, other_side);
    }

    void RailVehicle::couple_back(RailVehicle *other_vehicle, Side other_side) {
        couple(other_vehicle, Side::BACK, other_side);
    }

    RailVehicle *RailVehicle::uncouple_front() {
        if (front == nullptr) {
            UtilityFunctions::push_error("No car coupled at the front.");
            return nullptr;
        }
        RailVehicle *uncoupled = front;
        front->back = nullptr;
        front = nullptr;
        return uncoupled;
    }

    Ref<TrainSet> RailVehicle::get_trainset() const {
        return trainset;
    }

    RailVehicle *RailVehicle::uncouple_back() {
        if (back == nullptr) {
            UtilityFunctions::push_error("No car coupled at the back.");
            return nullptr;
        }
        RailVehicle *uncoupled = back;
        back->front = nullptr;
        back = nullptr;
        return uncoupled;
    }
} // namespace godot
