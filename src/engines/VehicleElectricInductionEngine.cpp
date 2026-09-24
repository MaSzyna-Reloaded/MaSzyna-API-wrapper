#include "VehicleElectricInductionEngine.hpp"

namespace godot {
    void VehicleElectricInductionEngine::_bind_methods() {
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, slip_current_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_slip);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, pole_pairs);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, nominal_uf_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, current_torque_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, current_three_phase_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_supply_voltage);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_supply_voltage_braking);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, inverter_voltage_drop);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, no_load_current);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, inverter_uf_setpoint);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, inverter_uf_setpoint_braking);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, initial_force);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, force_drop_rate);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_power);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_braking_force);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, max_braking_power);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, braking_decay_velocity);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, braking_decay_start_velocity);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, motor_max_current);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, nominal_voltage);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, electrodynamic_brake_cylinder_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::FLOAT, electrodynamic_ep_ratio);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::BOOL, logarithmic_force_control);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::INT, inverter_control_coupler_flag);
        BIND_PROPERTY(VehicleElectricInductionEngine, Variant::BOOL, flat_force_characteristic);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleElectricInductionEngine, Variant::ARRAY, max_power_table, PROPERTY_HINT_TYPE_STRING,
                "CurvePointItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleElectricInductionEngine, Variant::ARRAY, wwlist, PROPERTY_HINT_TYPE_STRING, "WWListItem");
    }

    VehicleEngine::EngineType VehicleElectricInductionEngine::get_engine_type() const {
        return VehicleEngine::EngineType::ELECTRIC_INDUCTION_MOTOR;
    }

    void VehicleElectricInductionEngine::set_nominal_voltage(const double p_value) {
        nominal_voltage = p_value;
    }
    double VehicleElectricInductionEngine::get_nominal_voltage() const {
        return nominal_voltage;
    }
    void VehicleElectricInductionEngine::set_electrodynamic_brake_cylinder_ratio(const double p_value) {
        electrodynamic_brake_cylinder_ratio = p_value;
    }
    double VehicleElectricInductionEngine::get_electrodynamic_brake_cylinder_ratio() const {
        return electrodynamic_brake_cylinder_ratio;
    }
    void VehicleElectricInductionEngine::set_electrodynamic_ep_ratio(const double p_value) {
        electrodynamic_ep_ratio = p_value;
    }
    double VehicleElectricInductionEngine::get_electrodynamic_ep_ratio() const {
        return electrodynamic_ep_ratio;
    }
    void VehicleElectricInductionEngine::set_logarithmic_force_control(const bool p_value) {
        logarithmic_force_control = p_value;
    }
    bool VehicleElectricInductionEngine::get_logarithmic_force_control() const {
        return logarithmic_force_control;
    }
    void VehicleElectricInductionEngine::set_inverter_control_coupler_flag(const int p_value) {
        inverter_control_coupler_flag = p_value;
    }
    int VehicleElectricInductionEngine::get_inverter_control_coupler_flag() const {
        return inverter_control_coupler_flag;
    }
    void VehicleElectricInductionEngine::set_flat_force_characteristic(const bool p_value) {
        flat_force_characteristic = p_value;
    }
    bool VehicleElectricInductionEngine::get_flat_force_characteristic() const {
        return flat_force_characteristic;
    }
} // namespace godot
