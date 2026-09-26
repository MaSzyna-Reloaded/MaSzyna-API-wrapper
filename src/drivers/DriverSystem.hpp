#pragma once
#include "DriverDelegate.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <queue>
#include <vector>

namespace godot {
    /// RID based registry of drivers - who drives a vehicle. A driver takes the orders a scenario
    /// gives (driver_send_command()) and, through its DriverDelegate, what they mean; it drives the
    /// vehicle as a player does, through the cab. A player at the controls needs no driver.
    ///
    /// A driver acts in moments, as the original's does after its reaction time (TController::
    /// ReactionTime, Driver.cpp:150-158): its delegate asks for the next one
    /// (driver_schedule_update()) and is called when it comes. The time is the simulation's: it
    /// follows MaszynaRuntime's simulation speed and stands still while the runtime is paused.
    class DriverSystem : public Object {
            GDCLASS(DriverSystem, Object)

        public:
            static DriverSystem *get_instance() {
                return Object::cast_to<DriverSystem>(Engine::get_singleton()->get_singleton("DriverSystem"));
            }

        private:
            /// A frame never moves the time on by more (Timer.cpp:78-89, as ScenarioEventServer)
            static constexpr double MAX_FRAME_TIME = 1.0;

            struct DriverData {
                    Ref<DriverDelegate> delegate;
                    RID vehicle;
                    /// The sequence of its scheduled update in the queue, 0 while none is
                    uint64_t update_sequence = 0;
            };

            /// A driver's update at a time; the sequence keeps entries of one time in the order
            /// they were scheduled
            struct UpdateEntry {
                    double time = 0.0;
                    uint64_t sequence = 0;
                    RID driver;

                    bool operator>(const UpdateEntry &p_other) const {
                        return time == p_other.time ? sequence > p_other.sequence : time > p_other.time;
                    }
            };

            HashMap<RID, DriverData> drivers;
            HashMap<RID, RID> drivers_by_vehicle;
            std::priority_queue<UpdateEntry, std::vector<UpdateEntry>, std::greater<UpdateEntry>> updates;
            uint64_t next_sequence = 1;
            double time = 0.0;
            double simulation_speed = 1.0;
            bool processing = false;

            void _on_vehicle_freed(const RID &p_vehicle);
            void _on_simulation_speed_changed();
            void _refresh_processing();
            void _set_processing(bool p_processing);
            void _process_updates();

        protected:
            static void _bind_methods();

        public:
            DriverSystem();
            ~DriverSystem() override;

            RID driver_create();
            void driver_free(const RID &p_driver);
            void driver_attach_delegate(const RID &p_driver, const Ref<DriverDelegate> &p_delegate);
            Ref<DriverDelegate> driver_get_delegate(const RID &p_driver) const;
            /// The RailVehicleServer vehicle the driver drives; one driver a vehicle
            void driver_attach_vehicle(const RID &p_driver, const RID &p_vehicle);
            RID driver_get_vehicle(const RID &p_driver) const;
            RID vehicle_get_driver(const RID &p_vehicle) const;
            /// An order for the driver - a scenario's command with its two values, and where what
            /// sent it stands
            void driver_send_command(
                    const RID &p_driver, const String &p_command, double p_value1, double p_value2,
                    const Vector3 &p_position = Vector3());
            /// The driver's delegate is updated in p_seconds of simulated time; a later call
            /// replaces the one pending
            void driver_schedule_update(const RID &p_driver, double p_seconds);
    };
} // namespace godot
