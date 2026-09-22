#pragma once
#include "../core/VehicleComponent.hpp"
#include "../engines/VehicleEngine.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;
    class VehicleHeating : public VehicleComponent {
            GDCLASS(VehicleHeating, VehicleComponent);

        private:
            static void _bind_methods();

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;

            enum StateProperty {
                STATE_HEATING_ENABLED,
                STATE_HEATING_POWER,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerSource, heating_source,
                    VehicleController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS_NR(VehicleEngine::EngineType, heating_generator_engine, VehicleEngine::EngineType::MAIN);
            MAKE_MEMBER_GS(double, heating_generator_min_rpm, 0.0);
            MAKE_MEMBER_GS(double, heating_generator_min_voltage, 0.0);
            MAKE_MEMBER_GS(double, heating_generator_max_rpm, 0.0);
            MAKE_MEMBER_GS(double, heating_generator_max_voltage, 0.0);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerType, heating_power_cable_type,
                    VehicleController::TrainPowerType::POWER_TYPE_ELECTRIC);
            MAKE_MEMBER_GS(double, heating_max_voltage, 0.0);

            void heating(bool p_enabled);
            void _register_commands() override;
            void _unregister_commands() override;
    };
} // namespace godot
