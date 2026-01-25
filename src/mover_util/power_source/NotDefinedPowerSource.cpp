#include "NotDefinedPowerSource.hpp"

namespace godot {
    void NotDefinedPowerSource::_bind_methods() {
        // intentionally left blank
    };

    void NotDefinedPowerSource::update_config(TPowerParameters &p_power_parameters, TMoverParameters &p_mover) const {
        PowerSource::update_config(p_power_parameters, p_mover);
        p_power_parameters.SourceType = TPowerSource::NotDefined;
        p_power_parameters.PowerType = TPowerType::NoPower;
    }

    void NotDefinedPowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &state, const String &prefix) const {
        PowerSource::fetch_config(p_power_parameters, state, prefix);
        state[prefix + String("/power_type")] = PowerSource::to_string(PowerType::NoPower);
    }

    void NotDefinedPowerSource::fetch_state(
            const TPowerParameters &p_power_parameters, godot::Dictionary &state, const String &prefix) const {
        PowerSource::fetch_state(p_power_parameters, state, prefix);
        // No dynamic state to fetch for NotDefined power source
    }
} // namespace godot
