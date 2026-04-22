#include "GameLog.hpp"
#include "LegacyRailVehicle.hpp"
#include "LegacyRailVehicleModule.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void LegacyRailVehicleModule::_bind_methods() {
        ClassDB::bind_method(D_METHOD("update_mover"), &LegacyRailVehicleModule::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &LegacyRailVehicleModule::get_mover_state);
        ClassDB::bind_method(D_METHOD("set_rail_vehicle", "rail_vehicle"), &LegacyRailVehicleModule::set_rail_vehicle);
        ClassDB::bind_method(D_METHOD("get_rail_vehicle"), &LegacyRailVehicleModule::get_rail_vehicle);
        ClassDB::bind_method(D_METHOD("initialize"), &LegacyRailVehicleModule::initialize);
        ClassDB::bind_method(D_METHOD("finalize"), &LegacyRailVehicleModule::finalize);
        ClassDB::bind_method(D_METHOD("update", "delta"), &LegacyRailVehicleModule::update);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &LegacyRailVehicleModule::get_supported_commands);
    }

    void LegacyRailVehicleModule::_initialize() {}

    void LegacyRailVehicleModule::_finalize() {}

    void LegacyRailVehicleModule::_update(const double delta) {}

    void LegacyRailVehicleModule::set_rail_vehicle(const Ref<RailVehicle> &p_rail_vehicle) {
        rail_vehicle = p_rail_vehicle;
    }

    Ref<RailVehicle> LegacyRailVehicleModule::get_rail_vehicle() const {
        return rail_vehicle;
    }

    void LegacyRailVehicleModule::initialize() {
        _initialize();
    }

    void LegacyRailVehicleModule::finalize() {
        _finalize();
        rail_vehicle.unref();
    }

    void LegacyRailVehicleModule::update(const double delta) {
        _update(delta);
        _process_mover(delta);
    }

    void LegacyRailVehicleModule::_do_update_internal_mover(TMoverParameters *mover) {}

    void LegacyRailVehicleModule::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}

    void LegacyRailVehicleModule::_do_process_mover(TMoverParameters *mover, double delta) {}

    LegacyRailVehicle *LegacyRailVehicleModule::get_legacy_rail_vehicle() const {
        return Object::cast_to<LegacyRailVehicle>(rail_vehicle.ptr());
    }

    TMoverParameters *LegacyRailVehicleModule::get_mover() {
        if (LegacyRailVehicle *legacy_rail_vehicle = get_legacy_rail_vehicle(); legacy_rail_vehicle != nullptr) {
            return legacy_rail_vehicle->get_mover();
        }

        return nullptr;
    }

    void LegacyRailVehicleModule::_process_mover(const double delta) {
        if (LegacyRailVehicle *legacy_rail_vehicle = get_legacy_rail_vehicle(); legacy_rail_vehicle != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                legacy_rail_vehicle->_get_state_internal().merge(get_mover_state(), true);
            }
        }
    }

    void LegacyRailVehicleModule::update_mover() {
        if (LegacyRailVehicle *legacy_rail_vehicle = get_legacy_rail_vehicle(); legacy_rail_vehicle != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle->get_mover();
            if (mover != nullptr) {
                DEBUG("LegacyRailVehicleModule::update_mover vehicle=%s module_class=%s mover=%s",
                      legacy_rail_vehicle->get_name(), get_class(), static_cast<const void *>(mover));
                _do_update_internal_mover(mover);
                Dictionary new_config;
                _do_fetch_config_from_mover(mover, new_config);
                legacy_rail_vehicle->update_config(new_config);
            } else {
                UtilityFunctions::push_warning(
                        "LegacyRailVehicleModule::update_mover() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning(
                    "LegacyRailVehicleModule::update_mover() failed: missing legacy rail vehicle node");
        }
    }

    Dictionary LegacyRailVehicleModule::get_mover_state() {
        if (LegacyRailVehicle *legacy_rail_vehicle = get_legacy_rail_vehicle(); legacy_rail_vehicle != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle->get_mover();
            if (mover != nullptr) {
                _do_fetch_state_from_mover(mover, state);
            } else {
                UtilityFunctions::push_warning(
                        "LegacyRailVehicleModule::get_mover_state() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning(
                    "LegacyRailVehicleModule::get_mover_state() failed: missing legacy rail vehicle node");
        }

        return state;
    }

    Dictionary LegacyRailVehicleModule::get_supported_commands() {
        return {};
    }
} // namespace godot
