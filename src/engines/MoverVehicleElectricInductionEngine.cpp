#include "../mover/MoverBackend.hpp"
#include "MoverVehicleElectricInductionEngine.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleElectricInductionEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverVehicleElectricInductionEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverVehicleElectricInductionEngine::get_circuit_imax);
        ClassDB::bind_method(
                D_METHOD("get_dynamic_brake_active"), &MoverVehicleElectricInductionEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverVehicleElectricInductionEngine::get_fuse_active);
        ClassDB::bind_method(
                D_METHOD("get_motor_connectors_open"), &MoverVehicleElectricInductionEngine::get_motor_connectors_open);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverVehicleElectricInductionEngine::fuse_reset);
        ClassDB::bind_method(
                D_METHOD("set_motor_connectors_open", "open"),
                &MoverVehicleElectricInductionEngine::set_motor_connectors_open);
    }

    double MoverVehicleElectricInductionEngine::get_motor_current() const {
        return traction.get_motor_current(this);
    }

    double MoverVehicleElectricInductionEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(this);
    }

    bool MoverVehicleElectricInductionEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(this);
    }

    bool MoverVehicleElectricInductionEngine::get_fuse_active() const {
        return traction.get_fuse_active(this);
    }

    bool MoverVehicleElectricInductionEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(this);
    }

    void MoverVehicleElectricInductionEngine::fuse_reset() {
        traction.reset_fuse(this);
    }

    void MoverVehicleElectricInductionEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(this, p_open);
    }

    void MoverVehicleElectricInductionEngine::_register_commands() {
        VehicleElectricInductionEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleElectricInductionEngine::_unregister_commands() {
        VehicleElectricInductionEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleElectricInductionEngine::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleElectricInductionEngine::_apply_configuration();

        p_mover->eimc[Maszyna::eimc_s_dfic] = get_slip_current_ratio();
        p_mover->eimc[Maszyna::eimc_s_dfmax] = get_max_slip();
        p_mover->eimc[Maszyna::eimc_s_p] = get_pole_pairs();
        p_mover->eimc[Maszyna::eimc_s_cfu] = get_nominal_uf_ratio();
        p_mover->eimc[Maszyna::eimc_s_cim] = get_current_torque_ratio();
        p_mover->eimc[Maszyna::eimc_s_icif] = get_current_three_phase_ratio();
        p_mover->eimc[Maszyna::eimc_f_Uzmax] = get_max_supply_voltage();
        p_mover->eimc[Maszyna::eimc_f_Uzh] = get_max_supply_voltage_braking();
        p_mover->eimc[Maszyna::eimc_f_DU] = get_inverter_voltage_drop();
        p_mover->eimc[Maszyna::eimc_f_I0] = get_no_load_current();
        p_mover->eimc[Maszyna::eimc_f_cfu] = get_inverter_uf_setpoint();
        p_mover->eimc[Maszyna::eimc_f_cfuH] = get_inverter_uf_setpoint_braking();
        p_mover->eimc[Maszyna::eimc_p_F0] = get_initial_force();
        p_mover->eimc[Maszyna::eimc_p_a1] = get_force_drop_rate();
        p_mover->eimc[Maszyna::eimc_p_Pmax] = get_max_power();
        p_mover->eimc[Maszyna::eimc_p_Fh] = get_max_braking_force();
        p_mover->eimc[Maszyna::eimc_p_Ph] = get_max_braking_power();
        p_mover->eimc[Maszyna::eimc_p_Vh0] = get_braking_decay_velocity();
        p_mover->eimc[Maszyna::eimc_p_Vh1] = get_braking_decay_start_velocity();
        p_mover->eimc[Maszyna::eimc_p_Imax] = get_motor_max_current();
        // the rest of LoadFIZ_Engine's induction motor block (Mover.cpp:11276-11306); edep defaults
        // to eimc[eimc_p_eped] = 1.5 (Mover.cpp:497), InvCtrCplFlag to InverterControlCouplerFlag{4}
        p_mover->eimc[Maszyna::eimc_p_abed] = get_electrodynamic_brake_cylinder_ratio();
        p_mover->eimc[Maszyna::eimc_p_eped] = get_electrodynamic_ep_ratio();
        p_mover->NominalVoltage = get_nominal_voltage();
        p_mover->EIMCLogForce = get_logarithmic_force_control();
        p_mover->InverterControlCouplerFlag = get_inverter_control_coupler_flag();
        p_mover->Flat = get_flat_force_characteristic();
        // Mover.cpp:11302 - a powered EIM without InvNo has one inverter; with none the traction
        // step divides by InvertersNo (Mover.cpp:5627) and every force becomes NaN
        if (p_mover->eimc[Maszyna::eimc_p_Pmax] > 0 && p_mover->Power > 0 && p_mover->InvertersNo == 0) {
            p_mover->InvertersNo = 1;
        }
        p_mover->Inverters.resize(p_mover->InvertersNo);

        /* Pmaxlist: tabela mocy maksymalnej od predkosci (niedokumentowana na wiki, patrz EIM_Pmax_Table w MOVER.h) */
        p_mover->EIM_Pmax_Table.clear();
        for (int i = 0; i < get_max_power_table().size(); i++) {
            const Ref<CurvePointItem> &row = get_max_power_table()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[MoverVehicleElectricInductionEngine]: max_power_table property is null at index " +
                        String::num(i));
                continue;
            }
            p_mover->EIM_Pmax_Table.emplace(row->get_x(), row->get_y());
        }

        /* ffList:/ffBrakeList: -> DElist/RlistSize, read by TractionForce()'s
         * ElectricInductionMotor branch to compute InverterFrequency (Mover.cpp:5895-5908).
         * RlistSize mirrors VehicleElectricSeriesEngine's own RList:-driven wiring exactly - it's
         * a single mover-wide field, so only one engine part may legitimately drive it. */
        const int max_delist = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(get_wwlist().size());
        p_mover->RlistSize = std::min(max_delist, wwlist_size);
        for (int i = 0; i < p_mover->RlistSize; i++) {
            const Ref<WWListItem> &row = get_wwlist()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[MoverVehicleElectricInductionEngine]: wwlist property is null at index " + String::num(i));
                continue;
            }
            p_mover->DElist[i].RPM = row->get_rpm();
            p_mover->DElist[i].GenPower = row->get_max_power();
        }
    }
} // namespace godot
