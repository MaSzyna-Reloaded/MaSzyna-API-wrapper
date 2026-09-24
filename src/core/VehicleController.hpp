#pragma once
#include "VehicleComponentType.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>


namespace godot {
    class VehicleBrake;
    class VehicleComponent;
    class VehicleEngine;
    class VehicleSecuritySystem;
    class VehicleLighting;
    class TrainSystem;


    /// The vehicle itself: its configuration, its components and the operations that change them,
    /// with no statement about what simulates it - that is the implementation's business
    /// (MoverVehicleController). It is not a node - VehiclePhysicsNode is the vehicle's presence in
    /// the tree, and it owns one of these. Reached from outside by RID, through RailVehicleServer.
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
            // original engine defaults this to 1, not 0 (vehicle/Driver.h: "int iRadioChannel =
            // 1") - 0 is never a valid channel (radio_channel_min defaults to 1 too), so starting
            // at 0 meant the very first radio_channel_increase call was invisible: CabinSwitch's
            // own switch_min_position clamp had already displayed the invalid 0 as channel 1
            // before any command ran, so the real 0->1 transition produced no visible change.
            int radio_channel = 1;

            bool prev_is_powered = false;
            bool prev_radio_enabled = false;
            int prev_radio_channel = radio_channel;
            bool prev_roof_light_enabled = false;
            /// Bumped by command_executed(); what tells a cached state dump that it is stale.
            uint64_t command_serial = 0;
            int prev_cabin_occupied = 0;

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
            void _fill_state_dictionary(Dictionary &p_state) const;

            /* Live state, read straight from the backend - nothing is stored. */
            /* The battery as it actually is, which drains and recharges. The authored
             * `battery_voltage` property next to it is the nominal one the vehicle is built with
             * and that the backend keeps as NominalBatteryVoltage (Mover.cpp:946) - the two are
             * only equal on a full battery. */
            virtual double get_live_battery_voltage() const = 0;
            virtual double get_tachometer_speed() const = 0;
            virtual double get_tachometer_speed_jump() const = 0;
            virtual double get_tachometer_clock_speed() const = 0;
            virtual int get_direction_absolute() const = 0;
            virtual int get_cabin() const = 0;
            virtual bool get_cabin_controleable() const = 0;
            virtual int get_cabin_occupied() const = 0;
            virtual bool get_battery_enabled() const = 0;
            virtual bool get_radio_enabled() const = 0;
            virtual bool get_radio_powered() const = 0;
            int get_radio_channel() const;
            virtual double get_power24_voltage() const = 0;
            virtual bool get_power24_available() const = 0;
            virtual bool get_power110_available() const = 0;
            virtual double get_current0() const = 0;
            virtual double get_current1() const = 0;
            virtual double get_current2() const = 0;
            virtual bool get_relay_novolt() const = 0;
            virtual bool get_relay_overvoltage() const = 0;
            virtual bool get_relay_ground() const = 0;
            virtual int get_train_damage() const = 0;
            virtual int get_controller_second_position() const = 0;
            virtual int get_controller_main_position() const = 0;
            virtual int get_controller_joint_position() const = 0;
            virtual int get_controller_main_actual_position() const = 0;
            virtual int get_circuit_rlist_size() const = 0;


        public:
            /* shared enum for every FIZ "...Start=" device activation mode field (Cntrl. section);
             * duplicated from VehicleEngine::StartMode to avoid a circular include (VehicleEngine.hpp includes
             * VehicleComponent.hpp, which includes this file) */
            enum StartMode {
                START_MODE_DISABLED,
                START_MODE_MANUAL,
                START_MODE_AUTOMATIC,
                START_MODE_MANUAL_WITH_AUTO_FALLBACK,
                START_MODE_CONVERTER,
                START_MODE_BATTERY,
                START_MODE_DIRECTION,
            };

            /* The element a coupler attached or detached, as the original names them
             * (coupling::coupler, coupling::brakehose, ..., Mover.cpp:590) */
            enum CouplingElement {
                COUPLING_ELEMENT_COUPLER,
                COUPLING_ELEMENT_BRAKEHOSE,
                COUPLING_ELEMENT_MAINHOSE,
                COUPLING_ELEMENT_CONTROL,
                COUPLING_ELEMENT_GANGWAY,
                COUPLING_ELEMENT_HEATING,
            };

            /* Category= (train / road / ship / airplane) */
            enum Category {
                CATEGORY_TRAIN = 1,
                CATEGORY_ROAD = 2,
                CATEGORY_SHIP = 4,
                CATEGORY_AIRPLANE = 8,
            };

