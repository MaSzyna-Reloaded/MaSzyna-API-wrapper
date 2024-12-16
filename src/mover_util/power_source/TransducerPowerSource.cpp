#include "TransducerPowerSource.hpp"

namespace godot {
    void TransducerPowerSource::_bind_methods() {
        godot::ClassDB::bind_method(D_METHOD("get_input_voltage"), &TransducerPowerSource::get_input_voltage);
        godot::ClassDB::bind_method(D_METHOD("set_input_voltage", "value"), &TransducerPowerSource::set_input_voltage);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "input_voltage"), "set_input_voltage", "get_input_voltage");
    };

    TPowerSource TransducerPowerSource::get_source_type() const {
        return TPowerSource::Transducer;
    }

    void TransducerPowerSource::update_config(TPowerParameters &p_power_parameters) const {
        PowerSource::update_config(p_power_parameters);
        p_power_parameters.Transducer.InputVoltage = input_voltage;
    }

    void TransducerPowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &state, const godot::String &prefix) const {
        PowerSource::fetch_config(p_power_parameters, state, prefix);
        state[prefix + godot::String("/input_voltage")] = input_voltage;
    }

    double TransducerPowerSource::get_input_voltage() const {
        return input_voltage;
    }

    void TransducerPowerSource::set_input_voltage(double p_input_voltage) {
        input_voltage = p_input_voltage;
        emit_signal(POWER_SOURCE_CHANGED);
    }
} // namespace godot
