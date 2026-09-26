#pragma once
#include "VehicleDieselEngine.hpp"
#include "macros.hpp"
#include "resources/engines/WWListItem.hpp"

namespace godot {
    class VehicleDieselElectricEngine : public VehicleDieselEngine {
            GDCLASS(VehicleDieselElectricEngine, VehicleDieselEngine)
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            virtual double get_motor_current() const = 0;
            virtual double get_circuit_imax() const = 0;
            virtual bool get_dynamic_brake_active() const = 0;
            virtual bool get_fuse_active() const = 0;
            virtual bool get_motor_connectors_open() const = 0;
            /* The line contactors are closed (StLinFlag); the control pressure switch tripped - the
             * brake cylinder or pipe pressure is out of its working range (ControlPressureSwitch,
             * Mover.cpp:7177) */
            virtual bool is_line_contactor_closed() const = 0;
            virtual bool is_pressure_switch_tripped() const = 0;
            virtual void fuse_reset() = 0;
            virtual void set_motor_connectors_open(bool p_open) = 0;

        private:
            static void _bind_methods();
            TypedArray<WWListItem> wwlist;

            /* Engine: (Kont.), przekladnia elektryczna */
            MAKE_MEMBER_GS(bool, generator_voltage_flat, false);
            MAKE_MEMBER_GS(double, hyperbolic_speed, 1.0);
            MAKE_MEMBER_GS(double, additional_speed, 1.0);
            MAKE_MEMBER_GS(double, rpm_change_rate, 2.0);
            MAKE_MEMBER_GS(double, power_correction_ratio, 1.0);
            MAKE_MEMBER_GS(int, shunt_relay_type, 0);
            MAKE_MEMBER_GS(bool, shunt_mode_allowed, false);
            MAKE_MEMBER_GS(double, heating_rpm, 0.0);

        protected:
            VehicleEngine::EngineType get_engine_type() const override;

        public:
            TypedArray<WWListItem> get_wwlist() {
                return wwlist;
            }

            void set_wwlist(const TypedArray<WWListItem> &p_wwlist) {
                wwlist.clear();
                wwlist.append_array(p_wwlist);
            }
    };
} // namespace godot
