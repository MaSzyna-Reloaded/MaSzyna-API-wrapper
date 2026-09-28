#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {
    /// How a scenario script reaches the cabs. The cab layer (CabinSystem) is written in GDScript,
    /// so the scripting layer cannot name it; a delegate implemented next to it forwards the
    /// scripts' manipulations and reports back what changed in a cab (control_changed).
    class ScenarioScriptCabinDelegate : public Resource {
            GDCLASS(ScenarioScriptCabinDelegate, Resource)

        protected:
            static void _bind_methods();

            GDVIRTUAL5R(Variant, _act, RID, int, StringName, StringName, Variant)
            GDVIRTUAL3RC(Variant, _get_control, RID, int, StringName)
            GDVIRTUAL2RC(Array, _get_controls, RID, int)
            GDVIRTUAL1RC(int, _get_occupied_cab, RID)

        public:
            /// A control of a cab changed (vehicle: RID, cab: int, control_id: StringName, value)
            static const char *control_changed_signal;

            /// Manipulates a control of the vehicle's cab; returns what the control answered
            Variant act(
                    const RID &p_vehicle, int p_cab, const StringName &p_control_id, const StringName &p_action,
                    const Variant &p_value);
            Variant get_control(const RID &p_vehicle, int p_cab, const StringName &p_control_id) const;
            Array get_controls(const RID &p_vehicle, int p_cab) const;
            int get_occupied_cab(const RID &p_vehicle) const;
    };
} // namespace godot
