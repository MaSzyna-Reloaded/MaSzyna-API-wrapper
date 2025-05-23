#pragma once
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainSecuritySystem : public TrainPart {
            GDCLASS(TrainSecuritySystem, TrainPart)

            static void _bind_methods();
            friend class TSecuritySystem;


        public:
            enum EBrakeWarningSignal : int {
                SIREN_LOW_TONE = 0,
                SIREN_HIGH_TONE,
                WHISTLE,
            };

            //////////
            ///
            /// Getter Methods
            //////////
            bool GetAwareSystemActive() const {
                return m_bAwareSystemActive;
            }

            bool GetAwareSystemCabinSignalEnabled() const {
                return m_bAwareSystemCabinSignalEnabled;
            }

            bool GetAwareSystemSeparateAcknowledge() const {
                return m_bAwareSystemSeparateAcknowledge;
            }

            bool GetAwareSystemSifaAvailable() const {
                return m_bAwareSystemSifaAvailable;
            }

            double GetAwareDelay() const {
                return m_dAwareDelay;
            }

            double GetEmergencyBrakeDelay() const {
                return m_dEmergencyBrakeDelay;
            }

            EBrakeWarningSignal GetBrakeWarningSignalType() const {
                return m_CurrentBrakeWarningSignal;
            }

            bool GetRadioStopEnabled() const {
                return m_bRadioStopEnabled;
            }

            double GetSoundSignalDelay() const {
                return m_dSoundSignalDelay;
            }

            double GetShpMagnetDistance() const {
                return m_dShpMagnetDistance;
            }

            double GetAwareSystemMaxHoldTime() const {
                return m_dAwareSystemMaxHoldTime;
            }

            //////////
            ///
            /// Setter Methods
            //////////

            void SetSecurityAcknowledgeEnabled(bool enabled);

            void SetAwareSystemActive(const bool enabled) {
                m_bAwareSystemActive = enabled;
                _dirty = true;
            }

            void SetAwareSystemCabinSignalEnabled(const bool enabled) {
                m_bAwareSystemCabinSignalEnabled = enabled;
                _dirty = true;
            }

            void SetAwareSystemSeparateAcknowledge(const bool enabled) {
                m_bAwareSystemSeparateAcknowledge = enabled;
                _dirty = true;
            }

            void SetAwareSystemSifaAvailable(const bool enabled) {
                m_bAwareSystemSifaAvailable = enabled;
                _dirty = true;
            }

            void SetAwareDelay(const double value) {
                m_dAwareDelay = value;
                _dirty = true;
            }

            void SetEmergencyBrakeDelay(const double value) {
                m_dEmergencyBrakeDelay = value;
                _dirty = true;
            }

            void SetBrakeWarningSignalType(const EBrakeWarningSignal eValue) {
                m_CurrentBrakeWarningSignal = eValue;
                _dirty = true;
            }

            void SetRadioStopEnabled(const bool enabled) {
                m_bRadioStopEnabled = enabled;
                _dirty = true;
            }

            void SetSoundSignalDelay(const double value) {
                m_dSoundSignalDelay = value;
                _dirty = true;
            }

            void SetShpMagnetDistance(const double value) {
                m_dShpMagnetDistance = value;
                _dirty = true;
            }

            void SetAwareSystemMaxHoldTime(const double value) {
                m_dAwareSystemMaxHoldTime = value;
                _dirty = true;
            }

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _register_commands() override;
            void _unregister_commands() override;

        private:
            bool m_bAwareSystemActive = false;
            bool m_bAwareSystemCabinSignalEnabled = false;
            bool m_bAwareSystemSeparateAcknowledge = false;
            bool m_bAwareSystemSifaAvailable = false;
            bool m_bRadioStopEnabled = true; // RadioStop -> SecuritySystem->radiostop_enabled

            double m_dAwareDelay = 0.0;          // AwareDelay -> SecuritySystem->AwareDelay
            double m_dEmergencyBrakeDelay = 0.0; // EmergencyBrakeDelay -> SecuritySystem->EmergencyBrakeDelay

            double m_dSoundSignalDelay = 0.0;      // SoundSignalDelay -> SecuritySystem->SoundSignalDelay
            double m_dShpMagnetDistance = 0.0;     // MagnetLocation -> SecuritySystem->MagnetLocation
            double m_dAwareSystemMaxHoldTime = 0.0; // MaxHoldTime -> SecuritySystem->MaxHoldTime

            EBrakeWarningSignal m_CurrentBrakeWarningSignal = SIREN_HIGH_TONE;
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainSecuritySystem::EBrakeWarningSignal)
