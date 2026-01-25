#include "InternalPowerSource.hpp"

namespace godot {

    void InternalPowerSource::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                Variant::INT, "power_type", "power_type", &InternalPowerSource::set_power_type,
                &InternalPowerSource::get_power_type, "power_type", PROPERTY_HINT_ENUM,
                "NoPower,BioPower,MechPower,ElectricPower,SteamPower");
    };

    void InternalPowerSource::update_config(TPowerParameters &p_power_parameters, TMoverParameters &p_mover) const {
        PowerSource::update_config(p_power_parameters, p_mover);
        p_power_parameters.SourceType = TPowerSource::InternalSource;
        p_power_parameters.PowerType = PowerSource::cast(power_type);
    }

    void InternalPowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, Dictionary &state, const String &prefix) const {
        PowerSource::fetch_config(p_power_parameters, state, prefix);
        state[prefix + godot::String("/source_type")] = PowerSource::to_string(TPowerSource::InternalSource);
        state[prefix + godot::String("/power_type")] = PowerSource::to_string(power_type);
    }

    void InternalPowerSource::fetch_state(
            const TPowerParameters &p_power_parameters, Dictionary &state, const String &prefix) const {
        PowerSource::fetch_state(p_power_parameters, state, prefix);
        // No dynamic state to fetch for Internal power source
    }

} // namespace godot
