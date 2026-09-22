#pragma once
#include "MoverElectricTraction.hpp"
#include "VehicleDieselEngine.hpp"
#include "macros.hpp"
#include "resources/engines/WWListItem.hpp"

namespace godot {
    class VehicleDieselElectricEngine : public VehicleDieselEngine {
            GDCLASS(VehicleDieselElectricEngine, VehicleDieselEngine)


        private:
            /* Polish diesel-electrics are a diesel engine driving electric traction motors, so
             * this takes the same traction delegate a catenary-fed locomotive uses. */
            MoverElectricTraction traction;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            double get_motor_current() const;
            double get_circuit_imax() const;
            bool get_dynamic_brake_active() const;
            bool get_fuse_active() const;
            bool get_motor_connectors_open() const;
            void fuse_reset();
            void set_motor_connectors_open(bool p_open);
            void _register_commands() override;
            void _unregister_commands() override;
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
            void _do_update_internal_mover(TMoverParameters *p_mover) override;

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
