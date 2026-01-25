#pragma once

#include "PowerSource.hpp"

namespace godot {
    class NotDefinedPowerSource : public PowerSource {
            GDCLASS(NotDefinedPowerSource, PowerSource)
        protected:
            static void _bind_methods();

        public:
            virtual TPowerSource get_source_type() const override {
                return TPowerSource::NotDefined;
            }
            void update_config(TPowerParameters &p_power_parameters, TMoverParameters &p_mover) const override;
            void fetch_config(
                    const TPowerParameters &p_power_parameters, godot::Dictionary &p_config,
                    const String &p_prefix) const override;
            void fetch_state(
                    const TPowerParameters &p_power_parameters, godot::Dictionary &p_state,
                    const String &p_prefix) const override;
    };
} // namespace godot
