#pragma once

#include "PowerSource.hpp"

namespace godot {
    class NotDefinedPowerSource : public PowerSource {
            GDCLASS(NotDefinedPowerSource, PowerSource)

            PowerType power_type = NoPower;

        protected:
            static void _bind_methods();
            TPowerSource get_source_type() const override;

        public:
            void update_config(TPowerParameters &p_power_parameters) const override;
            void fetch_config(
                    const TPowerParameters &p_power_parameters, godot::Dictionary &p_config,
                    const godot::String &p_prefix) const override;

            // GETTERS AND SETTERS
            void set_power_type(const PowerType p_power_type);
            PowerType get_power_type() const;
    };
} // namespace godot
