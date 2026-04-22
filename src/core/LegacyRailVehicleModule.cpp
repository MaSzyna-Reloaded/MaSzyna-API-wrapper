#include "GameLog.hpp"
#include "LegacyRailVehicle.hpp"
#include "LegacyRailVehicleModule.hpp"
#include "RailVehicle.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void LegacyRailVehicleModule::_bind_methods() {
        ClassDB::bind_method(D_METHOD("update_mover"), &LegacyRailVehicleModule::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &LegacyRailVehicleModule::get_mover_state);
        ClassDB::bind_method(
                D_METHOD("get_legacy_rail_vehicle"), &LegacyRailVehicleModule::get_legacy_rail_vehicle);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &LegacyRailVehicleModule::get_supported_commands);
    }

    void LegacyRailVehicleModule::_initialize() {
        RailVehicleModule::_initialize();

        if (legacy_rail_vehicle != nullptr) {
            const Error config_con = legacy_rail_vehicle->connect(
                    LegacyRailVehicle::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
            if (config_con != OK) {
                UtilityFunctions::push_warning(
                        "LegacyRailVehicleModule::initialize() failed with error code " + String::num(config_con));
            }

            const Error init_con = legacy_rail_vehicle->connect(
                    LegacyRailVehicle::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
            if (init_con != OK) {
                UtilityFunctions::push_warning(
                        "LegacyRailVehicleModule::initialize() mover init connect failed with error code " +
                        String::num(init_con));
            }

            if (legacy_rail_vehicle->get_mover() != nullptr) {
                update_mover();
            }
        }
    }

    void LegacyRailVehicleModule::_deinitialize() {
        if (legacy_rail_vehicle != nullptr) {
            legacy_rail_vehicle->disconnect(
                    LegacyRailVehicle::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
            legacy_rail_vehicle->disconnect(
                    LegacyRailVehicle::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
        }

        RailVehicleModule::_deinitialize();
    }

    void LegacyRailVehicleModule::_update(const double delta) {
        RailVehicleModule::_update(delta);

        if (_dirty) {
            update_mover();
            _dirty = false;
        }

        _process_mover(delta);
    }

    void LegacyRailVehicleModule::set_rail_vehicle(RailVehicle *p_rail_vehicle) {
        RailVehicleModule::set_rail_vehicle(p_rail_vehicle);

        legacy_rail_vehicle = Object::cast_to<LegacyRailVehicle>(p_rail_vehicle);
    }

    void LegacyRailVehicleModule::_do_update_internal_mover(TMoverParameters *mover) {}

    void LegacyRailVehicleModule::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}

    void LegacyRailVehicleModule::_do_process_mover(TMoverParameters *mover, double delta) {}

    TMoverParameters *LegacyRailVehicleModule::get_mover() {
        if (legacy_rail_vehicle != nullptr) {
            return legacy_rail_vehicle->get_mover();
        }

        return nullptr;
    }

    void LegacyRailVehicleModule::_process_mover(const double delta) {
        if (legacy_rail_vehicle != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                legacy_rail_vehicle->_get_state_internal().merge(get_mover_state(), true);
            }
        }
    }

    void LegacyRailVehicleModule::update_mover() {
        if (legacy_rail_vehicle != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle->get_mover();
            if (mover != nullptr) {
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
        if (legacy_rail_vehicle != nullptr) {
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

    LegacyRailVehicle *LegacyRailVehicleModule::get_legacy_rail_vehicle() const {
        return legacy_rail_vehicle;
    }

    Dictionary LegacyRailVehicleModule::get_supported_commands() {
        return RailVehicleModule::get_supported_commands();
    }
} // namespace godot
