#pragma once
#include "ScenarioEventAction.hpp"

namespace godot {
    /// The original's `putvalues` and `getvalues` events (Event.cpp:577-844): a command for the
    /// vehicle that queued the event - its own, or read from a memory when the event runs. What the
    /// vehicle itself does with one (TMoverParameters::RunCommand(), Mover.cpp:12187) is sent as its
    /// component's command: `CabSignal` (the cab signal magnet) and `Emergency_brake` (Radio-Stop).
    /// The driver's orders (TController::PutCommand(), Driver.cpp:4468-4906) and the rest are not
    /// ported (TODO.md).
    class MaszynaLegacyVehicleCommandAction : public ScenarioEventAction {
            GDCLASS(MaszynaLegacyVehicleCommandAction, ScenarioEventAction)

        public:
            /// The scenery's commands a vehicle acts on (Mover.cpp:12624, 12649)
            static constexpr const char *CAB_SIGNAL = "CabSignal";
            static constexpr const char *EMERGENCY_BRAKE = "Emergency_brake";

        private:
            String command;
            double value1 = 0.0;
            RID source;

        protected:
            static void _bind_methods();

            void run(const RID &p_event, const RID &p_activator) override;

        public:
            void set_command(const String &p_command);
            String get_command() const;
            void set_value1(double p_value1);
            double get_value1() const;
            /// `getvalues`: the memory whose text and values are the command, read when the event
            /// runs; empty for the action's own
            void set_source(const RID &p_source);
            RID get_source() const;
    };
} // namespace godot
