#pragma once
#include "vehicles/base/VehicleComponentType.hpp"
#include "vehicles/base/VehicleController.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    class VehicleComponent;

    /* The vehicles, whatever they run on: their handles, the controller behind each, the names a
     * scenery gives them, their commands, components and dumps. What a kind of vehicle adds - a
     * rail vehicle's track, stepping and couplers - is a server of its own (RailVehicleServer),
     * which takes a vehicle created here and knows it by the same handle. */
    class VehicleServer : public Object {
            GDCLASS(VehicleServer, Object)

        public:
            static VehicleServer *get_instance() {
                return Object::cast_to<VehicleServer>(Engine::get_singleton()->get_singleton("VehicleServer"));
            }

        private:
            /* A controller this server holds (controller_create()): the copy of a description it
             * was configured with, and the vehicle it drives while bound */
            struct Controller {
                    Ref<VehicleController> controller;
                    RID vehicle;
            };

            struct Vehicle {
                    /* The controller driving this vehicle (vehicle_bind_controller()), RID() for none */
                    RID controller;
                    /* What the scenery placed the vehicle with, handed to every controller bound
                     * to it before its simulation starts (the `.scn` `dynamic` entry) */
                    double initial_velocity = 0.0;
                    VehicleController::DriverType driver_type = VehicleController::DRIVER_NOBODY;
                    /* The implementation that steps it, as its controller names it */
                    StringName implementation;
                    /* What a scenery calls this vehicle. Only the things that know a vehicle by
                     * name alone need it - an event, the console, the radio, a `.scn` command -
                     * and they reach the vehicle through vehicle_get_rid_by_name(). */
                    String name;
                    /* The last dump handed out, and the controller's state serial it was built
                     * at. A cab is dozens of widgets asking the same vehicle in one frame, and
                     * only a step or a command moves the values between them. */
                    Dictionary state_dump;
                    uint64_t state_dump_serial = 0;
                    bool state_dump_valid = false;
            };

            HashMap<RID, Vehicle> vehicles;
            HashMap<String, RID> vehicles_by_name;
            int64_t next_vehicle_id = 0;
            HashMap<RID, Controller> controllers;
            int64_t next_controller_id = 0;
            /* What simulates vehicles, by the name a controller gives (implementation_register()) */
            HashMap<StringName, ObjectID> implementations;
            /// Stepping holds SimulationServer's clock and steps as it advances
            bool stepping = false;
            bool stepping_enabled = true;
            /* Rebuilt every step, kept as a member so the step allocates no map per frame */
            HashMap<StringName, Vector<RID>> stepped_vehicles;

            void _refresh_stepping();
            void _on_simulation_advanced(double p_seconds);

            VehicleController *_get_controller(const RID &p_vehicle) const;
            /* The controller's own events, relayed under the handle, so whoever follows a vehicle
             * never holds its controller - connected when a controller is attached, disconnected
             * when it is replaced or the vehicle is freed */
            void _connect_relays(const RID &p_vehicle);
            void _disconnect_relays(const RID &p_vehicle);
            void _on_vehicle_moved(const Vector3 &p_position, const RID &p_vehicle);
            void _on_vehicle_command_received(
                    const String &p_command, const Variant &p_p1, const Variant &p_p2, const RID &p_vehicle);
            void _on_vehicle_configured(const RID &p_vehicle);
            void _on_vehicle_config_changed(const RID &p_vehicle);

        protected:
            static void _bind_methods();

        public:
            static const char *vehicle_moved_signal;
            static const char *vehicle_command_received_signal;
            static const char *vehicle_freed_signal;
            /* Another controller drives the vehicle now - whoever relays its events reconnects */
            static const char *vehicle_controller_changed_signal;
            /* The simulation behind the vehicle exists and carries its configuration */
            static const char *vehicle_configured_signal;
            /* A component of the vehicle (re)applied its configuration */
            static const char *vehicle_config_changed_signal;

            VehicleServer();
            ~VehicleServer() override;

            /* What simulates the vehicles whose controller names p_name, by the instance id of its
             * VehicleImplementationServer - the extension registers the Mover, an addon may
             * register its own */
            void implementation_register(const StringName &p_name, uint64_t p_implementation_id);
            void implementation_unregister(const StringName &p_name);
            PackedStringArray implementation_get_list() const;

            /* One step of every vehicle: each implementation is handed its own vehicles, at once.
             * Driven by SimulationServer's clock, and callable directly with an explicit delta
             * where the caller wants to decide when it happens. */
            void stepping_advance(double p_delta);
            /* Freezing the step while a scenery is torn down: the vehicles are freed one by one and
             * stepping a registry that is being emptied is work for nothing. */
            void stepping_set_enabled(bool p_enabled);
            bool stepping_is_enabled() const;

            RID vehicle_create();
            void vehicle_free(const RID &p_vehicle);
            bool vehicle_exists(const RID &p_vehicle) const;
            /* A controller of this server, empty until configured */
            RID controller_create();
            /* The controller becomes a copy of p_description (a VehicleController with its
             * components - its stored configuration); a vehicle it drives is restarted on it */
            void controller_configure(const RID &p_controller, const Ref<VehicleController> &p_description);
            void controller_free(const RID &p_controller);
            /* The vehicle is driven by p_controller from now on: the controller it had lets go of
             * its simulation, this one takes the scenery's values and starts its own. RID()
             * unbinds; binding the same pair again restarts the vehicle's simulation. */
            void vehicle_bind_controller(const RID &p_vehicle, const RID &p_controller);
            /* The controller object bound to the vehicle, 0 without one - C++ only and unbound,
             * for the servers that step and couple the vehicles */
            uint64_t vehicle_get_controller_instance_id(const RID &p_vehicle) const;
            /* The controller the vehicle runs on - the copy its simulation was built from, null
             * without one */
            Ref<VehicleController> vehicle_get_controller(const RID &p_vehicle) const;
            /* What the scenery placed the vehicle with - the velocity it starts with and who drives
             * it. Handed to its controller when it is bound. */
            void vehicle_set_initial_velocity(const RID &p_vehicle, double p_velocity);
            void vehicle_set_driver_type(const RID &p_vehicle, VehicleController::DriverType p_driver_type);
            /* The scenery's name for this vehicle, and the way back from one. A name is what a
             * `.scn`, an event or the console has; everything that holds the vehicle uses its
             * handle and never comes through here (TrackServer::track_get_rid_by_name() is the
             * same shape, for the same reason). */
            void vehicle_set_name(const RID &p_vehicle, const String &p_name);
            String vehicle_get_name(const RID &p_vehicle) const;
            RID vehicle_get_rid_by_name(const String &p_name) const;
            TypedArray<RID> vehicle_get_rids() const;
            /* Who is aboard - a vehicle with nobody fires no crew events (Owner->Mechanik, TrkFoll.cpp:125) */
            VehicleController::DriverType vehicle_get_driver_type(const RID &p_vehicle) const;
            /* Whether the simulation behind the vehicle exists yet - nothing can be read off a
             * vehicle before it does */
            bool vehicle_is_simulation_ready(const RID &p_vehicle) const;
            /* The cab the driver sits in: 1 the front one, -1 the rear one, 0 nobody */
            int vehicle_get_occupied_cab(const RID &p_vehicle) const;
            /* Width (x), height (y) and length (z) of the body [m], in the vehicle's own frame */
            Vector3 vehicle_get_dimensions(const RID &p_vehicle) const;

            /* A command to one vehicle, by handle; returns what its handler answered (#43), or
             * Variant() when the vehicle has no such command */
            Variant vehicle_send_command(
                    const RID &p_vehicle, const StringName &p_command, const Variant &p_p1 = Variant(),
                    const Variant &p_p2 = Variant());
            /* The same command to every vehicle that has it */
            void vehicle_broadcast_command(
                    const StringName &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            PackedStringArray vehicle_get_commands(const RID &p_vehicle) const;
            bool vehicle_has_command(const RID &p_vehicle, const StringName &p_command) const;

            /* The hot values, typed and by handle - which backend answers is not the caller's
             * business. Everything else is read from the component that owns it. */
            double vehicle_get_velocity(const RID &p_vehicle) const;
            double vehicle_get_speed(const RID &p_vehicle) const;
            /* The component of a kind, as a typed object - the shape
             * PhysicsServer3D::body_get_direct_state() has: a live view on the vehicle, valid
             * while the vehicle is. A per-frame reader takes it once and reads its properties. */
            Ref<VehicleComponent> vehicle_component_get(const RID &p_vehicle, VehicleComponentType::Type p_type) const;
            /* Scripted components carrying a tag of the modder's own choosing */
            TypedArray<VehicleComponent>
            vehicle_generic_component_find(const RID &p_vehicle, const StringName &p_tag) const;
            /* Everything this vehicle publishes, by name, in one Dictionary. Expensive on
             * purpose: a console, a test or a diagnostic dump asks for it, never a per-frame
             * reader - those take the component that owns the value and read its property. */
            Dictionary vehicle_dump_state(const RID &p_vehicle);
            Dictionary vehicle_dump_config(const RID &p_vehicle) const;
    };
} // namespace godot
