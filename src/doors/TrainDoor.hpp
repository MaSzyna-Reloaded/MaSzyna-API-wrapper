#pragma once
#include "../core/TrainPart.hpp"

namespace godot {
    class TrainController;
    class TrainDoor : public TrainPart {
            GDCLASS(TrainDoor, TrainPart)
        protected:
            static void _bind_methods();

            /**
             * Door opening control methods:
             * Passenger - default type if not defined; doors are closed manually, ignoring any remote commands
             * AutomaticCtrl - doors operate automatically
             * DriverCtrl - doors are controlled by the train driver, and respond only to remote commands
             * Conductor - doors are controlled by the train manager, and respond only to remote commands
             * Mixed - doors can be closed both manually and remotely
             */

            /**
             * Door closing control method
             */
            int close_control = 0;

            /**
             * Door opening control method
             */
            int open_control = 0;

            /**
             * Time period for which door would stay opened
             */
            double door_stay_open = 0.0;

            /**
             * Speed of opening the doors
             */
            double open_speed = 0.0;

            /**
             * Speed of closing the door
             */
            double close_speed = 0.0;

            /**
             * The width (or angle) of the door fully opening
             */
            double door_max_shift = 0.0;

            /**
             * The type of door (opening method)
             */
            int door_open_method = 0.0;

            /**
             * Low voltage circuit voltage required for door control
             */
            double door_voltage = 0.0;

            /**
             * Buzzer before closing the door
             */
            bool door_closure_warning = false;

            /**
             * When you press the door closing button, a buzzer is activated, when you release it, the door closes
             */
            bool auto_door_closure_warning = false;

            /**
             * Door closing delay, in seconds
             */
            double door_close_delay = 0.0;

            /**
             * Door opening delay, in seconds
             */
            double door_open_delay = 0.0;

            /**
             * Opening a train door by holding the impulse opening permission button, in seconds of holding the button
             */
            double door_open_with_permit = 0.0;

            /**
             * Is the door blocked
             */
            bool door_blocked = false;

            /**
             * The amount of rebound for Plug type doors (rebound-sliding), in meters
             */
            double door_max_shift_plug = 0.0;

            /**
             * Door programmer configuration. Number in the range of 0-3 where 0=no permissions, 1=allows left
             * door operation, 2=right door, 3=all
             */
            int door_permit_list = door_permit_list_default;

            /**
             * The default knob position is from the set defined by the door_permit_list entry, positions are numbered
             * from 1
             */
            int door_permit_list_default = 0;

            /**
             * Automatic closing of centrally opened doors after time has elapsed
             */
            bool door_auto_close_remote = false;

            /**
             * The speed at which the door automatically closes, set by default to -1, i.e. no automatic closing.
             */
            double door_auto_close_vel = -1;

            /**
             * Docs do not describe this properly. Need to figure this out
             */
            double platform_max_speed = 0.0;

            /**
             * Offset value in meters or rotation angle for a fully extended step
             */
            double platform_max_shift = 0.0;

            /**
             * The speed of the animation step, where 1.0 corresponds to an animation lasting one second, a value of 0.5
             * to two seconds, etc.
             */
            double platform_speed = 0.0;

            /**
             * Platform animation type
             * 0 - Shift
             * 1 - Rot
             */
            int platform_shift_method = 0;

            /**
             * Rotation angle for fully extended mirror
             */
            double mirror_max_shift = 0.0;

            /**
             * The speed of the traction vehicle at which the external mirrors are automatically closed
             */
            double mirror_vel_close = 0.0;

            /**
             * Opening doors by passengers requires the train driver's consent
             */
            bool door_needs_permit = false;

            /**
             * Flashing of the light indicating permission to open the door on the side:
             * 0 - continuous light
             * 1 - continuous light when any door is open or any step is extended; flashing only when permission is
             * given 2 - continuous light when any door is open; flashing when permission is given; the step extension
             * status is irrelevant 3 - flashing regardless of the door open status and step extension status
             */
            int door_permit_light_blinking = 0;

        public:
            void set_close_control(int p_value);
            int get_close_control() const;
            void set_open_control(int p_value);
            int get_open_control() const;
            void set_door_stay_open(double p_value);
            double get_door_stay_open() const;
            void set_open_speed(double p_value);
            double get_open_speed() const;
            void set_close_speed(double p_value);
            double get_close_speed() const;
    };
} // namespace godot
