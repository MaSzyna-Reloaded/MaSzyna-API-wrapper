#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
    /// The behaviour of a DriverSystem driver: what it makes of the orders it gets, and later how it
    /// drives. A driver is always the same object of the system; what differs between the original's
    /// AI driver and any other is its delegate. Implement it in C++ by overriding the virtual methods,
    /// or in GDScript by overriding their script counterparts.
    ///
    /// Every callback carries the driver: a Resource is shared, so one delegate serves several drivers
    /// and keeps what it needs per driver RID. A delegate drives the vehicle only the way a player
    /// does, through the cab.
    class DriverDelegate : public Resource {
            GDCLASS(DriverDelegate, Resource)
            friend class DriverSystem;

        protected:
            static void _bind_methods();

            GDVIRTUAL1(_driver_attached, RID)
            GDVIRTUAL1(_driver_detached, RID)
            GDVIRTUAL5(_handle_command, RID, String, double, double, Vector3)

            /// Called by DriverSystem. A C++ delegate overrides these; the default forwards to the
            /// script.
            virtual void driver_attached(const RID &p_driver);
            virtual void driver_detached(const RID &p_driver);
            /// An order for the driver, with where what sent it stands
            virtual void handle_command(
                    const RID &p_driver, const String &p_command, double p_value1, double p_value2,
                    const Vector3 &p_position);
    };
} // namespace godot
