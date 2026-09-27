#include "VehicleDieselElectricEngine.hpp"

namespace godot {
    void VehicleDieselElectricEngine::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleDieselEngine::_fill_state_dictionary(p_state);
        p_state["Im"] = get_motor_current();
        p_state["circuit_imax"] = get_circuit_imax();
        p_state["dynamic_brake_active"] = get_dynamic_brake_active();
        p_state["fuse_active"] = get_fuse_active();
        p_state["motor_connectors_open"] = get_motor_connectors_open();
        p_state["line_contactor_closed"] = is_line_contactor_closed();
        p_state["pressure_switch_tripped"] = is_pressure_switch_tripped();
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
    }

    VehicleEngine::EngineType VehicleDieselElectricEngine::get_engine_type() const {
        return VehicleEngine::EngineType::DIESEL_ELECTRIC;
    }
} // namespace godot
