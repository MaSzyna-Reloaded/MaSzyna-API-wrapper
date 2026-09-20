#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <unordered_map>


namespace godot {
    class TrainBrake;
    class TrainPart;
    class TrainEngine;
    class TrainSecuritySystem;
    class TrainSystem;


    class TrainController : public Node {
            GDCLASS(TrainController, Node)
        private:
            TMoverParameters *mover{};
            int cabin_number = 0;
            void initialize_mover();
            void initialize_mover_state();
            bool dirty = false;      // Refreshes all elements
            bool dirty_prop = false; // Refreshes only TrainController's properties
            Dictionary state;
            Dictionary config;
            Dictionary internal_state;
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
            int prev_cabin_occupied = 0;

            // Hasler speed recorder (Train.cpp:6917-6940 fTachoVelocity/fTachoVelocityJump/fTachoCount)
            double tacho_velocity = 0.0;
            double tacho_velocity_jump = 0.0;
            double tacho_count = 0.0;
            double tacho_time = 0.0;
            bool tacho_clock_active = false;
            void _update_tachometer(double p_delta);

            void _collect_train_parts(const Node *p_node, Vector<TrainPart *> &p_train_parts) {};
            void _update_mover_config_if_dirty();
            void _handle_mover_update();
            Object *_get_rail_vehicle_physics_server() const;
            void _emit_position_changed_if_needed();
            int _resolve_coupler_end(const Variant &p_where) const;
            // attach/detach sound requests of the couplers, one counter per coupling type and direction
            int coupler_sound_counts[12] = {};
            void _consume_coupler_sounds(TMoverParameters *p_mover, Dictionary &p_state);

        protected:
            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            Dictionary get_mover_state();
            // TrainController mozna bedzie rozszerzac klasami pochodnymi i przeslaniac metody
            void _do_update_internal_mover(TMoverParameters *p_mover) const;
            void _do_fetch_config_from_mover(const TMoverParameters *p_mover, Dictionary &p_config) const;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state);
            void _process_mover(double p_delta);


        public:
            /* shared enum for every FIZ "...Start=" device activation mode field (Cntrl. section);
             * duplicated from TrainEngine::StartMode to avoid a circular include (TrainEngine.hpp includes
             * TrainPart.hpp, which includes this file) */
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

            Dictionary get_config() const;
            void update_config(const Dictionary &p_config);
            void _process(double p_delta) override;
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
            void emit_command_received_signal(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void register_command(const String &p_command, const Callable &p_callable);
            void unregister_command(const String &p_command, const Callable &p_callable);
            void update_state();
            void update_mover();
            double process_movement(double p_delta);
            void update_location();
            void update_neighbour(int p_end, TrainController *p_other, int p_other_end, double p_track_distance);
            void compute_forces(double p_delta);
            void compute_movement(double p_delta);
            void compute_fast_movement(double p_delta);
            bool is_physics_active() const;
            void couple(TrainController *p_other, int p_end, int p_other_end, int p_coupling_type);
            void uncouple(int p_end);
            bool is_coupled(int p_end) const;
            void coupler_connect(const Variant &p_where);
            void coupler_disconnect(const Variant &p_where);
            TMoverParameters *get_mover() const;
            TrainController *get_coupled_controller(int p_end) const;
            int get_coupled_end(int p_end) const;
            void set_cabin_number(int p_value);
            int get_cabin_number() const;
            static void _bind_methods();
            void change_track(const String &p_track_name, float p_track_offset, int p_track_direction);
            RID get_rid() const;
            Vector3 get_world_position() const;
            Transform3D get_world_transform() const;
            MAKE_MEMBER_GS(String, train_id, "");
            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS_DIRTY(double, battery_voltage, 0.0); // FIXME: move to TrainPower ?
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

        private:
            // coupled movers only know each other (TCoupling::Connected) - maps them back to controllers
            static std::unordered_map<const TMoverParameters *, TrainController *> controllers_by_mover;
            RID rid;
            Vector3 last_emitted_position = Vector3(1e10, 1e10, 1e10);
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainController::TrainPowerSource);
VARIANT_ENUM_CAST(TrainController::TrainPowerType);
VARIANT_ENUM_CAST(TrainController::Category);
VARIANT_ENUM_CAST(TrainController::TrainType);
VARIANT_ENUM_CAST(TrainController::StartMode);
