#include "RailVehicleModule.hpp"
#include "RailVehicle.hpp"

namespace godot {
    void RailVehicleModule::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_rail_vehicle", "rail_vehicle"), &RailVehicleModule::set_rail_vehicle);
        ClassDB::bind_method(D_METHOD("get_rail_vehicle"), &RailVehicleModule::get_rail_vehicle);
        ClassDB::bind_method(D_METHOD("is_initialized"), &RailVehicleModule::is_initialized);
        ClassDB::bind_method(D_METHOD("initialize"), &RailVehicleModule::initialize);
        ClassDB::bind_method(D_METHOD("deinitialize"), &RailVehicleModule::deinitialize);
        ClassDB::bind_method(D_METHOD("update", "delta"), &RailVehicleModule::update);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &RailVehicleModule::get_supported_commands);
    }

    void RailVehicleModule::_initialize() {}

    void RailVehicleModule::_deinitialize() {}

    void RailVehicleModule::_update(const double delta) {}

    void RailVehicleModule::set_rail_vehicle(RailVehicle *p_rail_vehicle) {
        if (rail_vehicle == p_rail_vehicle) {
            return;
        }

        if (initialized) {
            deinitialize();
        }

        rail_vehicle = p_rail_vehicle;
    }

    RailVehicle *RailVehicleModule::get_rail_vehicle() const {
        return rail_vehicle;
    }

    bool RailVehicleModule::is_initialized() const {
        return initialized;
    }

    void RailVehicleModule::initialize() {
        if (initialized) {
            return;
        }

        initialized = true;
        _initialize();
    }

    void RailVehicleModule::deinitialize() {
        if (!initialized) {
            return;
        }

        _deinitialize();
        initialized = false;
    }

    void RailVehicleModule::update(const double delta) {
        _update(delta);
    }

    Dictionary RailVehicleModule::get_supported_commands() {
        return {};
    }
} // namespace godot
