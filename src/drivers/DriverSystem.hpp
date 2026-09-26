#pragma once
#include "DriverDelegate.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot {
    /// RID based registry of drivers - who drives a vehicle. A driver takes the orders a scenario
    /// gives (driver_send_command()) and, through its DriverDelegate, what they mean; it drives the
    /// vehicle as a player does, through the cab. A player at the controls needs no driver.
    class DriverSystem : public Object {
            GDCLASS(DriverSystem, Object)

        public:
            static DriverSystem *get_instance() {
                return Object::cast_to<DriverSystem>(Engine::get_singleton()->get_singleton("DriverSystem"));
            }

        private:
            struct DriverData {
                    Ref<DriverDelegate> delegate;
                    RID vehicle;
            };

            HashMap<RID, DriverData> drivers;
            HashMap<RID, RID> drivers_by_vehicle;

            void _on_vehicle_freed(const RID &p_vehicle);

        protected:
            static void _bind_methods();

        public:
            DriverSystem();

            RID driver_create();
            void driver_free(const RID &p_driver);
            void driver_attach_delegate(const RID &p_driver, const Ref<DriverDelegate> &p_delegate);
            Ref<DriverDelegate> driver_get_delegate(const RID &p_driver) const;
            /// The RailVehicleServer vehicle the driver drives; one driver a vehicle
            void driver_attach_vehicle(const RID &p_driver, const RID &p_vehicle);
            RID driver_get_vehicle(const RID &p_driver) const;
            RID vehicle_get_driver(const RID &p_vehicle) const;
            /// An order for the driver - a scenario's command with its two values, and where what
            /// sent it stands
            void driver_send_command(
                    const RID &p_driver, const String &p_command, double p_value1, double p_value2,
                    const Vector3 &p_position = Vector3());
    };
} // namespace godot
