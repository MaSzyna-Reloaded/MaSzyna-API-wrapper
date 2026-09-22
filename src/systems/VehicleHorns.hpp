#pragma once

#include "../core/VehicleComponent.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    // Ports the original engine's horn model (Train.cpp's OnCommand_hornlowactivate/
    // OnCommand_hornhighactivate/OnCommand_whistleactivate, DynObj.cpp's per-frame
    // WarningSignal -> sHorn1/sHorn2/sHorn3 dispatch): exactly 3 fixed slots (low/high/
    // whistle) backed by TMoverParameters::WarningSignal bits 1/2/4 - the same bit
    // convention VehicleSecuritySystem::emergency_signal already uses for
    // EmergencyBrakeWarningSignal. The original engine has no FIZ/mover-level config for
    // horn count - a vehicle's 0-3 horn complement is implied entirely by which MMD cabin
    // button (horn_bt:/hornlow_bt:/hornhigh_bt:/whistle_bt:) and sound (horn1:/horn2:/
    // horn3:) labels it declares. low_horn_enabled/high_horn_enabled/whistle_enabled
    // re-expose that same gate (the original's cabin SubModel-presence null check) as
    // explicit config, since this class has no visibility into cabin nodes - callers
    // building a vehicle from MMD data (e.g. DynamicRailVehicle3D) set these from label
    // presence.
    class VehicleHorns : public VehicleComponent {
            GDCLASS(VehicleHorns, VehicleComponent)

        private:
            static void _bind_methods();
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual bool get_low_pressed() const = 0;
            virtual bool get_high_pressed() const = 0;
            virtual bool get_whistle_pressed() const = 0;
            /* Below this the vehicle counts as standing, and the alarm chain does not sound
             * the emergency signal (DynObj.cpp, the same 0.5 m/s the original compares against) */
            static constexpr double HORN_EMERGENCY_MIN_SPEED = 0.5;
            /* DynObj.cpp's per-frame horn combination: while moving with the alarm chain
             * pulled, the emergency signal overrides the manually commanded one. */
            virtual int get_combined_signal() const = 0;
            virtual bool get_low_active() const = 0;
            virtual bool get_high_active() const = 0;
            virtual bool get_whistle_active() const = 0;
            virtual int get_horn() const = 0;
            virtual void set_horn_low(bool p_state) = 0;
            virtual void set_horn_high(bool p_state) = 0;
            virtual void set_whistle(bool p_state) = 0;
            // Compatibility entry point for a single bidirectional cabin widget (one physical
            // lever animating -1/0/+1) driving both low and high horn from one signed value,
            // e.g. CabinSwitch's command_set - positive activates the low horn, negative the
            // high horn, zero releases both. Internally routes to the same WarningSignal bits
            // as set_horn_low()/set_horn_high().
            virtual void set_horn(double p_position) = 0;
            MAKE_MEMBER_GS_NR(bool, low_horn_enabled, true);
            MAKE_MEMBER_GS_NR(bool, high_horn_enabled, true);
            MAKE_MEMBER_GS_NR(bool, whistle_enabled, true);
    };
} // namespace godot
