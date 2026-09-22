#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"

namespace godot {
    class VehicleController;
    class VehicleDoors : public VehicleComponent {
            GDCLASS(VehicleDoors, VehicleComponent)


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_DOORS;
            }

        private:
            static void _bind_methods();
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual bool get_locked() const = 0;
            virtual bool get_lock_enabled() const = 0;
            virtual bool get_step_enabled() const = 0;
            virtual int get_open_control() const = 0;
            virtual bool get_left_open() const = 0;
            virtual bool get_left_open_permit() const = 0;
            virtual bool get_left_local_open() const = 0;
            virtual bool get_left_remote_open() const = 0;
            virtual double get_left_position() const = 0;
            virtual double get_left_position_normalized() const = 0;
            virtual bool get_left_operating() const = 0;
            virtual double get_left_step_position() const = 0;
            virtual bool get_left_step_operating() const = 0;
            virtual bool get_right_open() const = 0;
            virtual bool get_right_open_permit() const = 0;
            virtual bool get_right_local_open() const = 0;
            virtual bool get_right_remote_open() const = 0;
            virtual double get_right_position() const = 0;
            virtual double get_right_position_normalized() const = 0;
            virtual bool get_right_operating() const = 0;
            virtual double get_right_step_position() const = 0;
            virtual bool get_right_step_operating() const = 0;
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
            virtual void permit_step(bool p_state) = 0;
            virtual void permit_doors(Side p_side, bool p_state) = 0;
            virtual void permit_left_doors(bool p_state) = 0;
            virtual void permit_right_doors(bool p_state) = 0;
            virtual void operate_doors(Side p_side, bool p_state) = 0;
            virtual void operate_left_doors(bool p_state) = 0;
            virtual void operate_right_doors(bool p_state) = 0;
            virtual void door_lock(bool p_state) = 0;
            virtual void door_remote_control(bool p_state) = 0;
            virtual void next_permit_preset() = 0;
            virtual void previous_permit_preset() = 0;
        private:
            MAKE_MEMBER_GS_NR(Type, type, Type::TYPE_ROTATE);
            MAKE_MEMBER_GS_NR(Controls, open_method, Controls::CONTROLS_PASSENGER);
            MAKE_MEMBER_GS_NR(Controls, close_method, Controls::CONTROLS_PASSENGER);
            MAKE_MEMBER_GS(float, open_time, -1.0f);
            MAKE_MEMBER_GS(float, open_speed, 1.0f);
            MAKE_MEMBER_GS(float, close_speed, 1.0f);
            MAKE_MEMBER_GS(float, max_shift, 0.5f);
            MAKE_MEMBER_GS_NR(Voltage, voltage, Voltage::VOLTAGE_AUTO);
            MAKE_MEMBER_GS(bool, close_warning, false);
            MAKE_MEMBER_GS(bool, close_auto_close_warning, false);
            MAKE_MEMBER_GS(float, close_delay, 0.0f);
            MAKE_MEMBER_GS(float, open_delay, 0.0f);
            MAKE_MEMBER_GS(float, open_with_permit, -1.0f);
            MAKE_MEMBER_GS(bool, has_lock, false);
            MAKE_MEMBER_GS(float, max_shift_plug, 0.1f);
            MAKE_MEMBER_GS(Array, permit_list, Array::make(0, 0, 0));
            MAKE_MEMBER_GS(int, permit_default, 1);
            MAKE_MEMBER_GS(bool, close_auto_close_remote, false);
            MAKE_MEMBER_GS(float, close_auto_close_velocity, -1.0f);
            MAKE_MEMBER_GS(double, platform_max_speed, 0.0);
            MAKE_MEMBER_GS(float, platform_max_shift, 0.0f);
            MAKE_MEMBER_GS(float, platform_speed, 0.5f);
            MAKE_MEMBER_GS(double, mirror_max_shift, 90.0);
            MAKE_MEMBER_GS(double, mirror_close_velocity, 5.0);
            MAKE_MEMBER_GS(bool, permit_required, false);
            MAKE_MEMBER_GS_NR(PermitLight, permit_light_blinking, PermitLight::PERMIT_LIGHT_CONTINUOUS);
            MAKE_MEMBER_GS_NR(PlatformType, platform_type, PlatformType::PLATFORM_TYPE_ROTATE);
            MAKE_MEMBER_GS_NR(Side, side, Side::SIDE_LEFT);
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleDoors::PermitLight)
VARIANT_ENUM_CAST(VehicleDoors::PlatformType)
VARIANT_ENUM_CAST(VehicleDoors::Side)
VARIANT_ENUM_CAST(VehicleDoors::Controls)
VARIANT_ENUM_CAST(VehicleDoors::Voltage)
VARIANT_ENUM_CAST(VehicleDoors::Type)
