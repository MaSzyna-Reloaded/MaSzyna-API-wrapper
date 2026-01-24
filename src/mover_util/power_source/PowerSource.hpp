#pragma once

#include "../../macros.hpp"
#include "../../maszyna/McZapkie/MOVER.h"
#include "../../mover_util/power_source/PowerTypeEnum.hpp"
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class PowerSource : public Resource {
            GDCLASS(PowerSource, Resource)

            MAKE_MEMBER_GS(double, max_voltage, 0.0);
            MAKE_MEMBER_GS(double, max_current, 0.0);
            MAKE_MEMBER_GS(double, int_r, 0.001);

        protected:
            static void _bind_methods();

        public:
            static constexpr char const *const POWER_SOURCE_CHANGED = "power_source_changed";

            static TPowerType cast(PowerType p);
            static String to_string(PowerType p);
            static String to_string(TPowerSource p);
            static PowerType from_string(const String &p);

            /** Translates internal enum into readable form */
            static String get_power_source_name(const TPowerSource s);

            /** Updates the MOVER.h representation of the power source */
            virtual void update_config(TPowerParameters &p_power_parameters, TMoverParameters &p_mover) const;

            /** Fetches the config of the power source into the Godot state Dictionary. */
            virtual void fetch_config(
                    const TPowerParameters &p_power_parameters, Dictionary &p_config, const String &p_prefix) const;

            /** Fetches the state of the power source into the Godot state Dictionary. */
            virtual void
            fetch_state(const TPowerParameters &p_power_parameters, Dictionary &p_state, const String &p_prefix) const;
    };
} // namespace godot
