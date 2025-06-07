#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>


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
            double initial_velocity = 0.0;
            int cabin_number = 0;
            void initialize_mover();
            bool _dirty = false;      // Refreshes all elements
            bool _dirty_prop = false; // Refreshes only TrainController's properties
            Dictionary state;
            Dictionary config;
            Dictionary internal_state;
            int radio_channel = 0;

            bool prev_is_powered = false;
            bool prev_radio_enabled = false;
            int prev_radio_channel = radio_channel;

            void _collect_train_parts(const Node *node, Vector<TrainPart *> &train_parts) {};
            void _update_mover_config_if_dirty();
            void _handle_mover_update();

        protected:
            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            Dictionary get_mover_state();
            // TrainController mozna bedzie rozszerzac klasami pochodnymi i przeslaniac metody
            void _do_update_internal_mover(TMoverParameters *mover) const;
            void _do_fetch_config_from_mover(const TMoverParameters *mover, Dictionary &config) const;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);
            void _process_mover(double delta);


        public:
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
                    {TrainPowerSource::POWER_SOURCE_MAIN, TPowerSource::Main}
            };

            const std::map<TPowerSource, TrainPowerSource> tpower_source_map = {
                {TPowerSource::NotDefined, TrainPowerSource::POWER_SOURCE_NOT_DEFINED},
                {TPowerSource::InternalSource, TrainPowerSource::POWER_SOURCE_INTERNAL},
                {TPowerSource::Transducer, TrainPowerSource::POWER_SOURCE_TRANSDUCER},
                {TPowerSource::Generator, TrainPowerSource::POWER_SOURCE_GENERATOR},
                {TPowerSource::Accumulator, TrainPowerSource::POWER_SOURCE_ACCUMULATOR},
                {TPowerSource::CurrentCollector, TrainPowerSource::POWER_SOURCE_CURRENTCOLLECTOR},
                {TPowerSource::PowerCable, TrainPowerSource::POWER_SOURCE_POWERCABLE},
                {TPowerSource::Heater, TrainPowerSource::POWER_SOURCE_HEATER},
                {TPowerSource::Main, TrainPowerSource::POWER_SOURCE_MAIN}
            };

            const std::map<TrainPowerType, TPowerType> power_type_map = {
                    {TrainPowerType::POWER_TYPE_NONE, TPowerType::NoPower},
                    {TrainPowerType::POWER_TYPE_BIO, TPowerType::BioPower},
                    {TrainPowerType::POWER_TYPE_MECH, TPowerType::MechPower},
                    {TrainPowerType::POWER_TYPE_ELECTRIC, TPowerType::ElectricPower},
                    {TrainPowerType::POWER_TYPE_STEAM, TPowerType::SteamPower}
            };

            const std::map<TPowerType, TrainPowerType> tpower_type_map = {
                    {TPowerType::NoPower, TrainPowerType::POWER_TYPE_NONE},
                    {TPowerType::BioPower, TrainPowerType::POWER_TYPE_BIO},
                    {TPowerType::MechPower, TrainPowerType::POWER_TYPE_MECH},
                    {TPowerType::ElectricPower, TrainPowerType::POWER_TYPE_ELECTRIC},
                    {TPowerType::SteamPower, TrainPowerType::POWER_TYPE_STEAM}
            };

            static const char *MOVER_CONFIG_CHANGED_SIGNAL;
            static const char *MOVER_INITIALIZED_SIGNAL;
            static const char *POWER_CHANGED_SIGNAL;
            static const char *COMMAND_RECEIVED;
            static const char *RADIO_TOGGLED;
            static const char *RADIO_CHANNEL_CHANGED;
            static const char *CONFIG_CHANGED;

            Dictionary get_config() const;
            void update_config(const Dictionary &p_config);
            void _process(double delta) override;
            void _notification(int p_what);
            void send_command(const StringName &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void battery(bool p_enabled);
            void main_controller_increase(int p_step = 1);
            void main_controller_decrease(int p_step = 1);
            void direction_increase();
            void direction_decrease();
            void radio(bool p_enabled);
            void radio_channel_set(int p_channel);
            void radio_channel_increase(int p_step = 1);
            void radio_channel_decrease(int p_step = 1);
            void emit_command_received_signal(
                    const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void register_command(const String &command, const Callable &callable);
            void unregister_command(const String &command, const Callable &callable);
            void update_state();
            void update_mover();
            TMoverParameters *get_mover() const;
            static void _bind_methods();
            MAKE_MEMBER_GS(String, train_id, "");
            MAKE_MEMBER_GS(String, type_name, "");
            MAKE_MEMBER_GS_DIRTY(double, battery_voltage, 0.0); // FIXME: move to TrainPower ?
            MAKE_MEMBER_GS(double, mass, 0.0);
            MAKE_MEMBER_GS(double, power, 0.0);
            MAKE_MEMBER_GS(double, max_velocity, 0.0);
            MAKE_MEMBER_GS(String, axle_arrangement, "");
            MAKE_MEMBER_GS(int, radio_channel_min, 0);
            MAKE_MEMBER_GS(int, radio_channel_max, 0);
            Dictionary get_state();
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainController::TrainPowerSource);
VARIANT_ENUM_CAST(TrainController::TrainPowerType);
