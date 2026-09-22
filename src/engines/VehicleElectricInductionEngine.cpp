#include "VehicleElectricInductionEngine.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    double VehicleElectricInductionEngine::get_motor_current() const {
        return traction.get_motor_current(get_mover());
    }

    double VehicleElectricInductionEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(get_mover());
    }

    bool VehicleElectricInductionEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(get_mover());
    }

    bool VehicleElectricInductionEngine::get_fuse_active() const {
        return traction.get_fuse_active(get_mover());
    }

    bool VehicleElectricInductionEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(get_mover());
    }

    void VehicleElectricInductionEngine::fuse_reset() {
        traction.reset_fuse(get_mover());
    }

    void VehicleElectricInductionEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(get_mover(), p_open);
    }

    void VehicleElectricInductionEngine::_register_commands() {
        VehicleElectricEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void VehicleElectricInductionEngine::_unregister_commands() {
        VehicleElectricEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

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
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleElectricInductionEngine, Variant::ARRAY, max_power_table, PROPERTY_HINT_TYPE_STRING,
                "CurvePointItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleElectricInductionEngine, Variant::ARRAY, wwlist, PROPERTY_HINT_TYPE_STRING, "WWListItem");

        ClassDB::bind_method(D_METHOD("fuse_reset"), &VehicleElectricInductionEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("set_motor_connectors_open", "open"), &VehicleElectricInductionEngine::set_motor_connectors_open);
    }

    VehicleEngine::EngineType VehicleElectricInductionEngine::get_engine_type() const {
        return VehicleEngine::EngineType::ELECTRIC_INDUCTION_MOTOR;
    }

    void VehicleElectricInductionEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleElectricEngine::_do_update_internal_mover(p_mover);

        p_mover->eimc[Maszyna::eimc_s_dfic] = slip_current_ratio;
        p_mover->eimc[Maszyna::eimc_s_dfmax] = max_slip;
        p_mover->eimc[Maszyna::eimc_s_p] = pole_pairs;
        p_mover->eimc[Maszyna::eimc_s_cfu] = nominal_uf_ratio;
        p_mover->eimc[Maszyna::eimc_s_cim] = current_torque_ratio;
        p_mover->eimc[Maszyna::eimc_s_icif] = current_three_phase_ratio;
        p_mover->eimc[Maszyna::eimc_f_Uzmax] = max_supply_voltage;
        p_mover->eimc[Maszyna::eimc_f_Uzh] = max_supply_voltage_braking;
        p_mover->eimc[Maszyna::eimc_f_DU] = inverter_voltage_drop;
        p_mover->eimc[Maszyna::eimc_f_I0] = no_load_current;
        p_mover->eimc[Maszyna::eimc_f_cfu] = inverter_uf_setpoint;
        p_mover->eimc[Maszyna::eimc_f_cfuH] = inverter_uf_setpoint_braking;
        p_mover->eimc[Maszyna::eimc_p_F0] = initial_force;
        p_mover->eimc[Maszyna::eimc_p_a1] = force_drop_rate;
        p_mover->eimc[Maszyna::eimc_p_Pmax] = max_power;
        p_mover->eimc[Maszyna::eimc_p_Fh] = max_braking_force;
        p_mover->eimc[Maszyna::eimc_p_Ph] = max_braking_power;
        p_mover->eimc[Maszyna::eimc_p_Vh0] = braking_decay_velocity;
        p_mover->eimc[Maszyna::eimc_p_Vh1] = braking_decay_start_velocity;
        p_mover->eimc[Maszyna::eimc_p_Imax] = motor_max_current;

        /* Pmaxlist: tabela mocy maksymalnej od predkosci (niedokumentowana na wiki, patrz EIM_Pmax_Table w MOVER.h) */
        p_mover->EIM_Pmax_Table.clear();
        for (int i = 0; i < max_power_table.size(); i++) {
            const Ref<CurvePointItem> &row = max_power_table[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleElectricInductionEngine]: max_power_table property is null at index " + String::num(i));
                continue;
            }
            p_mover->EIM_Pmax_Table.emplace(row->get_x(), row->get_y());
        }

        /* ffList:/ffBrakeList: -> DElist/RlistSize, read by TractionForce()'s
         * ElectricInductionMotor branch to compute InverterFrequency (Mover.cpp:5895-5908).
         * RlistSize mirrors VehicleElectricSeriesEngine's own RList:-driven wiring exactly - it's
         * a single mover-wide field, so only one engine part may legitimately drive it. */
        const int max_delist = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(wwlist.size());
        p_mover->RlistSize = std::min(max_delist, wwlist_size);
        for (int i = 0; i < p_mover->RlistSize; i++) {
            const Ref<WWListItem> &row = wwlist[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleElectricInductionEngine]: wwlist property is null at index " + String::num(i));
                continue;
            }
            p_mover->DElist[i].RPM = row->get_rpm();
            p_mover->DElist[i].GenPower = row->get_max_power();
        }
    }
} // namespace godot
