#pragma once
#include "DriverDelegate.hpp"
#include "vehicles/base/VehiclePersonRole.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <queue>
#include <vector>

namespace godot {
    /// The AI drivers - PersonServer persons given a DriverDelegate - by the person's handle. A
    /// driver takes the orders a scenario gives (driver_send_command()) and, through its delegate,
    /// what they mean; it drives the vehicle it sits in (VehicleServer) as a player does, through
    /// the cab, while it sits there in the driver's role. A player at the controls needs no driver.
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
            std::priority_queue<UpdateEntry, std::vector<UpdateEntry>, std::greater<UpdateEntry>> updates;
            uint64_t next_sequence = 1;
            /// While something is scheduled it holds the runtime's clock and runs as it advances
            bool processing = false;

            void _on_person_freed(const RID &p_person);
            void _on_cabin_person_role_changed(const RID &p_cabin, const RID &p_person, VehiclePersonRole::Role p_role);
            void _detach(const RID &p_driver);
            void _set_processing(bool p_processing);
            void _process_updates(double p_seconds);

        protected:
            static void _bind_methods();

        public:
            static const char *driver_timetable_changed_signal;
            /// The person is no driver any more (driver: RID) - its delegate was taken, or the
            /// person freed
            static const char *driver_freed_signal;
            /// The person is a driver now (driver: RID) - it was given its first delegate
            static const char *driver_attached_signal;

            DriverSystem();
            ~DriverSystem() override;

            /// Every driver there is
            TypedArray<RID> driver_get_rids() const;
            /// The person becomes a driver thinking with p_delegate; null makes it none
            void driver_attach_delegate(const RID &p_driver, const Ref<DriverDelegate> &p_delegate);
            Ref<DriverDelegate> driver_get_delegate(const RID &p_driver) const;
            /// The driver aboard the vehicle, in whatever role (the original's Mechanik); RID() for none
            RID vehicle_get_driver(const RID &p_vehicle) const;
            /// An order for the driver - a scenario's command with its two values, and where what
            /// sent it stands
            void driver_send_command(
                    const RID &p_driver, const String &p_command, double p_value1, double p_value2,
                    const Vector3 &p_position = Vector3());
            /// The driver's delegate is updated in p_seconds of simulated time; a later call
            /// replaces the one pending
            void driver_schedule_update(const RID &p_driver, double p_seconds);
            /// Whether the vehicle's driver sits at its controls (AIControllFlag) - not while it
            /// rides along in a cab a player drives from; it still takes its orders then, but
            /// touches no control. False for a vehicle without a driver.
            bool vehicle_is_control_active(const RID &p_vehicle) const;
            /// The driver's timetable and its progress (DriverDelegate::get_timetable_state()); empty
            /// without a delegate
            Dictionary driver_get_timetable_state(const RID &p_driver) const;
            /// The seconds from p_hours (the time of day) to the departure of the vehicle's train
            /// (DriverDelegate::get_seconds_until_departure()): by the timetable of its own
            /// driver, else of the first driver of its trainset with one (Mechanik, else ctOwner,
            /// Event.cpp:2431-2435); 0 for a train without a timetable
            double vehicle_get_seconds_until_departure(const RID &p_vehicle, double p_hours) const;
            /// The driver's delegate reports that its timetable, or how far it got through it, has
            /// changed - announced as driver_timetable_changed
            void driver_report_timetable_changed(const RID &p_driver);
            /// What the driver keeps (DriverDelegate::get_state()); empty without a delegate
            Dictionary driver_get_state(const RID &p_driver) const;
    };
} // namespace godot
