#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleComponentType.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <unordered_map>


namespace godot {
    class VehicleBrake;
    class VehicleComponent;
    class VehicleEngine;
    class VehicleSecuritySystem;
    class VehicleLighting;
    class TrainSystem;


    /// The vehicle itself: the Mover, the components and the operations that change them. It is
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
            /* Owned by MaszynaMoverPhysicsServer, which created it and will free it; this
             * is a borrowed pointer, cached because every component reaches for it per
             * frame. It is never deleted here. */
            TMoverParameters *mover{};
            /* This vehicle's handle in the simulation backend. */
            RID physics_rid;
            DriverType driver_type = DRIVER_NOBODY;
            void initialize_mover();
            void initialize_mover_state();
            /// state is rebuilt from the mover when it is asked for, not on every physics step:
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

            // Hasler speed recorder (Train.cpp:6917-6940 fTachoVelocity/fTachoVelocityJump/fTachoCount)
            double tacho_velocity = 0.0;
            double tacho_velocity_jump = 0.0;
            double tacho_count = 0.0;
            double tacho_time = 0.0;
            bool tacho_clock_active = false;
            void _update_tachometer(double p_delta);

            /// Writes the wrapper's configuration - the vehicle's and every component's - to the
            /// backend, then announces that the backend carries it.
            void apply_configuration();
            void _handle_mover_update();
            int _resolve_coupler_end(const Variant &p_where) const;
            void _consume_coupler_sounds(TMoverParameters *p_mover);

        protected:
            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            // VehicleController mozna bedzie rozszerzac klasami pochodnymi i przeslaniac metody
            void _do_update_internal_mover(TMoverParameters *p_mover) const;
            void _fill_config_dictionary(Dictionary &p_config) const;
            /* The vehicle's own share of the dump - what every vehicle has, whatever it is
             * made of. Its components add theirs. */
            void _fill_state_dictionary(Dictionary &p_state) const;

            /* Live state, read straight from the backend - nothing is stored. */
            /* The battery as it actually is, which drains and recharges. The authored
             * `battery_voltage` property next to it is the nominal one the vehicle is built with
             * and that the backend keeps as NominalBatteryVoltage (Mover.cpp:946) - the two are
             * only equal on a full battery. */
            double get_live_battery_voltage() const;
            double get_tachometer_speed() const;
            double get_tachometer_speed_jump() const;
            double get_tachometer_clock_speed() const;
            int get_direction_absolute() const;
            int get_cabin() const;
            bool get_cabin_controleable() const;
            int get_cabin_occupied() const;
            bool get_battery_enabled() const;
            bool get_radio_enabled() const;
            bool get_radio_powered() const;
            int get_radio_channel() const;
            double get_power24_voltage() const;
            bool get_power24_available() const;
            bool get_power110_available() const;
            double get_current0() const;
            double get_current1() const;
            double get_current2() const;
            bool get_relay_novolt() const;
            bool get_relay_overvoltage() const;
            bool get_relay_ground() const;
            int get_train_damage() const;
            int get_controller_second_position() const;
            int get_controller_main_position() const;
            int get_controller_joint_position() const;
            int get_controller_main_actual_position() const;
            int get_circuit_rlist_size() const;
            void _process_mover(double p_delta);


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

