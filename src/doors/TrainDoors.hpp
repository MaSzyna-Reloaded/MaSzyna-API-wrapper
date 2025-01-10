#pragma once
#include "../core/TrainPart.hpp"

namespace godot {
    class TrainController;
    class TrainDoors: public TrainPart {
            GDCLASS(TrainDoors, TrainPart)

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            enum PermitLight {
                PERMIT_LIGHT_CONTINUOUS,
                PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP,
                PERMIT_LIGHT_FLASHING_ON_PERMISSION,
                PERMIT_LIGHT_FLASHING_ALWAYS
            };

            enum Side { SIDE_RIGHT, SIDE_LEFT };
            enum Voltage {
                VOLTAGE_AUTO,
                VOLTAGE_0,
                VOLTAGE_12,
                VOLTAGE_24,
                VOLTAGE_112,
            };
            enum Type {
                TYPE_SHIFT,
                TYPE_ROTATE,
                TYPE_FOLD,
                TYPE_PLUG,
            };
            enum PlatformType { PLATFORM_TYPE_SHIFT, PLATFORM_TYPE_ROTATE };

            enum Controls {
                CONTROLS_PASSENGER,
                CONTROLS_AUTOMATIC,
                CONTROLS_DRIVER,
                CONTROLS_CONDUCTOR,
                CONTROLS_MIXED,
            };

            static void _bind_methods();
            void set_type(Type p_type);
            Type get_type() const;
            void set_open_time(float p_value);
            float get_open_time() const;
            void set_open_speed(float p_value);
            float get_open_speed() const;
            void set_close_speed(float p_value);
            float get_close_speed() const;
            void set_max_shift(float p_max_shift);
            float get_max_shift() const;
            void set_open_method(Controls p_open_method);
            Controls get_open_method() const;
            void set_close_method(Controls p_close_method);
            Controls get_close_method() const;
            void set_voltage(Voltage p_voltage);
            Voltage get_voltage() const;
            void set_close_warning(bool p_close_warning);
            bool get_close_warning() const;
            void set_auto_close_warning(bool p_auto_close_warning);
            bool get_auto_close_warning() const;
            void set_open_delay(float p_open_delay);
            float get_open_delay() const;
            void set_close_delay(float p_close_delay);
            float get_close_delay() const;
            void set_open_with_permit(float p_holding_time);
            float get_open_with_permit() const;
            void set_has_lock(bool p_has_lock);
            bool get_has_lock() const;
            void set_max_shift_plug(float p_max_shift_plug);
            float get_max_shift_plug() const;
            void set_permit_list(const Array &p_permit_list);
            Array get_permit_list() const;
            void set_permit_list_default(int p_permit_list_default);
            int get_permit_list_default() const;
            void set_auto_close_remote(bool p_auto_close);
            bool get_auto_close_remote() const;
            void set_auto_close_velocity(float p_vel);
            float get_auto_close_velocity() const;
            void set_platform_max_speed(double p_max_speed);
            double get_platform_max_speed() const;
            void set_platform_max_shift(float p_max_shift);
            float get_platform_max_shift() const;
            void set_platform_speed(float p_speed);
            float get_platform_speed() const;
            void set_platform_type(PlatformType p_type);
            PlatformType get_platform_type() const;
            void set_mirror_max_shift(double p_max_shift);
            double get_mirror_max_shift() const;
            void set_mirror_close_velocity(double p_close_velocity);
            double get_mirror_close_velocity() const;
            void set_permit_required(bool p_permit_required);
            bool get_permit_required() const;
            void set_permit_light_blinking(PermitLight p_blinking_mode);
            PermitLight get_permit_light_blinking() const;

            void permit_step(bool p_state);
            void permit_doors(Side p_side, bool p_state);
            void permit_left_doors(bool p_state);
            void permit_right_doors(bool p_state);
            void operate_doors(Side p_side, bool p_state);
            void operate_left_doors(bool p_state);
            void operate_right_doors(bool p_state);
            void door_lock(bool p_state);
            void door_remote_control(bool p_state);
            void next_permit_preset();
            void previous_permit_preset();

        private:
            // Maszyna Mover has no consts for voltages
            const std::map<Voltage, float> voltageMap = {
                    {VOLTAGE_0, 0}, {VOLTAGE_12, 12}, {VOLTAGE_24, 24}, {VOLTAGE_112, 112}};

            // Maszyna Mover has no consts for door types
            const std::map<Type, int> doorTypeMap = {{TYPE_SHIFT, 1}, {TYPE_ROTATE, 2}, {TYPE_FOLD, 3}, {TYPE_PLUG, 4}};
            //
            // Maszyna Mover has no consts for door platform types
            const std::map<PlatformType, int> doorPlatformTypeMap = {
                    {PLATFORM_TYPE_SHIFT, 1}, {PLATFORM_TYPE_ROTATE, 2}};

