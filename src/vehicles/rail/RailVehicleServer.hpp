#pragma once
#include "vehicles/base/VehicleComponentType.hpp"
#include "vehicles/rail/RailVehicleController.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"
#include "vehicles/rail/RailVehicleRadio.hpp"

#include "RailVehicleNeighbour.hpp"
#include "tracks/TrackServer.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
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
     * and the simulation behind it. The vehicle itself - its handle, name, commands and
     * components - is VehicleServer's; this server takes a vehicle created there
     * (vehicle_attach()), moves it and ties it to the tracks, the traction wires and the other
     * servers. The step itself is the implementation's (MaszynaMoverVehicleServer), which calls
     * the per-vehicle rail operations below in its phases.
     *
     * Ported from addons/libmaszyna/servers/rail_vehicle_physics_server.gd. */
    class RailVehicleServer : public Object {
            GDCLASS(RailVehicleServer, Object)

        public:
            static RailVehicleServer *get_instance() {
                return Object::cast_to<RailVehicleServer>(Engine::get_singleton()->get_singleton("RailVehicleServer"));
            }

        private:
            /* Distance the neighbour scan steps past a track endpoint to enter the next track */
            static constexpr double SCAN_ENDPOINT_EPSILON = 0.001;
            /* 10 m is about 140 km/h at 4 fps, plus a safety margin (DynObj.cpp:7160) */
            static constexpr double SCAN_RANGE_MINIMUM = 10.0;
            static constexpr double SCAN_RANGE_MARGIN = 40.0;
            /* A vehicle moved along the track by more or less than requested (m) */
            static constexpr double DIAGNOSTICS_MOVE_TOLERANCE = 0.001;
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

            /* How far outside the slider the guide horn still catches a wire (DynObj.cpp:81,
             * fWidthExtra). Without it a pantograph drops the wire wherever it swings sideways -
             * at a span junction, over a switch, or on the zigzag - and the vehicle reads a real
             * loss of voltage where the original keeps contact. */
            static constexpr double PANTOGRAPH_HORN_WIDTH = 0.381;
            /* The slider's height over the upper arm's end (TAnimPant::fHeight, DynObj.cpp:97) */
            static constexpr double PANTOGRAPH_SLIDER_HEIGHT = 0.07;
            /* Tank pressure the arm rises from [bar], an EMU's lower (DynObj.cpp:3863-3866) */
            static constexpr double PANTOGRAPH_RAISING_PRESSURE = 3.45;
            static constexpr double PANTOGRAPH_EMU_RAISING_PRESSURE = 2.45;
            /* DynObj.cpp:3869-3905: the rise per bar and second, the share of the gap to the wire
             * closed in one step up and down, how fast a lowered arm falls [rad/s]; and the gap
             * under which the slider touches the wire (PantDiff < 0.01, DynObj.cpp:3784) [m] */
            static constexpr double PANTOGRAPH_RAISE_RATE = 0.015;
            static constexpr double PANTOGRAPH_RISE_SHARE = 0.55;
            static constexpr double PANTOGRAPH_PRESS_SHARE = 0.4;
            static constexpr double PANTOGRAPH_FALL_RATE = 0.15;
            static constexpr double PANTOGRAPH_SETTLED_GAP = 0.001;
            static constexpr double PANTOGRAPH_CONTACT_GAP = 0.01;
            /* A loss of the wire's voltage no longer than this [s] keeps the last one - the arm
             * catching up with the wire, an insulator at speed - so the line breaker does not trip
             * on it (DynObj.cpp:3132-3140, NoVoltTime) */
            static constexpr double NO_VOLTAGE_HOLD = 0.2;

            /* One pantograph of a vehicle (TAnimPant, DynObj.h:106): where it stands and how its
             * arms are built - the model's, measured by whoever draws the vehicle
             * (vehicle_set_pantograph_geometry()) - and how far it is raised, which is the
             * vehicle's own state and outlives any model rebuilt to draw it. */
            struct Pantograph {
                    bool present = false;
                    /* in the vehicle's own space (TAnimPant::vPos) */
                    Vector3 position;
                    double lower_length = 0.0;
                    double upper_length = 0.0;
                    double horizontal = 0.0;
                    double lower_rest_angle = 0.0;
                    double upper_rest_angle = 0.0;
                    /* fAngleL, fAngleU, PantWys */
                    double lower_angle = 0.0;
                    double upper_angle = 0.0;
                    double height = 0.0;
                    /* the slider reaches the wire (PantDiff < 0.01) */
                    bool reaches_wire = true;
                    /* the span it is on, followed along the chain (DynObj.cpp:8742-8770), and what
                     * was last reported of it */
                    RID wire;
                    bool touching = false;
                    bool powered = false;

                    /* One step of the arms, p_gap [m] below the wire: towards it at p_speed_factor
                     * while raised, down while lowered (DynObj.cpp:3877-3920) */
                    void raise(double p_gap, bool p_active, double p_speed_factor, double p_delta);
            };

            /* Where one vehicle sits on the route. */
            struct VehiclePlacement {
                    /* The controller this server steps and relays the rail events of - the one
                     * VehicleServer drives the vehicle with, taken again when that changes */
                    ObjectID controller_id;
                    /* What the scenery placed the rail vehicle with - its type and its load -
                     * handed to every controller bound to it before its simulation starts */
                    String type_name;
                    String load_name;
                    double load_amount = 0.0;
                    RID track;
                    double track_offset = 0.0;
                    TrackServer::Direction track_direction = TrackServer::DIRECTION_NORMAL;
                    TrackServer::SwitchTrack switch_track = TrackServer::TRACK_COMMON;
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
                    /* Moved since its position was last announced (once a frame), and since the
                     * simulated vehicle's location was last set (every sub-step) */
                    bool moved = true;
                    bool location_stale = true;
                    /* Moved since vehicle_placement_changed last said so */
                    bool placement_unreported = true;
                    /* The body's transform and whether it still describes the placement above. A
                     * parked vehicle is asked for it every frame by everything that draws it or
                     * listens from it, and composing it samples the track twice. */
                    Transform3D body_transform;
                    bool body_transform_valid = false;
                    /* Per end: the neighbour was already reported as none, so reporting it again
                     * says nothing */
                    bool neighbour_cleared[2] = {false, false};
                    /* PANTOGRAPH_FIRST, PANTOGRAPH_SECOND */
                    Pantograph pantographs[2];
                    /* The slider's width the model gives, for a vehicle whose FIZ declares none */
                    double pantograph_collector_width = 0.0;
                    /* How long the pantographs have fed no voltage [s] (NoVoltTime) */
                    double no_voltage_time = 0.0;
            };

            HashMap<RID, VehiclePlacement> vehicles;

            /* A scenery's trainset (deserialize_trainset(), simulationstateserializer.cpp): its
             * vehicles in order, standing one after another from its front on its track */
            struct TrainsetMember {
                    RID vehicle;
                    TrackServer::Direction direction = TrackServer::DIRECTION_NORMAL;
                    /* How far it stands behind the one before it [m] - the `offset` of its `dynamic` */
                    double gap = 0.0;
                    /* The coupling with the next vehicle - the `couplingdata` of its `dynamic` */
                    BitField<RailVehicleController::CouplingFlags> coupling = 0;
            };
            struct Trainset {
                    RID track;
                    double offset = 0.0;
                    Vector<TrainsetMember> members;
                    /* Asked to stand while a vehicle of it had no simulation yet: it stands once it has */
                    bool placement_pending = false;
            };
            HashMap<RID, Trainset> trainsets;
            void _on_vehicle_configured(const RID &p_vehicle);
            bool diagnostics = false;
            /* The rail vehicles on each track, rebuilt once a step (neighbour_index_rebuild()),
             * kept as a member so the step allocates nothing per frame */
            HashMap<RID, Vector<RID>> track_vehicles;

            RailVehicleController *_get_controller(const VehiclePlacement &p_placement) const;
            /* The controller's rail events, relayed under the handle, so whoever follows a vehicle
             * never holds its controller - connected when the vehicle is attached or VehicleServer
             * gives it another controller, disconnected when it is replaced or detached */
            void _connect_relays(const RID &p_vehicle, const VehiclePlacement &p_placement);
            void _disconnect_relays(const RID &p_vehicle, const VehiclePlacement &p_placement);
            void _on_vehicle_controller_changed(const RID &p_vehicle);
            void _on_vehicle_cabin_occupied_changed(int p_cab, const RID &p_vehicle);
            void _on_vehicle_trainset_changed(const RID &p_vehicle);
            void _on_vehicle_coupler_attached(int64_t p_flag, const RID &p_vehicle);
            void _on_vehicle_coupler_detached(int64_t p_flag, const RID &p_vehicle);
            void _move_placement(VehiclePlacement &p_placement, double p_distance, bool p_force_switch_state);
            VehiclePlacement _sample_placement(const VehiclePlacement &p_placement, double p_distance);
            Transform3D _compose_body_transform(VehiclePlacement &p_placement, const RID &p_vehicle);
            Transform3D _placement_transform(const VehiclePlacement &p_placement) const;
            double _placement_roll(const VehiclePlacement &p_placement) const;
            bool _motion_connection(
                    const RID &p_track, int p_endpoint_index, bool p_force_switch_state, RID &p_track_out,
                    int &p_endpoint_out);
            void _check_movement(const VehiclePlacement &p_placement, const Vector3 &p_start, double p_moved) const;
            void _clear_neighbour(
                    RailVehicleController *p_controller, VehiclePlacement &p_placement,
                    RailVehicleController::CouplerEnd p_end);
            void _update_neighbours(const RID &p_vehicle, VehiclePlacement &p_placement);
            bool _find_vehicle(
                    const RID &p_vehicle, const VehiclePlacement &p_placement, RailVehicleController::CouplerEnd p_end,
                    double p_scan_range, RID &p_found_out, RailVehicleController::CouplerEnd &p_found_end_out,
                    double &p_found_distance_out);
            /* Where the vehicle is, in the terms a scenery is written in - for the pantographs'
             * warnings, which a world position alone does not tie to the .scn */
            String _track_position_text(const RID &p_vehicle) const;
            /* The span over the pantograph's slider: the one it was on followed along the chain,
             * else searched for; {"rid", "height"} as TractionServer answers */
            Dictionary _find_pantograph_wire(
                    const RID &p_vehicle, Pantograph &p_pantograph, int p_index, const Transform3D &p_frame,
                    double p_half_width);

        protected:
            static void _bind_methods();

        public:
            /* The vehicle stands on a track now - it was put there (vehicle_set_track()) */
            static const char *vehicle_placed_signal;
            /* The vehicle is somewhere else on the route - once a step it moved in, or once a
             * deliberate move (vehicle_move()); what draws it follows */
            static const char *vehicle_placement_changed_signal;
            static const char *vehicle_occupied_cab_changed_signal;
            /* The vehicles coupled to this one are others now (RailVehicleController::trainset_changed) */
            static const char *vehicle_trainset_changed_signal;
            static const char *vehicle_coupler_attached_signal;
            static const char *vehicle_coupler_detached_signal;
            /* The vehicle has started moving along the track towards its start, its end, or has
             * stopped on it - once per change, what a scenery's track events are fired by
             * (TTrackFollower::Move(), TrkFoll.cpp:113-161) */
            static const char *vehicle_heading_to_track_start_signal;
            static const char *vehicle_heading_to_track_end_signal;
            static const char *vehicle_stopped_on_track_signal;
            static const char *vehicle_radio_called_signal;
            /* A Radio-Stop reached the vehicle and braked it (TDynamicObject::RadioStop,
             * DynObj.cpp:7229) */
            static const char *vehicle_emergency_signal_received_signal;

            RailVehicleServer();

            /* A VehicleServer vehicle becomes a rail vehicle: it gets a place on the route and is
             * stepped - called by whatever makes it one (RailVehicle3D, a FIZ vehicle). A vehicle
             * freed by VehicleServer is detached on its own. */
            void vehicle_attach(const RID &p_vehicle);
            void vehicle_detach(const RID &p_vehicle);
            bool vehicle_is_attached(const RID &p_vehicle) const;
            /* The rail vehicle's type - the original's CHK/MMD name, TMoverParameters::TypeName
             * (DynObj.cpp:2019) - and what it carries when the scenery places it (`loadtype`,
             * `loadcount` of a `dynamic`). Handed to its controller when it is bound. */
            void vehicle_set_type_name(const RID &p_vehicle, const String &p_type_name);
            String vehicle_get_type_name(const RID &p_vehicle) const;
            void vehicle_set_load(const RID &p_vehicle, const String &p_load_name, double p_load_amount);
            /* Wakes the vehicle's simulation, switched off while it stood with nothing to do -
             * somebody took it (DriverSystem) */
            void vehicle_wake(const RID &p_vehicle);
            /* The rail vehicles whose position falls in p_rect (x, z) */
            TypedArray<RID> vehicle_get_rids_in_rect(const Rect2 &p_rect) const;
            /* The railway component of a kind, as VehicleServer::vehicle_component_get() answers
             * the kinds every vehicle has */
            Ref<VehicleComponent>
            vehicle_component_get(const RID &p_vehicle, RailVehicleComponentType::Type p_type) const;
            /* Couples p_end of the vehicle to p_other_end of p_other by p_coupling
             * (TMoverParameters::Attach(), Mover.cpp:548) */
            void vehicle_couple(
                    const RID &p_vehicle, RailVehicleController::CouplerEnd p_end, const RID &p_other,
                    RailVehicleController::CouplerEnd p_other_end,
                    BitField<RailVehicleController::CouplingFlags> p_coupling);
            /* The vehicles joined to this one by every one of p_flags, in order: from the last of them
             * beyond p_end back through this one to the last on the other side (TDynamicObject::
             * GetFirstDynamic() + Next(), DynObj.cpp:501) */
            TypedArray<RID> vehicle_get_coupled(
                    const RID &p_vehicle, RailVehicleController::CouplerEnd p_end,
                    BitField<RailVehicleController::CouplingFlags> p_flags) const;
            /* The vehicle a cab's controls drive (TDynamicObject::FindPowered(), DynObj.cpp:7772): this
             * one if it has power, else the nearest with power joined to it - within an EMU's or DMU's
             * unit, else by the control line; this one when there is none */
            RID vehicle_find_powered(const RID &p_vehicle) const;
            /* The vehicle whose pantographs the cab raises (FindPantographCarrier(), DynObj.cpp:7798):
             * one fed by a current collector, of the unit first, then of the vehicles under control;
             * an invalid RID when there is none */
            RID vehicle_find_pantograph_carrier(const RID &p_vehicle) const;
            /* Radio-Stop sent from this vehicle reaches every vehicle within RADIO_STOP_RANGE of it,
             * itself included (basic_region::RadioStop, scene.cpp:1269) */
            void vehicle_emergency_signal_send(const RID &p_vehicle);
            /* A Radio-Stop sent from a place rather than a vehicle - a scenery's `Emergency_brake` -
             * heard by every vehicle within RADIO_STOP_RANGE */
            void emergency_signal_send(const Vector3 &p_position);
            /* The vehicle's radio sent a call from where it stands (Event.cpp:2255-2268 listens) */
            void vehicle_radio_call(const RID &p_vehicle, RailVehicleRadio::RadioCall p_call);


            void vehicle_set_track(
                    const RID &p_vehicle, const RID &p_track, double p_track_offset,
                    TrackServer::Direction p_track_direction);
            void vehicle_move(const RID &p_vehicle, double p_distance);
            /* A scenery's trainset: vehicles standing one after another on a track, coupled - the
             * handle of what a `trainset:` block places (TrainSet3D) */
            RID trainset_create();
            void trainset_free(const RID &p_trainset);
            /* Where the trainset's front stands - the track and the distance along it [m] */
            void trainset_set_track(const RID &p_trainset, const RID &p_track, double p_offset);
            void trainset_clear(const RID &p_trainset);
            /* The next vehicle of the trainset: which way it stands, how far behind the one before
             * it [m] (none when it stands reversed) and how it couples to the next one */
            void trainset_add_vehicle(
                    const RID &p_trainset, const RID &p_vehicle, TrackServer::Direction p_direction, double p_gap,
                    BitField<RailVehicleController::CouplingFlags> p_coupling);
            TypedArray<RID> trainset_get_vehicles(const RID &p_trainset) const;
            /* Stands the vehicles on the track one after another, each as long as it is, and couples
             * them (deserialize_dynamic(), simulationstateserializer.cpp:1062-1076, and endtrainset,
             * simulationstateserializer.cpp:818-837) - at once, or as soon as every vehicle of it
             * has its simulation */
            void trainset_place(const RID &p_trainset);
            /* The whole trainset coupled to the vehicle moved p_distance [m] towards the vehicle's
             * front, every vehicle whichever way round it stands (TDynamicObject::move_set(),
             * DynObj.cpp:4302) */
            void trainset_move(const RID &p_vehicle, double p_distance);
            /* Walks one vehicle the distance its own simulation asked for. The step does this for
             * every vehicle; on its own it is how a single vehicle is advanced deliberately. */
            void vehicle_process_movement(const RID &p_vehicle, double p_delta);

            /* What the rail does for a vehicle within a step - C++ only, called by the
             * implementation that steps its vehicles in its own phases (MaszynaMoverVehicleServer). */
            /* Which vehicle stands on which track, for this step's neighbour scans: once a step,
             * before any vehicle looks for its neighbours */
            void neighbour_index_rebuild();
            /* The vehicle moved since its position was last announced: announce it */
            void vehicle_report_position(const RID &p_vehicle);
            /* Hands the simulated vehicle its location on the route, where it has moved */
            void vehicle_update_location(const RID &p_vehicle);
            /* The nearest vehicle beyond each free end, the coupled one at a coupled end
             * (TDynamicObject::update_neighbours(), DynObj.cpp:7544) */
            void vehicle_update_neighbours(const RID &p_vehicle);
            /* The pantographs at the wire the vehicle now stands under, and the voltage they
             * feed it (DynObj.cpp:3714-3920) - before its circuits run on what they collect */
            void vehicle_collect_current(const RID &p_vehicle, double p_delta);
            /* The vehicle's heading on its track, reported to the track events on a change
             * (TTrackFollower::Move(), TrkFoll.cpp:113-161) */
            void vehicle_report_track_heading(const RID &p_vehicle);
            /* The vehicle moved since its placement was last announced: announce it, so nothing
             * draws it a frame late */
            void vehicle_report_placement(const RID &p_vehicle);
            Transform3D vehicle_get_transform(const RID &p_vehicle);
            /* A pantograph as the model builds it: where its lower arm stands in the vehicle's own
             * space, the arms' lengths, the horizontal offset between their ends and their angles
             * lowered; with the slider's width the model gives. Measured by whoever draws the
             * vehicle; a model rebuilt hands it again and the raise is kept. */
            void vehicle_set_pantograph_geometry(
                    const RID &p_vehicle, RailVehicleElectricEngine::PantographSelector p_pantograph,
                    const Vector3 &p_position, double p_lower_length, double p_upper_length, double p_horizontal,
                    double p_lower_rest_angle, double p_upper_rest_angle, double p_collector_width);
            /* Where the pantograph stands in the vehicle's own space; zero for one the model lacks */
            Vector3 vehicle_get_pantograph_position(
                    const RID &p_vehicle, RailVehicleElectricEngine::PantographSelector p_pantograph) const;
            /* How far the lower (x) and upper (y) arm are raised over lowered [rad] - what is drawn */
            Vector2 vehicle_get_pantograph_raise(
                    const RID &p_vehicle, RailVehicleElectricEngine::PantographSelector p_pantograph) const;
            Transform3D vehicle_get_transform_at_distance(const RID &p_vehicle, double p_distance);
            /* Track under the vehicle and its centre along that track, measured towards its front */
            Dictionary vehicle_get_track_position(const RID &p_vehicle) const;
            /* The tracks ahead of the vehicle the way p_direction leads - +1 towards its front, as
             * the mover's V > 0 moves it, -1 towards its rear - as far as p_distance [m], the one it
             * stands on first */
            TypedArray<TrackRouteSegment> vehicle_trace_route(const RID &p_vehicle, int p_direction, double p_distance);
            /* The nearest vehicle along the route from the vehicle's p_end (0 front, 1 rear), on the
             * tracks entered within p_distance [m] of its centre, null when there is none
             * (TDynamicObject::find_vehicle(), DynObj.cpp:7688) */
            Ref<RailVehicleNeighbour>
            vehicle_find_vehicle(const RID &p_vehicle, RailVehicleController::CouplerEnd p_end, double p_distance);
            /* Running shape of the bogies (DynObj.cpp:2950-2970): the curve radius from the yaw
             * difference of the bogie pivots, and the mean cant of both bogies in radians. Samples
             * the track twice - call it only when the radius is needed. */
            Dictionary vehicle_get_curve(const RID &p_vehicle, double p_bogie_pivot_spacing);
    };
} // namespace godot
