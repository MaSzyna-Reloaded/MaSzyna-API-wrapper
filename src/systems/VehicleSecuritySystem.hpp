#pragma once
#include "../core/VehicleComponent.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleSecuritySystem : public VehicleComponent {
            GDCLASS(VehicleSecuritySystem, VehicleComponent)

        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_SECURITY;
            }

        private:
            static void _bind_methods();
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual bool get_beeping() const = 0;
            virtual bool get_blinking() const = 0;
            virtual bool get_radiostop_available() const = 0;
            virtual bool get_vigilance_blinking() const = 0;
            virtual bool get_cabsignal_blinking() const = 0;
            virtual bool get_cabsignal_beeping() const = 0;
            virtual bool get_braking() const = 0;
            virtual bool get_engine_blocked() const = 0;
            virtual bool get_separate_acknowledge() const = 0;
            enum EmergencySignal {
                EMERGENCY_SIGNAL_SIREN_LOW_TONE,
                EMERGENCY_SIGNAL_SIREN_HIGH_TONE,
                EMERGENCY_SIGNAL_WHISTLE
            };
            virtual void security_acknowledge(bool p_enabled) = 0;
            virtual void security_cabsignal_acknowledge() = 0;
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
