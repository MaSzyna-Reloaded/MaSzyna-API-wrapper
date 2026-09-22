#pragma once
#include "MoverEngineBackend.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleEngine.hpp"
#include "macros.hpp"
#include "resources/engines/CurvePointItem.hpp"
#include "resources/engines/ThrottlePositionItem.hpp"

namespace godot {
    class VehicleController;

    class VehicleDieselEngine : public VehicleEngine {
            GDCLASS(VehicleDieselEngine, VehicleEngine)
            

        private:
            MoverEngineBackend engine_backend;

        public:
            VehicleDieselEngine() { backend = &engine_backend; }

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            /* Live state, read straight from the backend - nothing is stored. */
            double get_rpm() const;
            bool get_oil_pump_active() const;
            bool get_oil_pump_disabled() const;
            double get_oil_pump_pressure() const;
            bool get_fuel_pump_active() const;
            bool get_fuel_pump_disabled() const;
            bool get_startup() const;
            bool get_ignition() const;
            bool get_spinup() const;
            double get_output_power() const;
            double get_torque() const;
            double get_fill() const;
            double get_max_rpm() const;

            /* R_Place= : retarder location within the mechanical transmission */
            enum RetarderPlacement {
                RETARDER_PLACEMENT_AFTER_GEARBOX,
                RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC,
                RETARDER_PLACEMENT_BETWEEN_TC_AND_ENGINE,
            };

        private:
            static void _bind_methods();
            MAKE_MEMBER_GS(float, oil_pump_pressure_minimum, 0.0);
            MAKE_MEMBER_GS(float, oil_pump_pressure_maximum, 0.65);
            MAKE_MEMBER_GS_NR(StartMode, fuel_pump_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, oil_pump_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, water_pump_start_mode, START_MODE_MANUAL);

            /* Engine: (Kont.), przekladnia mechaniczna */
            MAKE_MEMBER_GS(double, mechanical_min_rpm, 0.0);
            MAKE_MEMBER_GS(double, mechanical_max_rpm, 0.0);
            MAKE_MEMBER_GS(double, mechanical_fuel_cutoff_rpm, 0.0);
            MAKE_MEMBER_GS(double, mechanical_inertia, 1.0);
            MAKE_MEMBER_GS(double, mechanical_clutch_engage_speed, 0.5);
            MAKE_MEMBER_GS(double, mechanical_clutch_disengage_speed, 0.9);
            MAKE_MEMBER_GS(bool, torque_converter_present, false);
            MAKE_MEMBER_GS(double, torque_converter_max_torque_ratio, 2.0);
            MAKE_MEMBER_GS(double, torque_converter_coupling_point, 0.85);
            MAKE_MEMBER_GS(double, torque_converter_lockup_torque, 3000.0);
            MAKE_MEMBER_GS(double, torque_converter_lockup_rate, 1.0);
            MAKE_MEMBER_GS(double, torque_converter_unlock_rate, 1.0);
            MAKE_MEMBER_GS(double, torque_converter_fill_rate_increase, 1.0);
            MAKE_MEMBER_GS(double, torque_converter_fill_rate_decrease, 1.0);
            MAKE_MEMBER_GS(double, torque_converter_torque_in_in, 4.5);
            MAKE_MEMBER_GS(double, torque_converter_torque_in_out, 0.0);
            MAKE_MEMBER_GS(double, torque_converter_torque_out_out, 0.0);
            MAKE_MEMBER_GS(double, torque_converter_lockup_speed, 1.0);
            MAKE_MEMBER_GS(double, torque_converter_unlock_speed, 1.0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<CurvePointItem>, torque_converter_table)
            /* V2NList: predkosc -> maksymalne obroty silnika (dizel_vel2nmax_Table) */
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<CurvePointItem>, vel2nmax_table)
            MAKE_MEMBER_GS(bool, retarder_present, false);
            MAKE_MEMBER_GS_NR(RetarderPlacement, retarder_placement, RETARDER_PLACEMENT_AFTER_GEARBOX);
            MAKE_MEMBER_GS(double, retarder_torque_in_in, 1.0);
            MAKE_MEMBER_GS(double, retarder_max_torque, 1.0);
            MAKE_MEMBER_GS(double, retarder_max_power, 1.0);
            MAKE_MEMBER_GS(double, retarder_fill_rate_increase, 1.0);
            MAKE_MEMBER_GS(double, retarder_fill_rate_decrease, 1.0);
            MAKE_MEMBER_GS(double, retarder_min_velocity, 1.0);

            /* DList: tabela przepustnicy */
            MAKE_MEMBER_GS(double, throttle_table_max_torque, 1.0);
            MAKE_MEMBER_GS(double, throttle_table_max_torque_rpm, 1.0);
            MAKE_MEMBER_GS(double, throttle_table_max_rpm_torque, 2.0);
            MAKE_MEMBER_GS(double, throttle_table_nominal_fuel_dose, 0.0);
            MAKE_MEMBER_GS(double, throttle_table_resistance_torque, 0.0);
            MAKE_MEMBER_GS(double, throttle_table_nominal_fuel_consumption_rate, 250.0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<ThrottlePositionItem>, throttle_table_positions)

            /* DMList: charakterystyka momentu obrotowego */
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<CurvePointItem>, torque_table)

        private:

        protected:
            EngineType get_engine_type() const override;
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            void oil_pump(bool p_enabled);
            void fuel_pump(bool p_enabled);
    };
} // namespace godot
VARIANT_ENUM_CAST(VehicleDieselEngine::RetarderPlacement)
