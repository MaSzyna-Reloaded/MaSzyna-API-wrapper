#pragma once
#include "DriverDelegate.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <queue>
#include <vector>

namespace godot {
    /// RID based registry of drivers - who drives a vehicle. A driver takes the orders a scenario
    /// gives (driver_send_command()) and, through its DriverDelegate, what they mean; it drives the
    /// vehicle as a player does, through the cab. A player at the controls needs no driver.
    ///
    /// A driver acts in moments, as the original's does after its reaction time (TController::
    /// ReactionTime, Driver.cpp:150-158): its delegate asks for the next one
    /// (driver_schedule_update()) and is called when it comes. The time is SimulationServer's
    /// simulation time, which the physics and the events read too.
    class DriverSystem : public Object {
            GDCLASS(DriverSystem, Object)

        public:
            static DriverSystem *get_instance() {
                return Object::cast_to<DriverSystem>(Engine::get_singleton()->get_singleton("DriverSystem"));
            }

        private:
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
            /// Vehicles a player drives - kept by the vehicle, not by its driver: a player may take
            /// the cab before the vehicle's driver is created, and that driver starts not driving
            HashSet<RID> player_controlled_vehicles;
            std::priority_queue<UpdateEntry, std::vector<UpdateEntry>, std::greater<UpdateEntry>> updates;
            uint64_t next_sequence = 1;
            /// While something is scheduled it holds the runtime's clock and runs as it advances
            bool processing = false;

            void _on_vehicle_freed(const RID &p_vehicle);
            void _report_driven(const RID &p_vehicle, bool p_was_driven);
            void _set_processing(bool p_processing);
            void _process_updates(double p_seconds);

        protected:
            static void _bind_methods();

        public:
            static const char *driver_timetable_changed_signal;
            static const char *driver_vehicle_attached_signal;
            static const char *driver_freed_signal;
            static const char *vehicle_driven_changed_signal;

            DriverSystem();
            ~DriverSystem() override;

            RID driver_create();
            /// Every driver there is
            TypedArray<RID> driver_get_rids() const;
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
            /// Whether the vehicle's driver drives it - off while a player drives it
            /// (MaszynaPlayer), also when it gets its driver only later; the driver
            /// still takes its orders then, but touches no control. False for a vehicle without a
            /// driver.
            void vehicle_set_control_active(const RID &p_vehicle, bool p_active);
            bool vehicle_is_control_active(const RID &p_vehicle) const;
            /// Whether somebody drives the vehicle - its driver or a player - announced as
            /// vehicle_driven_changed when it changes. Only a driven vehicle has a cab at work, as
            /// the original keeps a TTrain only for a driven train.
            bool vehicle_is_driven(const RID &p_vehicle) const;
            /// The driver's timetable and its progress (DriverDelegate::get_timetable_state()); empty
            /// without a delegate
            Dictionary driver_get_timetable_state(const RID &p_driver) const;
            /// The driver's delegate reports that its timetable, or how far it got through it, has
            /// changed - announced as driver_timetable_changed
            void driver_report_timetable_changed(const RID &p_driver);
            /// What the driver keeps (DriverDelegate::get_state()); empty without a delegate
            Dictionary driver_get_state(const RID &p_driver) const;
    };
} // namespace godot