            // Maszyna Mover has no consts for permit lights
            const std::map<PermitLight, int> doorPermitLightMap = {
                    {PERMIT_LIGHT_CONTINUOUS, 0},
                    {PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP, 1},
                    {PERMIT_LIGHT_FLASHING_ON_PERMISSION, 2},
                    {PERMIT_LIGHT_FLASHING_ALWAYS, 3}};

            const std::unordered_map<Controls, Maszyna::control_t> doorControlsMap = {
                    {Controls::CONTROLS_PASSENGER, Maszyna::control_t::passenger},
                    {Controls::CONTROLS_AUTOMATIC, Maszyna::control_t::autonomous},
                    {Controls::CONTROLS_DRIVER, Maszyna::control_t::driver},
                    {Controls::CONTROLS_CONDUCTOR, Maszyna::control_t::conductor},
                    {Controls::CONTROLS_MIXED, Maszyna::control_t::mixed},
            };

            /**
             * Type of the door. Default value from internal mover
             *
             */
            Type type = Type::TYPE_ROTATE;
            /**
             * The type of door (opening method). Default value from internal mover
             */
            Controls open_method = Controls::CONTROLS_PASSENGER;

            /**
             * The type of door (closing method). Default value from internal mover
             */
            Controls close_method = Controls::CONTROLS_PASSENGER;

            /**
             * Time period for which door would stay opened. Default value from internal mover
             */
            float open_time = -1.0f;

            /**
             * Speed of opening the doors. Default value from internal mover
             */
            float open_speed = 1.0f;

            /**
             * Speed of closing the door. Default value from internal mover
             */
            float close_speed = 1.0f;

            /**
             * The width (or angle) of the door fully opening. Default value from internal mover
             */
            float max_shift = 0.5f;

            /**
             * Low voltage circuit voltage required for door control. Default value from internal mover
             */
            Voltage voltage = Voltage::VOLTAGE_AUTO;

            /**
             * Buzzer before closing the door. Default value from internal mover
             */
            bool close_warning = false;

            /**
             * When you press the door closing button, a buzzer is activated, when you release it, the door closes.
             * Default value from internal mover
             */
            bool auto_close_warning = false;

            /**
             * TrainDoors closing delay, in seconds. Default value from internal mover
             */
            float close_delay = 0.0;

            /**
             * TrainDoors opening delay, in seconds. Default value from internal mover
             */
            float open_delay = 0.0;

            /**
             * Opening a train door by holding the impulse opening permission button, in seconds of holding the button
             */
            float open_with_permit = -1.0f;

            /**
             * Indicates whether the train door has lock. Defaults to false.
             */
            bool has_lock = false;

            /**
             * The amount of rebound for Plug type doors (rebound-sliding), in meters. Default value from internal mover
             */
            float max_shift_plug = 0.1f;

            /**
             * TrainDoors programmer configuration. Number in the range of 0-3 where 0=no permissions, 1=allows left
             * door operation, 2=right door, 3=all
             */
            Array permit_list = Array({"0", "0", "0"});

            /**
             * The default knob position is from the set defined by the permit_list entry; positions are numbered
             * from 1. Default value from internal mover
             */
            int permit_list_default = 1;

            /**
             * Automatic closing of centrally opened doors after time has elapsed
             */
            bool auto_close_remote = false;

            /**
             * The speed at which the door automatically closes. Default value from internal mover
             */
            float auto_close_velocity = -1.0f;

            /**
             * Docs do not describe this properly. Need to figure this out
             */
            double platform_max_speed = 0.0;

            /**
             * Offset value in meters or rotation angle for a fully extended step. Default value from internal mover
             */
            float platform_max_shift = 0.0f;

            /**
             * The speed of the animation step, where 1.0 corresponds to an animation lasting one second, a value of 0.5
             * to two seconds, etc. Default value from internal mover
             */
            float platform_speed = 0.5f;

            /**
             * Rotation angle for fully extended mirror
             */
            double mirror_max_shift = 90.0;

            /**
             * The speed of the traction vehicle at which the external mirrors are automatically closed
             */
            double mirror_close_velocity = 5.0;

            /**
             * Opening doors by passengers requires the train driver's consent. Default value from internal mover
             */
            bool permit_required = false;

            /**
             * Blinking permit light. Default value from internal mover
             */
            PermitLight permit_light_blinking = PermitLight::PERMIT_LIGHT_CONTINUOUS;

            PlatformType platform_type = PlatformType::PLATFORM_TYPE_ROTATE;

            /**
             * Describes the side where the doors are placed
             */
            Side side = Side::SIDE_LEFT;
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainDoors::PermitLight)
VARIANT_ENUM_CAST(TrainDoors::PlatformType)
VARIANT_ENUM_CAST(TrainDoors::Side)
VARIANT_ENUM_CAST(TrainDoors::Controls)
VARIANT_ENUM_CAST(TrainDoors::Voltage)
VARIANT_ENUM_CAST(TrainDoors::Type)
