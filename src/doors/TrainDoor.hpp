#pragma once
#include "../core/TrainPart.hpp"

namespace godot {
    class TrainController;
    class TrainDoor final : public TrainPart {
            GDCLASS(TrainDoor, TrainPart)

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;

        public:
            enum PermitLights {
                PERMIT_LIGHT_CONTINUOUS_LIGHT,
                PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP,
                PERMIT_LIGHT_FLASHING_ON_PERMISSION,
                PERMIT_LIGHT_FLASHING_ALWAYS
            };

            enum PlatformAnimationType { PLATFORM_ANIMATION_TYPE_SHIFT, PLATFORM_ANIMATION_TYPE_ROTATE };

            enum DoorSide { DOOR_SIDE_RIGHT, DOOR_SIDE_LEFT };

            enum DoorState { DOOR_STATE_CLOSE, DOOR_STATE_OPEN };

            enum NotificationRange { NOTIFICATION_RANGE_LOCAL, NOTIFICATION_RANGE_UNIT, NOTIFICATION_RANGE_CONSIST };

            enum DoorControls {
                DOOR_CONTROLS_PASSENGER = control_t::passenger,
                DOOR_CONTROLS_AUTOMATIC = control_t::autonomous,
                DOOR_CONTROLS_DRIVER = control_t::driver,
                DOOR_CONTROLS_CONDUCTOR = control_t::conductor,
                DOOR_CONTROLS_MIXED = control_t::mixed
            };

            enum DoorVoltage { DOOR_VOLTAGE_0 = 0, DOOR_VOLTAGE_12 = 12, DOOR_VOLTAGE_24 = 24, DOOR_VOLTAGE_112 = 112 };

            enum DoorType { DOOR_TYPE_SHIFT = 1, DOOR_TYPE_ROTATE = 2, DOOR_TYPE_FOLD = 3, DOOR_TYPE_PLUG = 4 };

            static void _bind_methods();
            void _on_command_received(const String &command, const Variant &p1, const Variant &p2) override;
            void set_door_type(DoorType p_door_type);
            DoorType get_door_type() const;
            void set_door_open_time(float p_value);
            float get_door_open_time() const;
            void set_open_speed(float p_value);
            float get_open_speed() const;
            void set_close_speed(float p_value);
            float get_close_speed() const;
            void set_door_max_shift(float p_max_shift);
            float get_door_max_shift() const;
            void set_door_open_method(DoorControls p_open_method);
            DoorControls get_door_open_method() const;
            void set_door_close_method(DoorControls p_close_method);
            DoorControls get_door_close_method() const;
            void set_door_voltage(DoorVoltage p_voltage);
            DoorVoltage get_door_voltage() const;
            void set_door_close_warning(bool p_close_warning);
            bool get_door_close_warning() const;
            void set_auto_door_close_warning(bool p_auto_close_warning);
            bool get_auto_door_close_warning() const;
            void set_door_open_delay(float p_open_delay);
            float get_door_open_delay() const;
            void set_door_close_delay(float p_close_delay);
            float get_door_close_delay() const;
            void set_door_open_with_permit(float p_holding_time);
            float get_door_open_with_permit() const;
            void set_door_blocked(bool p_blocked);
            bool get_door_blocked() const;
            void set_door_max_shift_plug(float p_max_shift_plug);
            float get_door_max_shift_plug() const;
            void set_door_permit_list(const Array &p_permit_list);
            Array get_door_permit_list() const;
            void set_door_permit_list_default(int p_permit_list_default);
            int get_door_permit_list_default() const;
            void set_door_auto_close_remote(bool p_auto_close);
            bool get_door_auto_close_remote() const;
            void set_door_auto_close_velocity(float p_vel);
            float get_door_auto_close_velocity() const;
            void set_door_auto_close_enabled(bool p_enabled);
            bool get_door_auto_close_enabled() const;
            void set_door_platform_max_speed(double p_max_speed);
            double get_door_platform_max_speed() const;
            void set_door_platform_max_shift(float p_max_shift);
            float get_door_platform_max_shift() const;
            void set_door_platform_speed(float p_speed);
            float get_door_platform_speed() const;
            void set_platform_open_method(PlatformAnimationType p_shift_method);
            int get_platform_open_method() const;
            void set_mirror_max_shift(double p_max_shift);
            double get_mirror_max_shift() const;
            void set_mirror_vel_close(double p_vel_close);
            double get_mirror_vel_close() const;
            void set_door_permit_required(bool p_permit_required);
            bool get_door_permit_required() const;
            void set_door_permit_light_blinking(PermitLights p_blinking_mode);
            int get_door_permit_light_blinking() const;

