#pragma once
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /// The player - the one owner of what the player drives (the original's simulation::Train).
    /// Taking a vehicle over and letting it go happen here, by the vehicles' RailVehicleServer
    /// handles, whoever asks - the keys, the HUD, scripts - and are announced; the player's node
    /// follows the signal into the cab and out. It knows nothing of the view: PlayerCameraServer
    /// follows player_vehicle_entered into the cab.
    class PlayerServer : public Object {
            GDCLASS(PlayerServer, Object)

        public:
            /// What the player drives changed (vehicle: RID, previous: RID; invalid for none)
            static const char *player_vehicle_changed_signal;
            /// The player took a vehicle over (vehicle: RID) - also the one it already drives
            static const char *player_vehicle_entered_signal;

            static PlayerServer *get_instance() {
                return Object::cast_to<PlayerServer>(Engine::get_singleton()->get_singleton("PlayerServer"));
            }

        private:
            RID vehicle;

            void _set_vehicle(const RID &p_vehicle);
            void _on_vehicle_freed(const RID &p_vehicle);

        protected:
            static void _bind_methods();

        public:
            PlayerServer();

            /// The player takes the vehicle over: its driver stops driving it, the one left - of
            /// another trainset - drives its own again (TakeControl(), Driver.cpp:5700). Refused
            /// for a vehicle without a node, whose cab there is nothing to sit in. The vehicle
            /// already driven is only taken back from its driver (drivermode.cpp:258-267).
            void player_enter_vehicle(const RID &p_vehicle);
            /// The player lets the trainset go: its driver drives it again (simulation.cpp:257-270)
            void player_leave_vehicle();
            /// What the player drives, an invalid RID for none
            RID player_get_vehicle() const;
    };
} // namespace godot
