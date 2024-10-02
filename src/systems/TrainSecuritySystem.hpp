#pragma once
#include <godot_cpp/classes/node.hpp>
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"


namespace godot {
    class TrainSecuritySystem : public TrainPart {
            GDCLASS(TrainSecuritySystem, TrainPart)

            static void _bind_methods();
            friend class TSecuritySystem;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            enum EmergencyBrakeWarningSignal {
                BRAKE_WARNINGSIGNAL_SIREN_LOWTONE,
                BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE,
                BRAKE_WARNINGSIGNAL_WHISTLE
            };

            TrainSecuritySystem();
            ~TrainSecuritySystem() override = default;

        private:
            bool aware_system_active = false;
            bool aware_system_cabsignal = false;
            bool aware_system_separate_acknowledge = false;
            bool aware_system_sifa = false;

            double aware_delay = 0.0;           // AwareDelay -> SecuritySystem->AwareDelay
            double emergency_brake_delay = 0.0; // EmergencyBrakeDelay -> SecuritySystem->EmergencyBrakeDelay
            EmergencyBrakeWarningSignal emergency_brake_warning_signal =
                    BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE; // EmergencyBrakeWarningSignal ->
                                                        // EmergencyBrakeWarningSignal
            bool radio_stop = true;                     // RadioStop -> SecuritySystem->radiostop_enabled
            double sound_signal_delay = 0.0;            // SoundSignalDelay -> SecuritySystem->SoundSignalDelay
            double shp_magnet_distance = 0.0;           // MagnetLocation -> SecuritySystem->MagnetLocation
            double ca_max_hold_time = 0.0;              // MaxHoldTime -> SecuritySystem->MaxHoldTime

        public:
            void security_acknowledge(const bool p_enabled);

            // Getters
            bool get_aware_system_active() const;
            bool get_aware_system_cabsignal() const;
            bool get_aware_system_separate_acknowledge() const;
            bool get_aware_system_sifa() const;
            double get_aware_delay() const;
            double get_emergency_brake_delay() const;
            EmergencyBrakeWarningSignal get_emergency_brake_warning_signal() const;
            bool get_radio_stop() const;
            double get_sound_signal_delay() const;
            double get_shp_magnet_distance() const;
            double get_ca_max_hold_time() const;

            // Setters
            void set_aware_system_active(bool p_state);
            void set_aware_system_cabsignal(bool p_state);
            void set_aware_system_separate_acknowledge(bool p_state);
            void set_aware_system_sifa(bool p_state);
            void set_aware_delay(double value);
            void set_emergency_brake_delay(double value);
            void set_emergency_brake_warning_signal(EmergencyBrakeWarningSignal value);
            void set_radio_stop(bool value);
            void set_sound_signal_delay(double value);
            void set_shp_magnet_distance(double value);
            void set_ca_max_hold_time(double value);
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainSecuritySystem::EmergencyBrakeWarningSignal)
