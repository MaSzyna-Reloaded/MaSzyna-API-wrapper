#pragma once
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    /* What a vehicle can be asked for.
     *
     * A consumer names the kind, not the implementation:
     * `vehicle_component_get(rid, VehicleComponentType.COMPONENT_ENGINE)` answers with a
     * VehicleEngine whether the vehicle is diesel or electric.
     *
     * It is a class of its own, holding nothing, for two reasons: GDScript only sees an enum's
     * constants as members of a registered class, and the component and the vehicle include each
     * other, so neither of them can own it. */
    class VehicleComponentType : public Object {
            GDCLASS(VehicleComponentType, Object)

        protected:
            static void _bind_methods();

        public:
            enum Type {
                COMPONENT_NONE,
                COMPONENT_BRAKES,
                COMPONENT_SPRING_BRAKE,
                COMPONENT_EP_ED_BRAKE,
                COMPONENT_BUFFERS,
                COMPONENT_DOORS,
                COMPONENT_ENGINE,
                COMPONENT_HEATING,
                COMPONENT_LIGHTING,
                COMPONENT_LOAD,
                COMPONENT_SPEED_CONTROL,
                COMPONENT_SWITCHES,
                COMPONENT_AI_HINTS,
                COMPONENT_HORNS,
                COMPONENT_SECURITY,
                COMPONENT_WHEELS,
                COMPONENT_WIPERS,
                COMPONENT_UNIVERSAL_CONTROLLER,
                COMPONENT_GENERIC,
                COMPONENT_RADIO,
            };
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleComponentType::Type);
