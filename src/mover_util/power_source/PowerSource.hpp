#pragma once

#include "../../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class PowerSource : public Resource {
            GDCLASS(PowerSource, Resource);

        protected:
            static void _bind_methods();

        public:
            static constexpr char const *const POWER_SOURCE_CHANGED = "power_source_changed";

            enum PowerType { NoPower, BioPower, MechPower, ElectricPower, SteamPower };

            static TPowerType cast(PowerType p);
            static String to_string(PowerType p);
            static PowerType from_string(const String &p);

            /** Factory method for PowerSource */
            static Ref<PowerSource> create(const TPowerParameters &p_power_parameters);

            /** Translates internal enum into readable form */
            static String get_power_source_name(const TPowerSource s);

            /** Returns the internal MOVER.h representaion of the kind of power */
            virtual TPowerSource get_source_type() const = 0;

            /** Updates the MOVER.h representation of the power source */
            virtual void update_config(TPowerParameters &p_power_parameters) const;

            /** Fetches the config of the power source into the Godot state Dictionary. */
            virtual void fetch_config(
                    const TPowerParameters &p_power_parameters, Dictionary &p_config, const String &p_prefix) const;
    };
} // namespace godot

VARIANT_ENUM_CAST(PowerSource::PowerType);
