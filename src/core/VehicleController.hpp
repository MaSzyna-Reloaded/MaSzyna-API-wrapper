#pragma once
#include "VehicleComponentType.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>


namespace godot {
    class VehicleBrake;
    class VehicleComponent;
    class VehicleEngine;
    class VehicleSecuritySystem;
    class VehicleLighting;


    /// The vehicle itself: its configuration, its components and the operations that change them,
    /// with no statement about what simulates it - that is the implementation's business. It is
    /// not a node - VehiclePhysicsNode is the vehicle's presence in the tree, and it owns one of
    /// these. Reached from outside by RID, through RailVehicleServer.
    class VehicleController : public Object {
            GDCLASS(VehicleController, Object)
        public:
            /* Who drives the vehicle, in the words the `.scn` uses for it - a `dynamic` names
             * `headdriver`, `reardriver` or `nobody` as its drivertype (DynObj.cpp:1812-1825). It
             * says which cab is manned, not how many cabs there are, and a vehicle nobody drives
             * is not simulated at all (Driver.cpp:2126). */
            enum DriverType {
                DRIVER_NOBODY,
                DRIVER_HEAD,
                DRIVER_REAR,
            };


        private:
            DriverType driver_type = DRIVER_NOBODY;
            /// state is rebuilt from the backend when it is asked for, not on every physics step:
            /// a scenery runs hundreds of vehicles and almost none of them is ever read
            bool prev_roof_light_enabled = false;
            /// Bumped by command_executed(); what tells a cached state dump that it is stale.
            uint64_t command_serial = 0;
            /// What this vehicle answers to, registered by itself and its components. A vehicle
            /// holds its own commands, so a scenery name shared by two vehicles, or none at all,
            /// leaves every one of them commandable.
            HashMap<StringName, Callable> commands;

        protected:
            /// Writes the wrapper's configuration - the vehicle's and every component's - to the
            /// backend, then announces that the backend carries it.
            void apply_configuration();
            /* Creates the simulation behind the vehicle and writes its configuration there. */
            virtual void _initialize_simulation() = 0;
            /* A component joined or left the vehicle - the implementation hooks it up to itself. */
            virtual void _component_attached(VehicleComponent *p_component) {}
            virtual void _component_detached(VehicleComponent *p_component) {}
            virtual void _fill_config_dictionary(Dictionary &p_config) const = 0;
            /* The vehicle's own share of the dump - what every vehicle has, whatever it is
             * made of. Its components add theirs. */
            virtual void _fill_state_dictionary(Dictionary &p_state) const;
            /* The vehicle's own commands, registered when it joins the system and given back on
             * shutdown - a kind of vehicle adds its own. */
            virtual void _register_commands() {}
            virtual void _unregister_commands() {}

        public:
        public:
            enum Category {
                CATEGORY_TRAIN = 1,
                CATEGORY_ROAD = 2,
                CATEGORY_SHIP = 4,
                CATEGORY_AIRPLANE = 8,
            };


            /// The simulation now carries the configuration (the vehicle's and every component's)
            static const char *simulation_configured_signal;
            /// The simulation behind the vehicle exists and is configured
            static const char *simulation_initialized_signal;
            static const char *command_received;
            static const char *roof_light_changed;
            static const char *config_changed;
            static const char *position_changed_signal;

