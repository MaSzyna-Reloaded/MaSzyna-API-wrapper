#include "LegacyRailVehicleModule.hpp"
#include "LegacyRailVehicle.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void LegacyRailVehicleModule::_bind_methods() {
        ClassDB::bind_method(D_METHOD("update_mover"), &LegacyRailVehicleModule::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &LegacyRailVehicleModule::get_mover_state);
        ClassDB::bind_method(
                D_METHOD("get_legacy_rail_vehicle_node"), &LegacyRailVehicleModule::get_legacy_rail_vehicle_node);
    }

    void LegacyRailVehicleModule::_notification(int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                set_process(true);
                Node *parent = get_parent();
                while (parent != nullptr) {
                    legacy_rail_vehicle_node = Object::cast_to<LegacyRailVehicle>(parent);
                    if (legacy_rail_vehicle_node != nullptr) {
                        break;
                    }
                    parent = parent->get_parent();
                }

                if (legacy_rail_vehicle_node != nullptr) {
                    const Error config_con = legacy_rail_vehicle_node->connect(
                            LegacyRailVehicle::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                    if (config_con != OK) {
                        UtilityFunctions::push_warning(
                                "LegacyRailVehicleModule::notification(NOTIFICATION_ENTER_TREE) failed with error code " +
                                String::num(config_con));
                    }

                    const Error init_con = legacy_rail_vehicle_node->connect(
                            LegacyRailVehicle::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
                    if (init_con != OK) {
                        UtilityFunctions::push_warning(
                                "LegacyRailVehicleModule::notification(NOTIFICATION_ENTER_TREE) mover init connect failed with error code " +
                                String::num(init_con));
                    }
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                set_process(false);
                if (legacy_rail_vehicle_node != nullptr) {
                    legacy_rail_vehicle_node->disconnect(
                            LegacyRailVehicle::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                    legacy_rail_vehicle_node->disconnect(
                            LegacyRailVehicle::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
                }
                legacy_rail_vehicle_node = nullptr;
            } break;
            default:
                break;
        }
    }

    void LegacyRailVehicleModule::_process(const double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (_dirty) {
            update_mover();
            _dirty = false;
        }

        _process_mover(delta);
    }

    void LegacyRailVehicleModule::_do_update_internal_mover(TMoverParameters *mover) {}

    void LegacyRailVehicleModule::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}

    void LegacyRailVehicleModule::_do_process_mover(TMoverParameters *mover, double delta) {}

    TMoverParameters *LegacyRailVehicleModule::get_mover() {
        if (legacy_rail_vehicle_node != nullptr) {
            return legacy_rail_vehicle_node->get_mover();
        }

        return nullptr;
    }

    void LegacyRailVehicleModule::_process_mover(const double delta) {
        if (legacy_rail_vehicle_node != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                legacy_rail_vehicle_node->_get_state_internal().merge(get_mover_state(), true);
            }
        }
    }

    void LegacyRailVehicleModule::update_mover() {
        if (legacy_rail_vehicle_node != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
                Dictionary new_config;
                _do_fetch_config_from_mover(mover, new_config);
                legacy_rail_vehicle_node->update_config(new_config);
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
        if (legacy_rail_vehicle_node != nullptr) {
            TMoverParameters *mover = legacy_rail_vehicle_node->get_mover();
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

    LegacyRailVehicle *LegacyRailVehicleModule::get_legacy_rail_vehicle_node() const {
        return legacy_rail_vehicle_node;
    }
} // namespace godot
