#include "NotDefinedPowerSource.hpp"

namespace godot {
    void NotDefinedPowerSource::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_power_type"), &NotDefinedPowerSource::get_power_type);
        ClassDB::bind_method(D_METHOD("set_power_type", "value"), &NotDefinedPowerSource::set_power_type);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "power_type", PROPERTY_HINT_ENUM,
                        "NoPower,BioPower,MechPower,ElectricPower,SteamPower"),
                "set_power_type", "get_power_type");
    };

    TPowerSource NotDefinedPowerSource::get_source_type() const {
        return TPowerSource::NotDefined;
    }

    void NotDefinedPowerSource::update_config(TPowerParameters &p_power_parameters) const {
        PowerSource::update_config(p_power_parameters);
        p_power_parameters.PowerType = PowerSource::cast(power_type);
    }

    void NotDefinedPowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &state, const godot::String &prefix) const {
        PowerSource::fetch_config(p_power_parameters, state, prefix);
        state[prefix + godot::String("/power_type")] = PowerSource::to_string(power_type);
    }

    // GETTERS AND SETTERS
    void NotDefinedPowerSource::set_power_type(const PowerType p_power_type) {
        power_type = p_power_type;
        emit_signal(POWER_SOURCE_CHANGED);
    }

    PowerSource::PowerType NotDefinedPowerSource::get_power_type() const {
        return power_type;
    }
} // namespace godot
