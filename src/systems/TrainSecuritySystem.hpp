#pragma once
#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainSecuritySystem : public TrainPart {
            GDCLASS(TrainSecuritySystem, TrainPart)
        private:
            static void _bind_methods();
            friend class TSecuritySystem;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            enum EmergencySignal {
                EMERGENCY_SIGNAL_SIREN_LOW_TONE,
                EMERGENCY_SIGNAL_SIREN_HIGH_TONE,
                EMERGENCY_SIGNAL_WHISTLE
            };

            TrainSecuritySystem();
            void security_acknowledge(bool p_enabled);

            MAKE_MEMBER_GS(bool, aware_system_active, false);
            MAKE_MEMBER_GS(bool, aware_system_cab_signal, false);
            MAKE_MEMBER_GS(bool, aware_system_separate_acknowledge, false);
            MAKE_MEMBER_GS(bool, aware_system_sifa, false);
            MAKE_MEMBER_GS(double, aware_delay, 0.0);
            MAKE_MEMBER_GS(double, emergency_brake_delay, 0.0);
            MAKE_MEMBER_GS_DIRTY(bool, radio_stop_enabled, false);
            MAKE_MEMBER_GS(double, sound_signal_delay, 0.0);
            MAKE_MEMBER_GS(double, shp_magnet_distance, 0.0);
            MAKE_MEMBER_GS(double, ca_max_hold_time, 0.0);
            MAKE_MEMBER_GS_NR(EmergencySignal, emergency_signal, EmergencySignal::EMERGENCY_SIGNAL_SIREN_HIGH_TONE);
    };
} // namespace godot
VARIANT_ENUM_CAST(TrainSecuritySystem::EmergencySignal)