            /* Type= : bitmask identifying a vehicle's special-cased behavior family */
            enum TrainType {
                TRAIN_TYPE_DEFAULT = 0,
                TRAIN_TYPE_EZT = 1,
                TRAIN_TYPE_ET41 = 2,
                TRAIN_TYPE_ET42 = 4,
                TRAIN_TYPE_PSEUDODIESEL = 8,
                TRAIN_TYPE_ET22 = 0x10,
                TRAIN_TYPE_SN61 = 0x20,
                TRAIN_TYPE_EP05 = 0x40,
                TRAIN_TYPE_ET40 = 0x80,
                TRAIN_TYPE_181 = 0x100,
                TRAIN_TYPE_DMU = 0x200,
            };

            enum TrainPowerSource {
                POWER_SOURCE_NOT_DEFINED,
                POWER_SOURCE_INTERNAL,
                POWER_SOURCE_TRANSDUCER,
                POWER_SOURCE_GENERATOR,
                POWER_SOURCE_ACCUMULATOR,
                POWER_SOURCE_CURRENTCOLLECTOR,
                POWER_SOURCE_POWERCABLE,
                POWER_SOURCE_HEATER,
                POWER_SOURCE_MAIN
            };

            enum TrainPowerType {
                POWER_TYPE_NONE,
                POWER_TYPE_BIO,
                POWER_TYPE_MECH,
                POWER_TYPE_ELECTRIC,
                POWER_TYPE_STEAM
            };


            static const char *mover_config_changed_signal;
            static const char *mover_initialized_signal;
            static const char *power_changed_signal;
            static const char *command_received;
            static const char *radio_toggled;
            static const char *radio_channel_changed;
            static const char *roof_light_changed;
            static const char *cabin_occupied_changed;
            static const char *config_changed;
            static const char *position_changed_signal;
            /// The consist this vehicle belongs to gained or lost a vehicle
            static const char *consist_changed_signal;
            /// One coupling element attached / detached, once per event. Two signals rather than
            /// one carrying a direction: every listener would have opened by branching on it.
            static const char *coupler_attached_signal;
            static const char *coupler_detached_signal;

