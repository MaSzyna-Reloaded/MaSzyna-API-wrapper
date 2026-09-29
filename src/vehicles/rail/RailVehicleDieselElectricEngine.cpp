#include "RailVehicleDieselElectricEngine.hpp"

namespace godot {
    void RailVehicleDieselElectricEngine::_fill_state_dictionary(Dictionary &p_state) const {
        RailVehicleDieselEngine::_fill_state_dictionary(p_state);
        p_state["Im"] = get_motor_current();
        p_state["circuit_imax"] = get_circuit_imax();
        p_state["dynamic_brake_active"] = get_dynamic_brake_active();
        p_state["fuse_active"] = get_fuse_active();
        p_state["motor_connectors_open"] = get_motor_connectors_open();
        p_state["line_contactor_closed"] = is_line_contactor_closed();
        p_state["pressure_switch_tripped"] = is_pressure_switch_tripped();
    }

    void RailVehicleDieselElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleDieselElectricEngine, Variant::ARRAY, wwlist, PROPERTY_HINT_TYPE_STRING,
                "RailVehicleWWListItem");
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::BOOL, generator_voltage_flat);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::FLOAT, hyperbolic_speed);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::FLOAT, additional_speed);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::FLOAT, rpm_change_rate);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::FLOAT, power_correction_ratio);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::INT, shunt_relay_type);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::BOOL, shunt_mode_allowed);
        BIND_PROPERTY(RailVehicleDieselElectricEngine, Variant::FLOAT, heating_rpm);
    }

    RailVehicleEngine::EngineType RailVehicleDieselElectricEngine::get_engine_type() const {
        return RailVehicleEngine::EngineType::DIESEL_ELECTRIC;
    }
} // namespace godot
