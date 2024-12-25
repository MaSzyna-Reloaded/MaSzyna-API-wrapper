#pragma once

#include <godot_cpp/classes/resource.hpp>
#include "../../maszyna/McZapkie/MOVER.h"

namespace godot {
    class PowerSource : public Resource {
            GDCLASS(PowerSource, Resource)

        protected:
            static void _bind_methods();

            /** Returns the internal MOVER.h representaion of the kind of power */
            virtual TPowerSource get_source_type() const = 0;

        public:
            static constexpr char const *const POWER_SOURCE_CHANGED = "power_source_changed";
            enum PowerType { NoPower, BioPower, MechPower, ElectricPower, SteamPower };

            static TPowerType cast(PowerType p);
            static String to_string(PowerType p);
            static PowerType from_string(const String &p);

            /** Translates internal enum into readable form */
            static String get_power_source_name(const TPowerSource s);

            /** Updates the MOVER.h representation of the power source */
            virtual void update_state(TPowerParameters &p_power_parameters) const;

            /** Fetches the state of the power source into the Godot state Dictionary. */
            virtual void
            fetch_state(const TPowerParameters &p_power_parameters, Dictionary &state, const String &prefix) const;
    };
} // namespace godot

VARIANT_ENUM_CAST(PowerSource::PowerType);