            const std::map<StartMode, Maszyna::start_t> start_mode_map = {
                    {START_MODE_DISABLED, Maszyna::start_t::disabled},
                    {START_MODE_MANUAL, Maszyna::start_t::manual},
                    {START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                    {START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                    {START_MODE_CONVERTER, Maszyna::start_t::converter},
                    {START_MODE_BATTERY, Maszyna::start_t::battery},
                    {START_MODE_DIRECTION, Maszyna::start_t::direction},
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

            const std::map<TrainPowerSource, TPowerSource> power_source_map = {
                    {TrainPowerSource::POWER_SOURCE_NOT_DEFINED, TPowerSource::NotDefined},
                    {TrainPowerSource::POWER_SOURCE_INTERNAL, TPowerSource::InternalSource},
                    {TrainPowerSource::POWER_SOURCE_TRANSDUCER, TPowerSource::Transducer},
                    {TrainPowerSource::POWER_SOURCE_GENERATOR, TPowerSource::Generator},
                    {TrainPowerSource::POWER_SOURCE_ACCUMULATOR, TPowerSource::Accumulator},
                    {TrainPowerSource::POWER_SOURCE_CURRENTCOLLECTOR, TPowerSource::CurrentCollector},
                    {TrainPowerSource::POWER_SOURCE_POWERCABLE, TPowerSource::PowerCable},
                    {TrainPowerSource::POWER_SOURCE_HEATER, TPowerSource::Heater},
                    {TrainPowerSource::POWER_SOURCE_MAIN, TPowerSource::Main}};

            const std::map<TPowerSource, TrainPowerSource> tpower_source_map = {
                    {TPowerSource::NotDefined, TrainPowerSource::POWER_SOURCE_NOT_DEFINED},
                    {TPowerSource::InternalSource, TrainPowerSource::POWER_SOURCE_INTERNAL},
                    {TPowerSource::Transducer, TrainPowerSource::POWER_SOURCE_TRANSDUCER},
                    {TPowerSource::Generator, TrainPowerSource::POWER_SOURCE_GENERATOR},
                    {TPowerSource::Accumulator, TrainPowerSource::POWER_SOURCE_ACCUMULATOR},
                    {TPowerSource::CurrentCollector, TrainPowerSource::POWER_SOURCE_CURRENTCOLLECTOR},
                    {TPowerSource::PowerCable, TrainPowerSource::POWER_SOURCE_POWERCABLE},
                    {TPowerSource::Heater, TrainPowerSource::POWER_SOURCE_HEATER},
                    {TPowerSource::Main, TrainPowerSource::POWER_SOURCE_MAIN}};

            const std::map<TrainPowerType, TPowerType> power_type_map = {
                    {TrainPowerType::POWER_TYPE_NONE, TPowerType::NoPower},
                    {TrainPowerType::POWER_TYPE_BIO, TPowerType::BioPower},
                    {TrainPowerType::POWER_TYPE_MECH, TPowerType::MechPower},
                    {TrainPowerType::POWER_TYPE_ELECTRIC, TPowerType::ElectricPower},
                    {TrainPowerType::POWER_TYPE_STEAM, TPowerType::SteamPower}};

            const std::map<TPowerType, TrainPowerType> tpower_type_map = {
                    {TPowerType::NoPower, TrainPowerType::POWER_TYPE_NONE},
                    {TPowerType::BioPower, TrainPowerType::POWER_TYPE_BIO},
                    {TPowerType::MechPower, TrainPowerType::POWER_TYPE_MECH},
                    {TPowerType::ElectricPower, TrainPowerType::POWER_TYPE_ELECTRIC},
                    {TPowerType::SteamPower, TrainPowerType::POWER_TYPE_STEAM}};

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
            void battery(bool p_enabled) const;
            void cab_activation(bool p_enabled) const;
            void cab_activation_auto() const;
            void cab_change(int p_direction) const;
            void main_controller_increase(int p_step = 1) const;
            void main_controller_decrease(int p_step = 1) const;
            void second_controller_increase(int p_step = 1) const;
            void second_controller_decrease(int p_step = 1) const;
            void direction_increase() const;
            void direction_decrease() const;
            void radio(bool p_enabled);
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
            bool is_simulation_ready() const;
            void attach_to_system();
            /// Lets go of everything this vehicle holds - its components, its registration and
            /// its simulation - without destroying the vehicle, so every reference to it stays
            /// valid across a rebuild.
            void release();
            void initialize();
            /* The reverse: the vehicle leaves TrainSystem and gives its commands back. */
            void shutdown();
            void process_components(double p_delta);
            void update_state();
            /// Straight from the mover, for the per-frame readers that only want this one number
            /// and would otherwise force the whole state dictionary to be rebuilt
            double get_velocity() const;
            /// Straight from the backend, like get_velocity() - the speed readers want this
            /// one number, not the whole state
            double get_speed() const;
            /// The rest of what every vehicle has, whatever it is made of. Read straight from the
            /// backend - nothing is stored, and the dump is built from these.
            double get_mass_total() const;
            double get_total_distance() const;
            int get_direction() const;
            void apply_config();
            double process_movement(double p_delta);
            void update_location();
            void update_neighbour(int p_end, VehicleController *p_other, int p_other_end, double p_track_distance);
            void compute_forces(double p_delta);
            void compute_movement(double p_delta);
            void compute_fast_movement(double p_delta);
            bool is_physics_active() const;
            void couple(VehicleController *p_other, int p_end, int p_other_end, int p_coupling_type);
            void uncouple(int p_end);
            bool is_coupled(int p_end) const;
            void coupler_connect(const Variant &p_where);
            void coupler_disconnect(const Variant &p_where);
            TMoverParameters *get_mover() const;
            VehicleController *get_coupled_controller(int p_end) const;
            int get_coupled_end(int p_end) const;
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
            // coupled movers only know each other (TCoupling::Connected) - maps them back to controllers
            static std::unordered_map<const TMoverParameters *, VehicleController *> controllers_by_mover;
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
