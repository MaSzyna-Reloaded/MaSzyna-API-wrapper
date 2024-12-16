#pragma once

#include "PowerSource.hpp"

namespace godot {
    /** https://en.wikipedia.org/wiki/Transducer */
    /** Note that's propably deprecated class, as current MaSzyna assets doesn't contain any fiz file with this kind of
     * power source */
    class TransducerPowerSource : public PowerSource {
            GDCLASS(TransducerPowerSource, PowerSource)

            // TODO: check default value
            double input_voltage = 0.0f;

        protected:
            static void _bind_methods();
            TPowerSource get_source_type() const override;

        public:
            void update_config(TPowerParameters &p_power_parameters) const override;
            void fetch_config(
                    const TPowerParameters &p_power_parameters, godot::Dictionary &state,
                    const godot::String &prefix) const override;

            // GETTERS AND SETTERS
            double get_input_voltage() const;
            void set_input_voltage(double p_input_voltage);
    };
} // namespace godot
