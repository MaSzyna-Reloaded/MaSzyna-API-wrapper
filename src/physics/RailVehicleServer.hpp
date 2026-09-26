#pragma once
#include "../core/VehicleComponentType.hpp"
#include "../core/VehicleController.hpp"

#include "../tracks/TrackManager.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    class VehicleComponent;
    class VehicleController;

    /* Where a rail vehicle is: which track it occupies, how far along it, which way round, and
     * which branch of a switch. Movement along the route, crossing endpoints, forcing switch
     * blades and the resulting world transform all live here.
     *
     * What the vehicle *does* - forces, integration, couplers - belongs to its VehicleController
     * and the simulation behind it. This server moves the vehicle and ties it to the tracks, the
     * traction wires and the other servers; it holds the vehicle's RID and steps its controller.
     *
     * Ported from addons/libmaszyna/servers/rail_vehicle_physics_server.gd. */
    class RailVehicleServer : public Object {
            GDCLASS(RailVehicleServer, Object)

        public:
            /* Original engine: primary physics update rate and the iteration limit per frame
             * (drivermode.cpp:186-206) */
            static constexpr double PHYSICS_STEP = 0.01;
            static constexpr int MAX_PHYSICS_ITERATIONS = 20;

            static RailVehicleServer *get_instance() {
                return Object::cast_to<RailVehicleServer>(Engine::get_singleton()->get_singleton("RailVehicleServer"));
            }

        private:
            /* Simulation time owed but not yet integrated - see step_frame(). Kept in seconds,
             * never dropped while it stays under the catch-up limit. */
            double owed_seconds = 0.0;
            /* How much owed time may pile up before the simulation stops spreading it and takes
             * it in one step instead. A setting, because the answer depends on the machine. */
            static constexpr const char *CATCH_UP_LIMIT_SETTING = "maszyna/physics/catch_up_limit";
            static constexpr double DEFAULT_CATCH_UP_LIMIT = 1.0;
            /// Read once - step_frame() runs every frame and must not look a setting up there.
            double catch_up_limit = DEFAULT_CATCH_UP_LIMIT;
            /* Distance the neighbour scan steps past a track endpoint to enter the next track */
            static constexpr double SCAN_ENDPOINT_EPSILON = 0.001;
            /* 10 m is about 140 km/h at 4 fps, plus a safety margin (DynObj.cpp:7160) */
            static constexpr double SCAN_RANGE_MINIMUM = 10.0;
            static constexpr double SCAN_RANGE_MARGIN = 40.0;
            /* A vehicle moved along the track by more or less than requested (m) */
            static constexpr double DIAGNOSTICS_MOVE_TOLERANCE = 0.001;
            /* Velocity change of a vehicle within one frame reported as a kick (m/s^2) */
            static constexpr double DIAGNOSTICS_MAX_ACCELERATION = 3.0;
            /* Movement below this is not worth walking the route for (m) */
            static constexpr double MOVEMENT_EPSILON = 0.0001;
            /* Below this speed (km/h) a vehicle stands - TTrackFollower::Move(), TrkFoll.cpp:104 */
            static constexpr double STANDING_SPEED = 0.01;

            /* What a vehicle last did on its track, as the track signals reported it */
            enum TrackHeading {
                HEADING_STANDING,
                HEADING_TO_START,
                HEADING_TO_END,
            };
            /* How far a Radio-Stop carries (m) - basic_region::RadioStop, scene.cpp:1271 */
            static constexpr double RADIO_STOP_RANGE = 2000.0;
            /* How far ahead and behind the transform samples the curve to find its heading (m) */
            static constexpr double HEADING_SAMPLE_DISTANCE = 0.1;
            /* Beyond this the track is straight as far as the running shape is concerned (m) */
            static constexpr double CURVE_RADIUS_LIMIT = 15000.0;

            /* Where one vehicle sits on the route. */
            struct VehiclePlacement {
                    RID track;
                    double track_offset = 0.0;
                    TrackManager::Direction track_direction = TrackManager::DIRECTION_NORMAL;
                    TrackManager::SwitchTrack switch_track = TrackManager::TRACK_COMMON;
                    /* Whether `track` is a switch - part of describing the occupied track, and a
                     * track never turns into one, so it is asked when the vehicle changes track
                     * rather than on every step. */
                    bool track_is_switch = false;
                    /* Which way the last movement went along `track`: towards its end (+1) or its
                     * start (-1) */
                    double travel_sign = 0.0;
                    /* The track and heading last reported by the track signals */
                    RID reported_track;
                    TrackHeading reported_heading = HEADING_STANDING;
                    /* Moved since the simulated vehicle's location was last updated */
                    bool moved = true;
                    /* The body's transform and whether it still describes the placement above. A
                     * parked vehicle is asked for it every frame by everything that draws it or
                     * listens from it, and composing it samples the track twice. */
                    Transform3D body_transform;
                    bool body_transform_valid = false;
                    /* Per end: the neighbour was already reported as none, so reporting it again
                     * says nothing */
                    bool neighbour_cleared[2] = {false, false};
                    /* The object driving this vehicle, held as an id rather than a pointer: an
                     * id says nothing about a lifetime this server does not own (AGENTS.md, and
                     * PhysicsServer3D::body_attach_object_instance_id is the same shape). The
                     * public method still takes the plain integer get_instance_id() returns,
                     * because that is what crosses into GDScript. */
                    ObjectID controller_id;
                    /* What a scenery calls this vehicle. Only the things that know a vehicle by
                     * name alone need it - an event, the console, the radio, a `.scn` command -
                     * and they reach the vehicle through vehicle_get_rid_by_name(). */
                    String name;
                    /* The RailVehicle3D that owns this handle, so the tick can hand it its new
                     * placement instead of letting it pull one a frame late */
                    uint64_t rail_vehicle_id = 0;
                    /* The last dump handed out, and the step it was built for. A cab is dozens of
                     * widgets asking the same vehicle in one frame, and a step is what normally
                     * moves the values between them. */
                    Dictionary state_dump;
                    uint64_t state_dump_step = 0;
                    /// ...and the vehicle's command count it was built after, because a command
                    /// changes the state inside a step (VehicleController::command_executed()).
                    uint64_t state_dump_command_serial = 0;
            };

            HashMap<RID, VehiclePlacement> vehicles;
            HashMap<String, RID> vehicles_by_name;
            int64_t next_vehicle_id = 0;
            bool diagnostics = false;
            HashMap<uint64_t, double> diagnostics_velocity;
            bool stepping = false;
            bool stepping_enabled = true;
            /// The node that drives step_frame(); freed when stepping stops.
            uint64_t stepper_id = 0;
            /* Bumped once per step; a dump older than this is stale. Comparing a
             * serial beats clearing every vehicle's dump each frame. */
            uint64_t step_serial = 1;
            /* Rebuilt every tick, kept as members so the step allocates nothing per frame */
            Vector<RID> stepped_vehicles;
            TypedArray<VehicleController> stepped_controllers;
            HashMap<RID, Vector<RID>> track_vehicles;

            VehicleController *_get_controller(const VehiclePlacement &p_placement) const;
            /* The controller's own events, relayed under the handle, so whoever follows a vehicle
             * never holds its controller - connected when a controller is attached, disconnected
             * when it is replaced or the vehicle is freed */
            void _connect_relays(const RID &p_vehicle, const VehiclePlacement &p_placement);
            void _disconnect_relays(const RID &p_vehicle, const VehiclePlacement &p_placement);
            void _on_vehicle_moved(const Vector3 &p_position, const RID &p_vehicle);
            void _on_vehicle_command_received(
                    const String &p_command, const Variant &p_p1, const Variant &p_p2, const RID &p_vehicle);
            void _on_vehicle_cabin_occupied_changed(int p_cab, const RID &p_vehicle);
            void _move_placement(VehiclePlacement &p_placement, double p_distance, bool p_force_switch_state);
            VehiclePlacement _sample_placement(const VehiclePlacement &p_placement, double p_distance);
            Transform3D _compose_body_transform(VehiclePlacement &p_placement, const RID &p_vehicle);
            Transform3D _placement_transform(const VehiclePlacement &p_placement) const;
            double _placement_roll(const VehiclePlacement &p_placement) const;
            bool _motion_connection(
                    const RID &p_track, int p_endpoint_index, bool p_force_switch_state, RID &p_track_out,
                    int &p_endpoint_out);
            void _check_movement(const VehiclePlacement &p_placement, const Vector3 &p_start, double p_moved) const;
            void _refresh_stepping();
            void _set_stepping(bool p_stepping);
            void _clear_neighbour(VehicleController *p_controller, VehiclePlacement &p_placement, int p_end);
            void _update_neighbours(const RID &p_vehicle, VehiclePlacement &p_placement);
            bool _find_vehicle(
                    const RID &p_vehicle, const VehiclePlacement &p_placement, int p_end, double p_scan_range,
                    RID &p_found_out, int &p_found_end_out, double &p_found_distance_out);
            void _check_velocity_jumps(double p_delta);

        protected:
            static void _bind_methods();

        public:
            static const char *vehicle_moved_signal;
            static const char *vehicle_command_received_signal;
            static const char *vehicle_occupied_cab_changed_signal;
            static const char *vehicle_freed_signal;
            /* The vehicle has started moving along the track towards its start, its end, or has
             * stopped on it - once per change, what a scenery's track events are fired by
             * (TTrackFollower::Move(), TrkFoll.cpp:113-161) */
            static const char *vehicle_heading_to_track_start_signal;
            static const char *vehicle_heading_to_track_end_signal;
            static const char *vehicle_stopped_on_track_signal;

            RailVehicleServer();
            ~RailVehicleServer() override;

            RID vehicle_create();
            void vehicle_free(const RID &p_vehicle);
            bool vehicle_exists(const RID &p_vehicle) const;
            /* The node driving this vehicle, by instance id - a public API carries no pointers
             * (PhysicsServer3D::body_attach_object_instance_id is the shape this follows). */
            void vehicle_attach_controller(const RID &p_vehicle, uint64_t p_controller_id);
            /* The scenery's name for this vehicle, and the way back from one. A name is what a
             * `.scn`, an event or the console has; everything that holds the vehicle uses its
             * handle and never comes through here (TrackManager::track_get_rid_by_name() is the
             * same shape, for the same reason). */
            void vehicle_set_name(const RID &p_vehicle, const String &p_name);
            String vehicle_get_name(const RID &p_vehicle) const;
            /* Who is aboard - a vehicle with nobody fires no crew events (Owner->Mechanik, TrkFoll.cpp:125) */
            VehicleController::DriverType vehicle_get_driver_type(const RID &p_vehicle) const;
            RID vehicle_get_rid_by_name(const String &p_name) const;
            /* Every vehicle the server holds, and those whose position falls in p_rect (x, z) */
            TypedArray<RID> get_vehicles() const;
            TypedArray<RID> get_vehicles_in_rect(const Rect2 &p_rect) const;
            /* A command to one vehicle, by handle; returns what its handler answered (#43), or
             * Variant() when the vehicle has no such command */
            Variant vehicle_send_command(
                    const RID &p_vehicle, const StringName &p_command, const Variant &p_p1 = Variant(),
                    const Variant &p_p2 = Variant());
            /* The same command to every vehicle that has it */
            void broadcast_command(
                    const StringName &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            PackedStringArray vehicle_get_commands(const RID &p_vehicle) const;
            /* The vehicles joined to this one by p_element, in order: from the last of them beyond
             * p_end back through this one to the last on the other side (TDynamicObject::
             * GetFirstDynamic() + Next(), DynObj.cpp:501) */
            TypedArray<RID>
            vehicle_get_coupled(const RID &p_vehicle, int p_end, VehicleController::CouplingElement p_element) const;
            /* Radio-Stop sent from this vehicle reaches every vehicle within RADIO_STOP_RANGE of it,
             * itself included (basic_region::RadioStop, scene.cpp:1269) */
            void vehicle_radio_stop(const RID &p_vehicle);
            /* The RailVehicle3D this handle belongs to, by instance id. */
            void vehicle_attach_rail_vehicle(const RID &p_vehicle, uint64_t p_rail_vehicle_id);
            uint64_t vehicle_get_rail_vehicle(const RID &p_vehicle) const;

            /* Freezing the step while a scenery is torn down: the vehicles are freed one by one and
             * stepping a registry that is being emptied is work for nothing. Replaces toggling the
             * old autoload's process_mode. */
            void set_stepping_enabled(bool p_enabled);
            bool is_stepping_enabled() const;

            void vehicle_set_track(
                    const RID &p_vehicle, const RID &p_track, double p_track_offset,
                    TrackManager::Direction p_track_direction);
            void vehicle_move(const RID &p_vehicle, double p_distance);
            /* Walks one vehicle the distance its own simulation asked for. The step does this for
             * every vehicle; on its own it is how a single vehicle is advanced deliberately. */
            void vehicle_process_movement(const RID &p_vehicle, double p_delta);

            /* One whole step of every registered vehicle. Driven by `process_frame`, and callable
             * directly with an explicit delta where the caller wants to decide when it happens. */
            void step(double p_delta);
            /* One frame's worth of simulation, called by RailVehicleStepper before any node has
             * been processed - see that class for why the timing matters. */
            void step_frame(double p_delta);
            Transform3D vehicle_get_transform(const RID &p_vehicle);
            Transform3D vehicle_get_transform_at_distance(const RID &p_vehicle, double p_distance);
            /* Track under the vehicle and its centre along that track, measured towards its front */
            Dictionary vehicle_get_track_position(const RID &p_vehicle) const;
            /* Running shape of the bogies (DynObj.cpp:2950-2970): the curve radius from the yaw
             * difference of the bogie pivots, and the mean cant of both bogies in radians. Samples
             * the track twice - call it only when the radius is needed. */
            Dictionary vehicle_get_curve(const RID &p_vehicle, double p_bogie_pivot_spacing);

            /* The hot values, typed and by handle - which backend answers is not the caller's
             * business. Everything else is read from the component that owns it. */
            double vehicle_get_velocity(const RID &p_vehicle) const;
            double vehicle_get_speed(const RID &p_vehicle) const;
            /* Everything this vehicle publishes, by name, in one Dictionary. Expensive on
             * purpose: a console, a test or a diagnostic dump asks for it, never a per-frame
             * reader - those take the component that owns the value and read its property. */
            /* The component of a kind, as a typed object - the shape
             * PhysicsServer3D::body_get_direct_state() has: a live view on the vehicle, valid
             * while the vehicle is. A per-frame reader takes it once and reads its properties. */
            VehicleComponent *vehicle_component_get(const RID &p_vehicle, VehicleComponentType::Type p_type) const;
            /* Scripted components carrying a tag of the modder's own choosing */
            TypedArray<VehicleComponent>
            generic_vehicle_component_find(const RID &p_vehicle, const StringName &p_tag) const;

            Dictionary vehicle_dump_state(const RID &p_vehicle);
            Dictionary vehicle_dump_config(const RID &p_vehicle) const;
    };
} // namespace godot
