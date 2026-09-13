#pragma once

#include "../core/TrainPart.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    // Ports the original engine's horn model (Train.cpp's OnCommand_hornlowactivate/
    // OnCommand_hornhighactivate/OnCommand_whistleactivate, DynObj.cpp's per-frame
    // WarningSignal -> sHorn1/sHorn2/sHorn3 dispatch): exactly 3 fixed slots (low/high/
    // whistle) backed by TMoverParameters::WarningSignal bits 1/2/4 - the same bit
    // convention TrainSecuritySystem::emergency_signal already uses for
    // EmergencyBrakeWarningSignal. The original engine has no FIZ/mover-level config for
    // horn count - a vehicle's 0-3 horn complement is implied entirely by which MMD cabin
    // button (horn_bt:/hornlow_bt:/hornhigh_bt:/whistle_bt:) and sound (horn1:/horn2:/
    // horn3:) labels it declares. low_horn_enabled/high_horn_enabled/whistle_enabled
    // re-expose that same gate (the original's cabin SubModel-presence null check) as
    // explicit config, since this class has no visibility into cabin nodes - callers
    // building a vehicle from MMD data (e.g. DynamicRailVehicle3D) set these from label
    // presence.
    class TrainHorns : public TrainPart {
            GDCLASS(TrainHorns, TrainPart)

        protected:
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static void _bind_methods();

            void set_horn_low(bool p_state);
            void set_horn_high(bool p_state);
            void set_whistle(bool p_state);
            // Compatibility entry point for a single bidirectional cabin widget (one physical
            // lever animating -1/0/+1) driving both low and high horn from one signed value,
            // e.g. CabinSwitch's command_set - positive activates the low horn, negative the
            // high horn, zero releases both. Internally routes to the same WarningSignal bits
            // as set_horn_low()/set_horn_high().
            void set_horn(double p_position);

            MAKE_MEMBER_GS_NR(bool, low_horn_enabled, true);
            MAKE_MEMBER_GS_NR(bool, high_horn_enabled, true);
            MAKE_MEMBER_GS_NR(bool, whistle_enabled, true);
    };
} // namespace godot
