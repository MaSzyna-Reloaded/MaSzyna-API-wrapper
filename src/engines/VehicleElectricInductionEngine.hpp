#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleElectricEngine.hpp"
#include "macros.hpp"
#include "resources/engines/CurvePointItem.hpp"
#include "resources/engines/WWListItem.hpp"

namespace godot {
    class VehicleController;

    class VehicleElectricInductionEngine : public VehicleElectricEngine {
            GDCLASS(VehicleElectricInductionEngine, VehicleElectricEngine)
        public:
            /// Mover.cpp:497 eimc[eimc_p_eped] = 1.5
            static constexpr double EIMC_P_EPED_DEFAULT = 1.5;
            /// MOVER.h InverterControlCouplerFlag{4}
            static constexpr int INVERTER_CONTROL_COUPLER_FLAG_DEFAULT = 4;

            static void _bind_methods();

        private:
            TypedArray<WWListItem> wwlist;

        protected:
            EngineType get_engine_type() const override;
            void _apply_configuration() override;

        public:
            TypedArray<WWListItem> get_wwlist() {
                return wwlist;
            }

            void set_wwlist(const TypedArray<WWListItem> &p_wwlist) {
                wwlist.clear();
                wwlist.append_array(p_wwlist);
            }

            MAKE_MEMBER_GS(double, slip_current_ratio, 0.0);
            MAKE_MEMBER_GS(double, max_slip, 0.0);
            MAKE_MEMBER_GS(double, pole_pairs, 0.0);
            MAKE_MEMBER_GS(double, nominal_uf_ratio, 0.0);
            MAKE_MEMBER_GS(double, current_torque_ratio, 0.0);
            MAKE_MEMBER_GS(double, current_three_phase_ratio, 0.0);
            MAKE_MEMBER_GS(double, max_supply_voltage, 0.0);
            MAKE_MEMBER_GS(double, max_supply_voltage_braking, 0.0);
            MAKE_MEMBER_GS(double, inverter_voltage_drop, 0.0);
            MAKE_MEMBER_GS(double, no_load_current, 0.0);
            MAKE_MEMBER_GS(double, inverter_uf_setpoint, 0.0);
            MAKE_MEMBER_GS(double, inverter_uf_setpoint_braking, 0.0);
            MAKE_MEMBER_GS(double, initial_force, 0.0);
            MAKE_MEMBER_GS(double, force_drop_rate, 0.0);
            MAKE_MEMBER_GS(double, max_power, 0.0);
            MAKE_MEMBER_GS(double, max_braking_force, 0.0);
            MAKE_MEMBER_GS(double, max_braking_power, 0.0);
            MAKE_MEMBER_GS(double, braking_decay_velocity, 0.0);
            MAKE_MEMBER_GS(double, braking_decay_start_velocity, 0.0);
            MAKE_MEMBER_GS(double, motor_max_current, 0.0);
            /* The rest of LoadFIZ_Engine's EIM block (Mover.cpp:11276-11306); defaults are the Mover's own. */
            MAKE_MEMBER_GS(double, nominal_voltage, 0.0);                  // Volt -> NominalVoltage
            MAKE_MEMBER_GS(double, electrodynamic_brake_cylinder_ratio, 0.0); // abed -> eimc[eimc_p_abed]
            MAKE_MEMBER_GS(double, electrodynamic_ep_ratio, EIMC_P_EPED_DEFAULT); // edep -> eimc[eimc_p_eped]
            MAKE_MEMBER_GS(bool, logarithmic_force_control, false);        // eimclf -> EIMCLogForce
            MAKE_MEMBER_GS(int, inverter_control_coupler_flag, INVERTER_CONTROL_COUPLER_FLAG_DEFAULT); // InvCtrCplFlag
            MAKE_MEMBER_GS(bool, flat_force_characteristic, false);        // Flat -> Flat
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<CurvePointItem>, max_power_table)
    };
} // namespace godot
