#include "VehicleDieselElectricEngine.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    double VehicleDieselElectricEngine::get_motor_current() const {
        return traction.get_motor_current(get_mover());
    }

    double VehicleDieselElectricEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(get_mover());
    }

    bool VehicleDieselElectricEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(get_mover());
    }

    bool VehicleDieselElectricEngine::get_fuse_active() const {
        return traction.get_fuse_active(get_mover());
    }

    bool VehicleDieselElectricEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(get_mover());
    }

    void VehicleDieselElectricEngine::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleDieselEngine::_fill_state_dictionary(p_state);
        p_state["Im"] = get_motor_current();
        p_state["circuit_imax"] = get_circuit_imax();
        p_state["dynamic_brake_active"] = get_dynamic_brake_active();
        p_state["fuse_active"] = get_fuse_active();
        p_state["motor_connectors_open"] = get_motor_connectors_open();
    }

    void VehicleDieselElectricEngine::fuse_reset() {
        traction.reset_fuse(get_mover());
    }

    void VehicleDieselElectricEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(get_mover(), p_open);
    }

    void VehicleDieselElectricEngine::_register_commands() {
        VehicleDieselEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void VehicleDieselElectricEngine::_unregister_commands() {
        VehicleDieselEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void VehicleDieselElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselElectricEngine, Variant::ARRAY, wwlist, PROPERTY_HINT_TYPE_STRING, "WWListItem");
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::BOOL, generator_voltage_flat);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::FLOAT, hyperbolic_speed);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::FLOAT, additional_speed);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::FLOAT, rpm_change_rate);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::FLOAT, power_correction_ratio);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::INT, shunt_relay_type);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::BOOL, shunt_mode_allowed);
        BIND_PROPERTY(VehicleDieselElectricEngine, Variant::FLOAT, heating_rpm);

        ClassDB::bind_method(D_METHOD("get_motor_current"), &VehicleDieselElectricEngine::get_motor_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "motor_current", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_current");
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &VehicleDieselElectricEngine::get_circuit_imax);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "circuit_imax", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_imax");
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &VehicleDieselElectricEngine::get_dynamic_brake_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "dynamic_brake_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_dynamic_brake_active");
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &VehicleDieselElectricEngine::get_fuse_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "fuse_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_fuse_active");
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &VehicleDieselElectricEngine::get_motor_connectors_open);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "motor_connectors_open", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_motor_connectors_open");

        ClassDB::bind_method(D_METHOD("fuse_reset"), &VehicleDieselElectricEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("set_motor_connectors_open", "open"), &VehicleDieselElectricEngine::set_motor_connectors_open);
    }

    VehicleEngine::EngineType VehicleDieselElectricEngine::get_engine_type() const {
        return VehicleEngine::EngineType::DIESEL_ELECTRIC;
    }

    void VehicleDieselElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleDieselEngine::_do_update_internal_mover(p_mover);

        p_mover->Flat = generator_voltage_flat;
        p_mover->Vhyp = hyperbolic_speed;
        p_mover->Vadd = additional_speed;
        p_mover->dizel_RevolutionsDecreaseRate = rpm_change_rate;
        p_mover->PowerCorRatio = power_correction_ratio;
        p_mover->RelayType = shunt_relay_type;
        p_mover->ShuntModeAllow = shunt_mode_allowed;
        p_mover->EngineHeatingRPM = heating_rpm;

        /* WWList: tablica rezystorow rozr. (eng. Starting resistor array) aka DEList aka TDESchemeTable */
        constexpr int MAX = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(wwlist.size());
        p_mover->MainCtrlPosNo = wwlist_size - 1;
        for (int i = 0; i < std::min(MAX, wwlist_size); i++) {
            const Ref<WWListItem> &row = wwlist[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselElectricEngine]: wwlist property is null at index " + String::num(i));
                continue;
            }

            p_mover->DElist[i].RPM = row->get_rpm();
            p_mover->DElist[i].GenPower = row->get_max_power();
            p_mover->DElist[i].Umax = row->get_max_voltage();
            p_mover->DElist[i].Imax = row->get_max_current();
            if (row->get_has_shunting()) {
                p_mover->SST[i].Umin = row->get_min_wakeup_voltage();
                p_mover->SST[i].Umax = row->get_max_wakeup_voltage();
                p_mover->SST[i].Pmax = row->get_max_wakeup_power();
                p_mover->SST[i].Pmin = std::sqrt(std::pow(p_mover->SST[i].Umin, 2) / 47.6);
                p_mover->SST[i].Pmax = std::min(p_mover->SST[i].Pmax, std::pow(p_mover->SST[i].Umax, 2) / 47.6);
            }
        }
    }
} // namespace godot
