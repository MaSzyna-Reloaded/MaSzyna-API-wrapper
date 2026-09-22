#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleElectricEngine.hpp"
#include "macros.hpp"
#include "resources/engines/RelayListItem.hpp"

namespace godot {
    class VehicleController;

    class VehicleElectricSeriesEngine : public VehicleElectricEngine {
            GDCLASS(VehicleElectricSeriesEngine, VehicleElectricEngine)
            enum StateProperty {
                STATE_RESISTOR_FAN_ROTATION,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            /* RVent= (Automatic / Yes / No): resistor cooling fan drive mode */
            enum FanType {
                FAN_TYPE_NONE,
                FAN_TYPE_YES,
                FAN_TYPE_AUTOMATIC,
            };

            static void _bind_methods();

        private:
            int state_base_index = 0;

        protected:
            EngineType get_engine_type() const override;
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;

        public:
            MAKE_MEMBER_GS(double, nominal_voltage, 0.0);
            MAKE_MEMBER_GS(double, winding_resistance, 0.0);
            MAKE_MEMBER_GS(double, max_rpm, 0.0);
            MAKE_MEMBER_GS_NR(FanType, resistor_fan_type, FAN_TYPE_NONE);
            MAKE_MEMBER_GS(double, resistor_fan_max_rpm, 1.0);
            MAKE_MEMBER_GS(double, resistor_fan_cutoff_resistance, 0.0);
            MAKE_MEMBER_GS(double, resistor_fan_min_current, 50.0);
            MAKE_MEMBER_GS(double, resistor_fan_speed, 0.5);
            MAKE_MEMBER_GS(double, dynamic_brake_resistance, 5.8);
            MAKE_MEMBER_GS(double, dynamic_brake_resistance_1, 5.8);
            MAKE_MEMBER_GS(double, dynamic_brake_resistance_2, 5.8);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<RelayListItem>, relay_list)
    };
} // namespace godot
VARIANT_ENUM_CAST(VehicleElectricSeriesEngine::FanType)
