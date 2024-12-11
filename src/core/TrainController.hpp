#pragma once

#include "LegacyRailVehicle.hpp"
#include "macros.hpp"

namespace godot {
    class TrainSystem;

    class TrainController : public LegacyRailVehicle {
            GDCLASS(TrainController, LegacyRailVehicle)

        private:
            int radio_channel = 0;
            bool prev_is_powered = false;
            bool prev_radio_enabled = false;
            int prev_radio_channel = 0;
            void refresh_runtime_signals();

        protected:
            void _notification(int p_what);
            void _notification_after_mover_ready() override;
            bool can_host_commands() const override;
            String get_command_target_id() const override;
            void _do_update_internal_mover(TMoverParameters *mover) const;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);

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

            static const char *POWER_CHANGED_SIGNAL;
            static const char *COMMAND_RECEIVED;
            static const char *RADIO_TOGGLED;
            static const char *RADIO_CHANNEL_CHANGED;

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

            void send_command(const StringName &command, const Variant &p1 = Variant(), const Variant &p2 = Variant()) const;
            void battery(bool p_enabled) const;
            void main_controller_increase(int p_step = 1) const;
            void main_controller_decrease(int p_step = 1) const;
            void direction_increase() const;
            void direction_decrease() const;
            void decouple(int relative_index);
            void radio(bool p_enabled);
            void radio_channel_set(int p_channel);
            void radio_channel_increase(int p_step = 1);
            void radio_channel_decrease(int p_step = 1);
            void emit_command_received_signal(
                    const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void register_command(const String &command, const Callable &callable);
            void unregister_command(const String &command, const Callable &callable);

            static void _bind_methods();

            MAKE_MEMBER_GS(String, train_id, "");
            MAKE_MEMBER_GS_DIRTY(double, power, 0.0);
            MAKE_MEMBER_GS_DIRTY(double, battery_voltage, 0.0);
            MAKE_MEMBER_GS(int, radio_channel_min, 0);
            MAKE_MEMBER_GS(int, radio_channel_max, 0);
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainController::TrainPowerSource);
VARIANT_ENUM_CAST(TrainController::TrainPowerType);
