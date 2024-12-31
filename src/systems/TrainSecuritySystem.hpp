#pragma once
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainSecuritySystem : public TrainPart {
            GDCLASS(TrainSecuritySystem, TrainPart)

            static void _bind_methods();
            friend class TSecuritySystem;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_initialize_train_controller(TrainController *train_controller) override;
            void _on_train_controller_state_changed();
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            enum EmergencyBrakeWarningSignal {
                BRAKE_WARNINGSIGNAL_SIREN_LOWTONE,
                BRAKE_WARNINGSIGNAL_SIREN_HIGHTONE,
                BRAKE_WARNINGSIGNAL_WHISTLE
            };

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

            bool prev_beeping = false;
            bool prev_blinking = false;

        public:
            void security_acknowledge(bool p_enabled);

            // Getters
            [[nodiscard]] bool get_aware_system_active() const;
            [[nodiscard]] bool get_aware_system_cabsignal() const;
            [[nodiscard]] bool get_aware_system_separate_acknowledge() const;
            [[nodiscard]] bool get_aware_system_sifa() const;
            [[nodiscard]] double get_aware_delay() const;
            [[nodiscard]] double get_emergency_brake_delay() const;
            [[nodiscard]] EmergencyBrakeWarningSignal get_emergency_brake_warning_signal() const;
            [[nodiscard]] bool get_radio_stop() const;
            [[nodiscard]] double get_sound_signal_delay() const;
            [[nodiscard]] double get_shp_magnet_distance() const;
            [[nodiscard]] double get_ca_max_hold_time() const;

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
