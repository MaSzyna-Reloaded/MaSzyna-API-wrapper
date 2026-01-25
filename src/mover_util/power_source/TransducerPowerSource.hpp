#pragma once

#include "../../macros.hpp"
#include "PowerSource.hpp"

namespace godot {
    /** https://en.wikipedia.org/wiki/Transducer */
    /** Note that's propably deprecated class, as current MaSzyna assets doesn't contain any fiz file with this kind of
     * power source */
    class TransducerPowerSource : public PowerSource {
            GDCLASS(TransducerPowerSource, PowerSource)

            // TODO: check default value
            MAKE_MEMBER_GS(double, input_voltage, 0.0f);

        protected:
            static void _bind_methods();

        public:
            virtual TPowerSource get_source_type() const override {
                return TPowerSource::Transducer;
            }
            virtual void update_config(TPowerParameters &params, TMoverParameters &mover) const override;
            virtual void
            fetch_config(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
            virtual void
            fetch_state(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
    };
} // namespace godot