            Dictionary get_config() const;
            /* One of this vehicle's components (re)applied its configuration. */
            void emit_config_changed();
            void _notification(int p_what);
            Variant
            send_command(const StringName &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            PackedStringArray get_commands() const;
            /* A command has run against this vehicle. Its state has moved on in the middle of a
             * step, which is the one thing a dump cached for that step cannot see by itself -
             * hence the serial below (RailVehicleServer::vehicle_dump_state). */
            void
            command_executed(const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            uint64_t get_command_serial() const;
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void register_command(const StringName &p_command, const Callable &p_callable);
            void unregister_command(const StringName &p_command);
            /* One tick of everything the vehicle is made of, after its physics has moved. The
             * components have no _process of their own to be driven by a scene tree. */
            /* Brings the vehicle up: its simulation, its state and the signals whose initial
             * value listeners expect. Called by whatever owns the vehicle, once it is built -
             * it used to wait for NOTIFICATION_READY, which a vehicle outside a tree never gets. */
            /// Whether this vehicle's simulation exists yet. A vehicle is a vehicle from the
            /// moment it is built, but nothing can be coupled to it or read off it until the
            /// backend behind it is there.
            virtual bool is_simulation_ready() const = 0;
            void attach_to_system();
            /// Lets go of everything this vehicle holds - its components, its registration and
            /// its simulation - without destroying the vehicle, so every reference to it stays
            /// valid across a rebuild.
            virtual void release();
            virtual void initialize();
            /* The reverse: the vehicle gives its commands back and lets go of its handle. */
            void shutdown();
            void process_components(double p_delta);
            virtual void update_state();
            /// Straight from the backend, for the per-frame readers that only want this one number
            /// and would otherwise force the whole state dictionary to be rebuilt
            virtual double get_velocity() const = 0;
            /// Straight from the backend, like get_velocity() - the speed readers want this
            /// one number, not the whole state
            virtual double get_speed() const = 0;
            /// The acceleration along the track [m/s2], every force counted (AccS)
            virtual double get_acceleration() const = 0;
            /// The rest of what every vehicle has, whatever it is made of. Read straight from the
            /// backend - nothing is stored, and the dump is built from these.
            virtual double get_mass_total() const = 0;
            virtual double get_total_distance() const = 0;
            virtual int get_direction() const = 0;
            virtual void apply_config() = 0;
            virtual bool is_physics_active() const = 0;
            void set_driver_type(DriverType p_value);
            DriverType get_driver_type() const;
            /* The cab the driver_type sits in, as the simulation counts it: 1 for the front cab, -1
             * for the rear one, 0 for nobody. */
            int get_occupied_cab() const;
            static void _bind_methods();
            /* This vehicle's handle in RailVehicleServer, set when the server attaches it. */
            void set_vehicle_rid(const RID &p_vehicle_rid);
            RID get_rid() const;
            void emit_position_changed_if_needed();
            Vector3 get_world_position() const;
            Transform3D get_world_transform() const;
            MAKE_MEMBER_GS(String, train_id, "");
            /* What the vehicle carries when the scenery places it, as the `.scn` names it - the
             * amount and the cargo's own name (`loadcount` and `loadtype` of a `dynamic`). The
             * simulation takes both at once, and it reads more than cargo out of them: `pantstate`
             * is how a scenery starts a locomotive with its pantographs already up. */
            MAKE_MEMBER_GS(String, load_name, "");
            MAKE_MEMBER_GS(double, load_amount, 0.0);
            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS(double, mass, 0.0);
            MAKE_MEMBER_GS(double, power, 0.0);
            MAKE_MEMBER_GS(double, max_velocity, 0.0);
            MAKE_MEMBER_GS_NR(Category, category, CATEGORY_TRAIN);
            MAKE_MEMBER_GS(double, dimensions_length, 0.0);
            MAKE_MEMBER_GS(double, dimensions_height, 0.0);
            MAKE_MEMBER_GS(double, dimensions_width, 0.0);
            MAKE_MEMBER_GS(double, dimensions_drag_coefficient, 0.0);
            MAKE_MEMBER_GS(double, dimensions_floor_height, 0.96);

            // Mirrors the original engine's scenery-line velocity token (TDynamicObject::Init's
            // `driveractive = (fVel != 0.0)`): a vehicle with initial_velocity == 0.0 is "not
            // ready to depart" (ReadyFlag=false, battery stays off until switched on manually);
            // any non-zero value (scenario authors commonly use 0.1 for a stationary-but-ready
            // vehicle) marks it ready, so CheckLocomotiveParameters() turns the battery on per
            // cntrl_battery_start_mode.
            MAKE_MEMBER_GS(double, initial_velocity, 0.0);

            Dictionary get_state();

            /* This vehicle's components, in the order they joined - which is the order of the
             * FIZ sections that built them. They announce themselves rather than being searched
             * for in the subtree. */
            /* The component of a kind, or null when this vehicle has none. One per kind: a
             * vehicle has one brake system and one engine, whatever kind it is. */
            VehicleComponent *get_component(VehicleComponentType::Type p_type) const;
            /* Every scripted component carrying this tag - modders add as many as they like */
            TypedArray<VehicleComponent> find_generic_components(const StringName &p_tag) const;

            /* Takes a component into the vehicle and owns it from then on - it is ticked with
             * the vehicle and freed with it. The shape Node::add_child() has, for the same
             * reason: the thing being handed over has no life of its own outside its owner. */
            void add_component(VehicleComponent *p_component);

            void register_component(VehicleComponent *p_component);
            void unregister_component(VehicleComponent *p_component);

        private:
            Vector<VehicleComponent *> components;
            void free_components();
            /* The lighting component, kept because the vehicle raises roof_light_changed for it.
             * Resolved when the component joins, not searched for per frame. */
            VehicleLighting *lighting = nullptr;
            RID rid;
            Vector3 last_emitted_position = Vector3(1e10, 1e10, 1e10);
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleController::DriverType);
VARIANT_ENUM_CAST(VehicleController::Category);
