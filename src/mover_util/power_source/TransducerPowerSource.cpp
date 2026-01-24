#include "TransducerPowerSource.hpp"

namespace godot {
    void TransducerPowerSource::_bind_methods() {
        // TODO: Should be double?
        BIND_PROPERTY(
                Variant::FLOAT, "input_voltage", "input_voltage", &TransducerPowerSource::get_input_voltage,
                &TransducerPowerSource::set_input_voltage, "input_voltage");
    };

    void TransducerPowerSource::update_config(TPowerParameters &params, TMoverParameters &mover) const {
        params.SourceType = TPowerSource::Transducer;
        params.Transducer.InputVoltage = input_voltage;
    }

    void
    TransducerPowerSource::fetch_config(const TPowerParameters &params, Dictionary &state, const String &prefix) const {
        state[prefix + String("/source_type")] = to_string(TPowerSource::Transducer);
        state[prefix + String("/input_voltage")] = input_voltage;
    }
    // TODO: changing voltage will be probably dirty
} // namespace godot
