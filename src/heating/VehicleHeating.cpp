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
        ClassDB::bind_method(D_METHOD("get_active"), &VehicleHeating::get_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active");
        ClassDB::bind_method(D_METHOD("get_allowed"), &VehicleHeating::get_allowed);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "allowed", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_allowed");
        ClassDB::bind_method(D_METHOD("get_power"), &VehicleHeating::get_power);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "power", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power");
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
