#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include "TrackManager.hpp"
#include "TrainCommand.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
    class TrainPart;
    class TrainSet;
    class TrainController : public Node {
            GDCLASS(TrainController, Node)

        public:
            enum Side {
                FRONT = 0,
                BACK,
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

        private:
            Ref<TrainSet> trainset;
            TMoverParameters *mover = nullptr;
            Dictionary state;
            Dictionary config;
            Dictionary internal_state;
            int debug_tick_counter = 0;
            double _external_move_accumulator = 0.0;
            RID current_track_rid;
            String current_track_id;
            double current_track_offset = 0.0;
            bool _tick_state_ready = false;
            bool _pending_publish_valid = false;
            RID _pending_track_rid;
            String _pending_track_id;
            double _pending_track_offset = 0.0;
            Vector3 _pending_world_position;
            bool _dirty = false;
            bool _dirty_prop = false;

            double initial_velocity = 0.0;
            int cabin_number = 0;

            int radio_channel = 0;
            bool prev_is_powered = false;
            bool prev_radio_enabled = false;
            int prev_radio_channel = 0;
            void initialize_mover();
            void _update_mover_config_if_dirty();
            void _handle_mover_update();
            void _sync_mover_neighbours();
            bool _prepare_mover_tick();
            void _compute_mover_forces(double delta);
            void _compute_mover_movement(double delta);
            void _publish_mover_tick();
            void _clear_pending_mover_tick();
            bool _resolve_track_state(
                    double requested_offset, RID &resolved_track_rid, String &resolved_track_id, double &resolved_offset,
                    Vector3 *resolved_position = nullptr, Vector3 *resolved_axis = nullptr,
                    TrackGeometry *resolved_shape = nullptr) const;
            bool _apply_resolved_track_state(
                    const RID &resolved_track_rid, const String &resolved_track_id, double resolved_offset);
            static void _process_all_movers(double delta);
            static void _register_active_controller(TrainController *controller);
            static void _unregister_active_controller(TrainController *controller);
            void sync_mover_coupling(TrainController *other_vehicle, Side self_side, Side other_side, bool attach);
            static int to_mover_end(Side side);
            void refresh_runtime_signals();

        protected:
            void _notification(int p_what);
            void _process_mover(double delta);
            virtual void _notification_after_mover_ready();
            virtual void _notification_before_mover_cleanup();
            virtual void _do_update_internal_mover(TMoverParameters *mover) const;
            virtual void _do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const;
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);
            virtual void _on_coupled(TrainController *other_vehicle, Side self_side, Side other_side);
            virtual void _on_uncoupled(TrainController *other_vehicle, Side self_side, Side other_side);

        public:
            TrainController();
            ~TrainController() override;

            static const char *MOVER_CONFIG_CHANGED_SIGNAL;
            static const char *MOVER_INITIALIZED_SIGNAL;
            static const char *CONFIG_CHANGED;
            static const char *POWER_CHANGED_SIGNAL;
            static const char *COMMAND_RECEIVED;
            static const char *RADIO_TOGGLED;
            static const char *RADIO_CHANNEL_CHANGED;
            static const char *DERAILED_SIGNAL;

            TrainController *front = nullptr;
            TrainController *back = nullptr;

            void _process(double delta) override;

            const std::map<TrainPowerSource, TPowerSource> power_source_map = {
                    {POWER_SOURCE_NOT_DEFINED, TPowerSource::NotDefined},
                    {POWER_SOURCE_INTERNAL, TPowerSource::InternalSource},
                    {POWER_SOURCE_TRANSDUCER, TPowerSource::Transducer},
                    {POWER_SOURCE_GENERATOR, TPowerSource::Generator},
                    {POWER_SOURCE_ACCUMULATOR, TPowerSource::Accumulator},
                    {POWER_SOURCE_CURRENTCOLLECTOR, TPowerSource::CurrentCollector},
                    {POWER_SOURCE_POWERCABLE, TPowerSource::PowerCable},
                    {POWER_SOURCE_HEATER, TPowerSource::Heater},
                    {POWER_SOURCE_MAIN, TPowerSource::Main}};

            const std::map<TPowerSource, TrainPowerSource> tpower_source_map = {
                    {TPowerSource::NotDefined, POWER_SOURCE_NOT_DEFINED},
                    {TPowerSource::InternalSource, POWER_SOURCE_INTERNAL},
                    {TPowerSource::Transducer, POWER_SOURCE_TRANSDUCER},
                    {TPowerSource::Generator, POWER_SOURCE_GENERATOR},
                    {TPowerSource::Accumulator, POWER_SOURCE_ACCUMULATOR},
                    {TPowerSource::CurrentCollector, POWER_SOURCE_CURRENTCOLLECTOR},
                    {TPowerSource::PowerCable, POWER_SOURCE_POWERCABLE},
                    {TPowerSource::Heater, POWER_SOURCE_HEATER},
                    {TPowerSource::Main, POWER_SOURCE_MAIN}};

            const std::map<TrainPowerType, TPowerType> power_type_map = {
                    {POWER_TYPE_NONE, TPowerType::NoPower},
                    {POWER_TYPE_BIO, TPowerType::BioPower},
                    {POWER_TYPE_MECH, TPowerType::MechPower},
                    {POWER_TYPE_ELECTRIC, TPowerType::ElectricPower},
                    {POWER_TYPE_STEAM, TPowerType::SteamPower}};

            const std::map<TPowerType, TrainPowerType> tpower_type_map = {
                    {TPowerType::NoPower, POWER_TYPE_NONE},
                    {TPowerType::BioPower, POWER_TYPE_BIO},
                    {TPowerType::MechPower, POWER_TYPE_MECH},
                    {TPowerType::ElectricPower, POWER_TYPE_ELECTRIC},
                    {TPowerType::SteamPower, POWER_TYPE_STEAM}};

            String _to_string() const;
            void couple(TrainController *other_vehicle, Side self_side, Side other_side);
            void couple_front(TrainController *other_vehicle, Side other_side);
            void couple_back(TrainController *other_vehicle, Side other_side);
            TrainController *get_front_vehicle() const;
            TrainController *get_back_vehicle() const;
            Array get_rail_vehicle_modules() const;
            Ref<TrainSet> get_trainset() const;
            TrainController *decouple(int relative_index);
            TrainController *uncouple_front();
            TrainController *uncouple_back();

            Dictionary get_config() const;
            void update_config(const Dictionary &p_config);
            Dictionary get_state();
            Dictionary &_get_state_internal();
            Dictionary get_mover_state();
            void update_state();
            void update_mover();
            TMoverParameters *get_mover() const;
            void set_mover_location(const Vector3 &position);
            Vector3 get_mover_location() const;
            void assign_track_rid(const RID &track_rid, const String &track_id = "");
            RID get_track_rid() const;
            void assign_track(const String &track_id);
            String get_track_id() const;
            void set_track_offset(double offset);
            double get_track_offset() const;

            TypedArray<TrainCommand> get_supported_commands();
            void battery(bool p_enabled) const;
            void main_controller_increase(int p_step = 1) const;
            void main_controller_decrease(int p_step = 1) const;
            void direction_increase() const;
            void direction_decrease() const;
            void radio(bool p_enabled);
            void radio_channel_set(int p_channel);
            void radio_channel_increase(int p_step = 1);
            void radio_channel_decrease(int p_step = 1);
            void emit_command_received_signal(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());

            static void _bind_methods();

            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS_DIRTY(float, drag_coefficient, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, mass, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, max_velocity, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, length, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, width, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, height, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, power, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, battery_voltage, 0.0);
            MAKE_MEMBER_GS(int, radio_channel_min, 0);
            MAKE_MEMBER_GS(int, radio_channel_max, 0);
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainController::Side);
VARIANT_ENUM_CAST(TrainController::TrainPowerSource);
VARIANT_ENUM_CAST(TrainController::TrainPowerType);