            DoorType door_type = DoorType::DOOR_TYPE_ROTATE;
            /**
             * The type of door (opening method)
             */
            DoorControls door_open_method = DoorControls::DOOR_CONTROLS_PASSENGER;

            /**
             * The type of door (closing method)
             */
            DoorControls door_close_method = DoorControls::DOOR_CONTROLS_PASSENGER;

            /**
             * Time period for which door would stay opened
             */
            float door_open_time = 0.0;

            /**
             * Speed of opening the doors
             */
            float open_speed = 0.0;

            /**
             * Speed of closing the door
             */
            float close_speed = 0.0;

            /**
             * The width (or angle) of the door fully opening
             */
            float door_max_shift = 0.0;

            /**
             * Low voltage circuit voltage required for door control.
             */
            DoorVoltage door_voltage = DoorVoltage::DOOR_VOLTAGE_0;

            /**
             * Buzzer before closing the door
             */
            bool door_close_warning = false;

            /**
             * When you press the door closing button, a buzzer is activated, when you release it, the door closes
             */
            bool auto_door_close_warning = false;

            /**
             * Door closing delay, in seconds
             */
            float door_close_delay = 0.0;

            /**
             * Door opening delay, in seconds
             */
            float door_open_delay = 0.0;

            /**
             * Opening a train door by holding the impulse opening permission button, in seconds of holding the button
             */
            float door_open_with_permit = 0.0;

            /**
             * Is the door blocked
             */
            bool door_blocked = false;

            /**
             * The amount of rebound for Plug type doors (rebound-sliding), in meters
             */
            float door_max_shift_plug = 0.0;

            /**
             * Door programmer configuration. Number in the range of 0-3 where 0=no permissions, 1=allows left
             * door operation, 2=right door, 3=all
             */
            Array door_permit_list = Array({"0", "0", "0"});

            /**
             * The default knob position is from the set defined by the door_permit_list entry; positions are numbered
             * from 1
             */
            int door_permit_list_default = 1;

            /**
             * Automatic closing of centrally opened doors after time has elapsed
             */
            bool door_auto_close_remote = false;

            /**
             * Enable the velocity
             */
            bool door_auto_close_enabled = false;

            /**
             * The speed at which the door automatically closes
             */
            float door_auto_close_velocity = 0;

            /**
             * Docs do not describe this properly. Need to figure this out
             */
            double platform_max_speed = 0.0;

            /**
             * Offset value in meters or rotation angle for a fully extended step
             */
            float platform_max_shift = 0.0;

            /**
             * The speed of the animation step, where 1.0 corresponds to an animation lasting one second, a value of 0.5
             * to two seconds, etc.
             */
            float platform_speed = 0.0;

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
            bool door_permit_required = false;

            /**
             * Blinking permit light
             */
            PermitLights door_permit_light_blinking = PermitLights::PERMIT_LIGHT_CONTINUOUS_LIGHT;

            /**
             * Platform animation type
             */
            PlatformAnimationType platform_open_method = PlatformAnimationType::PLATFORM_ANIMATION_TYPE_SHIFT;

            /**
             * Describes the side where the doors are placed
             */
            DoorSide door_side = DoorSide::DOOR_SIDE_LEFT;

            /**
             * @TODO: Maybe move to TrainController since it seems to be more generic property? It is not used there
             * anyways
             */
            NotificationRange notification_range = NotificationRange::NOTIFICATION_RANGE_UNIT;
            TrainDoor();
            ~TrainDoor() override = default;
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainDoor::PermitLights)
VARIANT_ENUM_CAST(TrainDoor::PlatformAnimationType)
VARIANT_ENUM_CAST(TrainDoor::DoorSide)
VARIANT_ENUM_CAST(TrainDoor::NotificationRange)
VARIANT_ENUM_CAST(TrainDoor::DoorState)
VARIANT_ENUM_CAST(TrainDoor::DoorControls)
VARIANT_ENUM_CAST(TrainDoor::DoorVoltage)
VARIANT_ENUM_CAST(TrainDoor::DoorType)