            Dictionary get_config() const;
            /* One of this vehicle's components (re)applied its configuration. */
            void emit_config_changed();
            void _notification(int p_what);
            Variant send_command(
                    const StringName &p_command, const Variant &p_p1 = Variant(),
                    const Variant &p_p2 = Variant()) const;
            virtual void battery(bool p_enabled) const = 0;
            virtual void cab_activation(bool p_enabled) const = 0;
            virtual void cab_activation_auto() const = 0;
            virtual void cab_change(int p_direction) const = 0;
            virtual void main_controller_increase(int p_step = 1) const = 0;
            virtual void main_controller_decrease(int p_step = 1) const = 0;
            virtual void second_controller_increase(int p_step = 1) const = 0;
            virtual void second_controller_decrease(int p_step = 1) const = 0;
            virtual void direction_increase() const = 0;
            virtual void direction_decrease() const = 0;
            virtual void radio(bool p_enabled) = 0;
            void radio_channel_set(int p_channel);
            void radio_channel_increase(int p_step = 1);
            void radio_channel_decrease(int p_step = 1);
            /* A command has run against this vehicle. Its state has moved on in the middle of a
             * step, which is the one thing a dump cached for that step cannot see by itself -
             * hence the serial below (RailVehicleServer::vehicle_dump_state). */
            void command_executed(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            uint64_t get_command_serial() const;
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void register_command(const String &p_command, const Callable &p_callable);
            void unregister_command(const String &p_command, const Callable &p_callable);
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
            void initialize();
            /* The reverse: the vehicle leaves TrainSystem and gives its commands back. */
            void shutdown();
            void process_components(double p_delta);
            virtual void update_state();
            /// Straight from the backend, for the per-frame readers that only want this one number
            /// and would otherwise force the whole state dictionary to be rebuilt
            virtual double get_velocity() const = 0;
            /// Straight from the backend, like get_velocity() - the speed readers want this
            /// one number, not the whole state
            virtual double get_speed() const = 0;
            /// The rest of what every vehicle has, whatever it is made of. Read straight from the
            /// backend - nothing is stored, and the dump is built from these.
            virtual double get_mass_total() const = 0;
            virtual double get_total_distance() const = 0;
            virtual int get_direction() const = 0;
            virtual void apply_config() = 0;
            virtual double process_movement(double p_delta) = 0;
            virtual void update_location() = 0;
            virtual void update_neighbour(int p_end, VehicleController *p_other, int p_other_end, double p_track_distance) = 0;
            virtual void compute_forces(double p_delta) = 0;
            virtual void compute_movement(double p_delta) = 0;
            virtual void compute_fast_movement(double p_delta) = 0;
            virtual bool is_physics_active() const = 0;
            virtual void couple(VehicleController *p_other, int p_end, int p_other_end, int p_coupling_type) = 0;
            virtual void uncouple(int p_end) = 0;
            virtual bool is_coupled(int p_end) const = 0;
            virtual void coupler_connect(const Variant &p_where) = 0;
            virtual void coupler_disconnect(const Variant &p_where) = 0;
            virtual VehicleController *get_coupled_controller(int p_end) const = 0;
            virtual int get_coupled_end(int p_end) const = 0;
            void set_driver_type(DriverType p_value);
            DriverType get_driver_type() const;
            /* The cab the driver_type sits in, as the backend counts it: 1 for the front cab, -1 for
             * the rear one, 0 for nobody (TMoverParameters::CabActivisation). */
            int get_occupied_cab() const;
            static void _bind_methods();
            void change_track(const String &p_track_name, float p_track_offset, int p_track_direction);
            /* This vehicle's handle in RailVehicleServer, set when the server attaches it. */
            void set_vehicle_rid(const RID &p_vehicle_rid);
            RID get_rid() const;
            void emit_position_changed_if_needed();
            Vector3 get_world_position() const;
            Transform3D get_world_transform() const;
            MAKE_MEMBER_GS(String, train_id, "");
            /* What the vehicle carries when the scenery places it, as the `.scn` names it - the
             * amount and the cargo's own name (`loadcount` and `loadtype` of a `dynamic`). The
             * backend takes both at once, and it reads more than cargo out of them: `pantstate`
             * is how a scenery starts a locomotive with its pantographs already up
             * (TMoverParameters::AssignLoad, Mover.cpp:7647). */
            MAKE_MEMBER_GS(String, load_name, "");
            MAKE_MEMBER_GS(double, load_amount, 0.0);
            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS(double, battery_voltage, 0.0); // FIXME: move to TrainPower ?
            MAKE_MEMBER_GS(double, mass, 0.0);
            MAKE_MEMBER_GS(double, power, 0.0);
            MAKE_MEMBER_GS(double, max_velocity, 0.0);
            // original engine hardcodes this same 1..10 range for every vehicle
            // (OnCommand_radiochannelset: std::clamp((int)Command.param1, 1, 10)) - it is not
            // actually per-vehicle configurable there, so these default to the same range rather
            // than 0..0 (which silently clamped every radio_channel_increase/decrease/set call to
            // a no-op on any vehicle that never overrides them, since none currently do).
            MAKE_MEMBER_GS(int, radio_channel_min, 1);
            MAKE_MEMBER_GS(int, radio_channel_max, 10);
            MAKE_MEMBER_GS_NR(Category, category, CATEGORY_TRAIN);
            MAKE_MEMBER_GS_NR(TrainType, train_type, TRAIN_TYPE_DEFAULT);
            MAKE_MEMBER_GS(double, reduced_mass, 0.0);
            MAKE_MEMBER_GS(double, sand_capacity, 0.0);
            MAKE_MEMBER_GS(double, heating_power, 0.0);
            MAKE_MEMBER_GS(double, light_power, 0.0);
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

            /* Cntrl. (ogolne, bateria/przekaznik ziemnozwarciowy/oswietlenie przedzialow/aktywacja kabiny) */
            MAKE_MEMBER_GS_NR(StartMode, cntrl_battery_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, cntrl_ground_relay_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, cntrl_compartment_lights_start_mode, START_MODE_DISABLED);
            MAKE_MEMBER_GS(bool, cntrl_automatic_cab_activation, true);
            MAKE_MEMBER_GS(int, cntrl_inactive_cab_flag, 0);
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
VARIANT_ENUM_CAST(VehicleController::TrainPowerSource);
VARIANT_ENUM_CAST(VehicleController::TrainPowerType);
VARIANT_ENUM_CAST(VehicleController::Category);
VARIANT_ENUM_CAST(VehicleController::CouplingElement);
VARIANT_ENUM_CAST(VehicleController::TrainType);
VARIANT_ENUM_CAST(VehicleController::StartMode);
