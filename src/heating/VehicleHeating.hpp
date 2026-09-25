#pragma once
#include "../core/VehicleComponent.hpp"
#include "../engines/VehicleEngine.hpp"
#include "macros.hpp"

namespace godot {
    class VehicleController;

    /* Train heating, as a vehicle offers it - the public contract, with no backend in it.
     *
     * The configuration is the wrapper's own and lives here, because the wrapper's properties and
     * enums are the authoring source of truth. The live values are pure virtual: whichever
     * simulation is underneath answers them, and a caller that took this component through
     * VehicleServer sees the same interface either way. */
    class VehicleHeating : public VehicleComponent {
            GDCLASS(VehicleHeating, VehicleComponent);


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_HEATING;
            }

        private:
            static void _bind_methods();

        public:
            /* Live state */
            virtual bool get_active() const = 0;
            /* The heating switch's own state (HeatingAllow) - what trainheating_sw flips
             * (Train.cpp:6674) */
            virtual bool get_allowed() const = 0;
            virtual double get_power() const = 0;

            /* Original engine: OnCommand_heatingenable/disable (Train.cpp:5256-5296) ->
             * HeatingSwitch(State) - "trainheating_sw:"/ggTrainHeatingButton (Train.cpp:10116). */
            virtual void heating(bool p_enabled) = 0;

            void _register_commands() override;
            void _unregister_commands() override;

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
    };
} // namespace godot
