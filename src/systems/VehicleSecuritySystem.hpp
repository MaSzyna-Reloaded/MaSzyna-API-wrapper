#pragma once
#include "../core/VehicleComponent.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleSecuritySystem : public VehicleComponent {
            GDCLASS(VehicleSecuritySystem, VehicleComponent)
        private:
            static void _bind_methods();
            friend class TSecuritySystem;

        private:

        protected:
            /// Change detection for blinking_changed/beeping_changed, compared in
            /// _do_process_mover(). Both start false, as the not-yet-filled state dictionary
            /// they used to be compared against did.
            bool previous_blinking = false;
            bool previous_beeping = false;

            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
            void _register_commands() override;
            void _unregister_commands() override;

            
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            enum EmergencySignal {
                EMERGENCY_SIGNAL_SIREN_LOW_TONE,
                EMERGENCY_SIGNAL_SIREN_HIGH_TONE,
                EMERGENCY_SIGNAL_WHISTLE
            };

            void security_acknowledge(bool p_enabled);
            void security_cabsignal_acknowledge();

            MAKE_MEMBER_GS(bool, aware_system_active, false);
            MAKE_MEMBER_GS(bool, aware_system_cabsignal, false);
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
VARIANT_ENUM_CAST(VehicleSecuritySystem::EmergencySignal)
