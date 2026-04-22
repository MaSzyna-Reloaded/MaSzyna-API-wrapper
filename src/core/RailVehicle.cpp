#include "LegacyRailVehicleModule.hpp"
#include "RailVehicle.hpp"
#include "TrainSet.hpp"
#include <cstdlib>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void RailVehicle::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple", "other_vehicle", "self_side", "other_side"), &RailVehicle::couple);
        ClassDB::bind_method(D_METHOD("couple_front", "other_vehicle", "other_side"), &RailVehicle::couple_front);
        ClassDB::bind_method(D_METHOD("couple_back", "other_vehicle", "other_side"), &RailVehicle::couple_back);
        ClassDB::bind_method(D_METHOD("get_front_vehicle"), &RailVehicle::get_front_vehicle);
        ClassDB::bind_method(D_METHOD("get_back_vehicle"), &RailVehicle::get_back_vehicle);
        ClassDB::bind_method(D_METHOD("get_rail_vehicle_modules"), &RailVehicle::get_rail_vehicle_modules);
        ClassDB::bind_method(D_METHOD("set_modules", "modules"), &RailVehicle::set_modules);
        ClassDB::bind_method(D_METHOD("get_modules"), &RailVehicle::get_modules);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &RailVehicle::get_supported_commands);
        ClassDB::bind_method(D_METHOD("initialize"), &RailVehicle::initialize);
        ClassDB::bind_method(D_METHOD("finalize"), &RailVehicle::finalize);
        ClassDB::bind_method(D_METHOD("update", "delta"), &RailVehicle::update);
        ClassDB::bind_method(D_METHOD("decouple", "relative_index"), &RailVehicle::decouple);
        ClassDB::bind_method(D_METHOD("uncouple_front"), &RailVehicle::uncouple_front);
        ClassDB::bind_method(D_METHOD("uncouple_back"), &RailVehicle::uncouple_back);
        ClassDB::bind_method(D_METHOD("get_trainset"), &RailVehicle::get_trainset);
        ClassDB::bind_method(D_METHOD("_to_string"), &RailVehicle::_to_string);
        ClassDB::bind_method(D_METHOD("set_mass", "value"), &RailVehicle::set_mass);
        ClassDB::bind_method(D_METHOD("get_mass"), &RailVehicle::get_mass);
        ClassDB::bind_method(D_METHOD("set_max_velocity", "value"), &RailVehicle::set_max_velocity);
        ClassDB::bind_method(D_METHOD("get_max_velocity"), &RailVehicle::get_max_velocity);
        ClassDB::bind_method(D_METHOD("set_length", "value"), &RailVehicle::set_length);
        ClassDB::bind_method(D_METHOD("get_length"), &RailVehicle::get_length);
        ClassDB::bind_method(D_METHOD("set_width", "value"), &RailVehicle::set_width);
        ClassDB::bind_method(D_METHOD("get_width"), &RailVehicle::get_width);
        ClassDB::bind_method(D_METHOD("set_height", "value"), &RailVehicle::set_height);
        ClassDB::bind_method(D_METHOD("get_height"), &RailVehicle::get_height);

        /*
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "trainset", PROPERTY_HINT_RESOURCE_TYPE, "TrainSet",
                        PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE | PROPERTY_USAGE_NO_INSTANCE_STATE),
                "", "get_trainset");
        */

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_velocity"), "set_max_velocity", "get_max_velocity");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length"), "set_length", "get_length");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height"), "set_height", "get_height");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::ARRAY, "modules", PROPERTY_HINT_ARRAY_TYPE,
                        String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) +
                                ":LegacyRailVehicleModule",
                        PROPERTY_USAGE_DEFAULT, "Array[LegacyRailVehicleModule]"),
                "set_modules", "get_modules");

        BIND_ENUM_CONSTANT(FRONT);
        BIND_ENUM_CONSTANT(BACK);
    }

    RailVehicle::RailVehicle() {
        trainset.instantiate();
        trainset->_init(this);
    }

    RailVehicle::~RailVehicle() = default;

    void RailVehicle::_on_coupled(RailVehicle *other_vehicle, Side self_side, Side other_side) {}

    void RailVehicle::_on_uncoupled(RailVehicle *other_vehicle, Side self_side, Side other_side) {}

    String RailVehicle::_to_string() const {
        return String("<RailVehicle({0})>").format(Array::make(get_name()));
    }

    void RailVehicle::_initialize() {}

    void RailVehicle::_finalize() {}

    void RailVehicle::_update(const double delta) {}

    void RailVehicle::couple(RailVehicle *other_vehicle, Side self_side, Side other_side) {
        if (self_side == Side::FRONT && other_side == Side::BACK) {
            if (front != nullptr || other_vehicle->back != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            front = other_vehicle;
            other_vehicle->back = this;
            _on_coupled(other_vehicle, self_side, other_side);
            other_vehicle->_on_coupled(this, other_side, self_side);
        } else if (self_side == Side::BACK && other_side == Side::FRONT) {
            if (back != nullptr || other_vehicle->front != nullptr) {
                UtilityFunctions::push_error("One of the cars is already coupled.");
                return;
            }
            back = other_vehicle;
            other_vehicle->front = this;
            _on_coupled(other_vehicle, self_side, other_side);
            other_vehicle->_on_coupled(this, other_side, self_side);
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

    RailVehicle *RailVehicle::get_front_vehicle() const {
        return front;
    }

    RailVehicle *RailVehicle::get_back_vehicle() const {
        return back;
    }

    Array RailVehicle::get_rail_vehicle_modules() const {
        return modules;
    }

    void RailVehicle::set_modules(const Array &p_modules) {
        _clear_module_vehicle_references();
        modules = p_modules;
        _assign_modules_to_vehicle();
        _dirty = true;
    }

    Array RailVehicle::get_modules() const {
        return modules;
    }

    Dictionary RailVehicle::get_supported_commands() {
        return {};
    }

    void RailVehicle::_assign_modules_to_vehicle() {
        Ref<RailVehicle> vehicle_ref(this);
        for (int index = 0; index < modules.size(); ++index) {
            if (auto *module = Object::cast_to<LegacyRailVehicleModule>(modules[index]); module != nullptr) {
                module->set_rail_vehicle(vehicle_ref);
            }
        }
    }

    void RailVehicle::_clear_module_vehicle_references() {
        for (int index = 0; index < modules.size(); ++index) {
            if (auto *module = Object::cast_to<LegacyRailVehicleModule>(modules[index]); module != nullptr) {
                module->set_rail_vehicle(Ref<RailVehicle>());
            }
        }
    }

    void RailVehicle::initialize() {
        if (runtime_initialized) {
            return;
        }

        _assign_modules_to_vehicle();
        _initialize();

        for (int index = 0; index < modules.size(); ++index) {
            if (auto *module = Object::cast_to<LegacyRailVehicleModule>(modules[index]); module != nullptr) {
                module->initialize();
            }
        }
        runtime_initialized = true;
    }

    void RailVehicle::finalize() {
        if (!runtime_initialized) {
            _clear_module_vehicle_references();
            return;
        }

        for (int index = 0; index < modules.size(); ++index) {
            if (auto *module = Object::cast_to<LegacyRailVehicleModule>(modules[index]); module != nullptr) {
                module->finalize();
            }
        }

        _finalize();
        runtime_initialized = false;
        _clear_module_vehicle_references();
    }

    void RailVehicle::update(const double delta) {
        if (!runtime_initialized) {
            return;
        }

        _update(delta);

        for (int index = 0; index < modules.size(); ++index) {
            if (auto *module = Object::cast_to<LegacyRailVehicleModule>(modules[index]); module != nullptr) {
                module->update(delta);
            }
        }
    }

    RailVehicle *RailVehicle::decouple(const int relative_index) {
        if (relative_index == 0) {
            UtilityFunctions::push_error("RailVehicle::decouple() requires a non-zero relative index.");
            return nullptr;
        }

        RailVehicle *target = this;
        const bool decouple_front_side = relative_index > 0;
        const int steps = std::abs(relative_index) - 1;

        for (int index = 0; index < steps; ++index) {
            target = decouple_front_side ? target->front : target->back;
            if (target == nullptr) {
                UtilityFunctions::push_error("RailVehicle::decouple() target vehicle is out of consist bounds.");
                return nullptr;
            }
        }

        return decouple_front_side ? target->uncouple_front() : target->uncouple_back();
    }

    RailVehicle *RailVehicle::uncouple_front() {
        if (front == nullptr) {
            UtilityFunctions::push_error("No car coupled at the front.");
            return nullptr;
        }
        RailVehicle *uncoupled = front;
        front->back = nullptr;
        front = nullptr;
        _on_uncoupled(uncoupled, Side::FRONT, Side::BACK);
        uncoupled->_on_uncoupled(this, Side::BACK, Side::FRONT);
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
        _on_uncoupled(uncoupled, Side::BACK, Side::FRONT);
        uncoupled->_on_uncoupled(this, Side::FRONT, Side::BACK);
        return uncoupled;
    }
} // namespace godot
