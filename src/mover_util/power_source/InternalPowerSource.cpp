#include "InternalPowerSource.hpp"

namespace godot {

    void InternalPowerSource::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_power_type"), &InternalPowerSource::get_power_type);
        ClassDB::bind_method(D_METHOD("set_power_type", "value"), &InternalPowerSource::set_power_type);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "power_type", PROPERTY_HINT_ENUM,
                        "NoPower,BioPower,MechPower,ElectricPower,SteamPower"),
                "set_power_type", "get_power_type");
    };

    TPowerSource InternalPowerSource::get_source_type() const {
        return TPowerSource::InternalSource;
    }

    void InternalPowerSource::update_config(TPowerParameters &p_power_parameters) const {
        PowerSource::update_config(p_power_parameters);
        p_power_parameters.PowerType = PowerSource::cast(power_type);
    }

    void InternalPowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &state, const godot::String &prefix) const {
        PowerSource::fetch_config(p_power_parameters, state, prefix);
        state[prefix + godot::String("/power_type")] = PowerSource::to_string(power_type);
    }

    // GETTERS AND SETTERS
    void InternalPowerSource::set_power_type(const PowerType p_power_type) {
        power_type = p_power_type;
        emit_signal(POWER_SOURCE_CHANGED);
    }

    PowerSource::PowerType InternalPowerSource::get_power_type() const {
        return power_type;
    }
} // namespace godot
