#include "VehicleElectricSeriesEngine.hpp"
#include "macros.hpp"

#include <algorithm>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleElectricSeriesEngine::_bind_methods() {
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, nominal_voltage);
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, winding_resistance);
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, max_rpm);
        BIND_PROPERTY_W_HINT(
                VehicleElectricSeriesEngine, Variant::INT, resistor_fan_type, "resistor_fan", PROPERTY_HINT_ENUM,
                "None,Yes,Automatic");
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, resistor_fan_max_rpm, "resistor_fan");
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, resistor_fan_cutoff_resistance, "resistor_fan");
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, resistor_fan_min_current, "resistor_fan");
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, resistor_fan_speed, "resistor_fan");
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, dynamic_brake_resistance);
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, dynamic_brake_resistance_1);
        BIND_PROPERTY(VehicleElectricSeriesEngine, Variant::FLOAT, dynamic_brake_resistance_2);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleElectricSeriesEngine, Variant::ARRAY, relay_list, PROPERTY_HINT_TYPE_STRING, "RelayListItem");

        BIND_ENUM_CONSTANT(FAN_TYPE_NONE);
        BIND_ENUM_CONSTANT(FAN_TYPE_YES);
        BIND_ENUM_CONSTANT(FAN_TYPE_AUTOMATIC);

        ClassDB::bind_method(
                D_METHOD("get_resistor_fan_rotation"), &VehicleElectricSeriesEngine::get_resistor_fan_rotation);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "resistor_fan_rotation", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_resistor_fan_rotation");
    }

    VehicleEngine::EngineType VehicleElectricSeriesEngine::get_engine_type() const {
        return VehicleEngine::EngineType::ELECTRIC_SERIES_MOTOR;
    }

    void VehicleElectricSeriesEngine::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleElectricEngine::_fill_state_dictionary(p_state);
        if (!is_simulation_ready()) {
            return;
        }
        p_state["resistor_fan_rotation"] = get_resistor_fan_rotation();
    }

} // namespace godot
