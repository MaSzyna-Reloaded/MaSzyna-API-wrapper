#include "../core/VehicleController.hpp"
#include "VehicleHeating.hpp"

namespace godot {
    void VehicleHeating::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleHeating, Variant::INT, heating_source, "heating", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                VehicleHeating, Variant::INT, heating_generator_engine, "heating/generator", PROPERTY_HINT_ENUM,
                "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                "DieselElectric,Main");
        BIND_PROPERTY(VehicleHeating, Variant::FLOAT, heating_generator_min_rpm, "heating/generator");
        BIND_PROPERTY(VehicleHeating, Variant::FLOAT, heating_generator_min_voltage, "heating/generator");
        BIND_PROPERTY(VehicleHeating, Variant::FLOAT, heating_generator_max_rpm, "heating/generator");
        BIND_PROPERTY(VehicleHeating, Variant::FLOAT, heating_generator_max_voltage, "heating/generator");
        BIND_PROPERTY_W_HINT(
                VehicleHeating, Variant::INT, heating_power_cable_type, "heating/power_cable", PROPERTY_HINT_ENUM,
                "NoPower,BioPower,MechPower,ElectricPower,SteamPower");
        BIND_PROPERTY(VehicleHeating, Variant::FLOAT, heating_max_voltage, "heating");
        ClassDB::bind_method(D_METHOD("heating", "enabled"), &VehicleHeating::heating);
    }

    void VehicleHeating::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);

        p_mover->HeatingPowerSource.SourceType = train_controller_node->power_source_map.at(heating_source);
        p_mover->HeatingPowerSource.MaxVoltage = heating_max_voltage;

        switch (heating_source) {
            case VehicleController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551); HeatingCheck() dereferences it unconditionally whenever
                // SourceType == Generator, so it must be pointed at a real double before that can
                // run safely. enrot is the vehicle's own engine revolutions counter.
                p_mover->HeatingPowerSource.EngineGenerator.engine_revolutions = &p_mover->enrot;
                p_mover->HeatingPowerSource.EngineGenerator.revolutions_min = heating_generator_min_rpm / 60.0;
                p_mover->HeatingPowerSource.EngineGenerator.revolutions_max = heating_generator_max_rpm / 60.0;
                p_mover->HeatingPowerSource.EngineGenerator.voltage_min = heating_generator_min_voltage;
                p_mover->HeatingPowerSource.EngineGenerator.voltage_max = heating_generator_max_voltage;
                break;
            }
            case VehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->HeatingPowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(heating_power_cable_type);
                break;
            }
            default:
                break;
        }
    }

    void VehicleHeating::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        ASSERT_MOVER(p_mover);
        p_state["heating_enabled"] = p_mover->Heating;
        p_state["heating_power"] = p_mover->HeatingPower;
    }

    void VehicleHeating::heating(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_heatingenable/disable (Train.cpp:5256-5296) ->
        // HeatingSwitch(State) - "trainheating_sw:"/ggTrainHeatingButton (Train.cpp:10116).
        mover->HeatingSwitch(p_enabled);
    }

    void VehicleHeating::_register_commands() {
        VehicleComponent::_register_commands();
        register_command("heating", Callable(this, "heating"));
    }

    void VehicleHeating::_unregister_commands() {
        VehicleComponent::_unregister_commands();
        unregister_command("heating", Callable(this, "heating"));
    }
} // namespace godot
