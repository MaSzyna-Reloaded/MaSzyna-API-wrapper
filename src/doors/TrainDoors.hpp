#pragma once
#include "../core/TrainPart.hpp"
#include "macros.hpp"

namespace godot {
    class TrainNode;
    class TrainDoors: public TrainPart {
            GDCLASS(TrainDoors, TrainPart)

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            TrainDoors();
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
                    {VOLTAGE_0, 0.0f}, {VOLTAGE_12, 12.0f}, {VOLTAGE_24, 24.0f}, {VOLTAGE_112, 112.0f}};

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

            MAKE_MEMBER_GS_NR(Type, type, Type::TYPE_ROTATE);
            MAKE_MEMBER_GS_NR(Controls, open_method, Controls::CONTROLS_PASSENGER);
            MAKE_MEMBER_GS_NR(Controls, close_method, Controls::CONTROLS_PASSENGER);
            MAKE_MEMBER_GS(float, open_time, -1.0f);
            MAKE_MEMBER_GS(float, open_speed, 1.0f);
            MAKE_MEMBER_GS(float, close_speed, 1.0f);
            MAKE_MEMBER_GS(float, max_shift, 0.5f);
            MAKE_MEMBER_GS_NR(Voltage, voltage, Voltage::VOLTAGE_AUTO);
            MAKE_MEMBER_GS(bool, close_warning, false);
            MAKE_MEMBER_GS(bool, auto_close_warning, false);
            MAKE_MEMBER_GS(float, close_delay, 0.0f);
            MAKE_MEMBER_GS(float, open_delay, 0.0f);
            MAKE_MEMBER_GS(float, open_with_permit, -1.0f);
            MAKE_MEMBER_GS_DIRTY(bool, has_lock, false);
            MAKE_MEMBER_GS(float, max_shift_plug, 0.1f);
            MAKE_MEMBER_GS(Array, permit_list, Array::make(0, 0, 0));
            MAKE_MEMBER_GS(int, permit_list_default, 1);
            MAKE_MEMBER_GS(bool, auto_close_remote, false);
            MAKE_MEMBER_GS(float, auto_close_velocity, -1.0f);
            MAKE_MEMBER_GS(double, platform_max_speed, 0.0);
            MAKE_MEMBER_GS(float, platform_max_shift, 0.0f);
            MAKE_MEMBER_GS_DIRTY(float, platform_speed, 0.5f);
            MAKE_MEMBER_GS(double, mirror_max_shift, 90.0);
            MAKE_MEMBER_GS(double, mirror_close_velocity, 5.0);
            MAKE_MEMBER_GS_DIRTY(bool, permit_required, false);
            MAKE_MEMBER_GS_NR(PermitLight, permit_light_blinking, PermitLight::PERMIT_LIGHT_CONTINUOUS);
            MAKE_MEMBER_GS_NR(PlatformType, platform_type, PlatformType::PLATFORM_TYPE_ROTATE);
            MAKE_MEMBER_GS_NR(Side, side, Side::SIDE_LEFT);
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainDoors::PermitLight)
VARIANT_ENUM_CAST(TrainDoors::PlatformType)
VARIANT_ENUM_CAST(TrainDoors::Side)
VARIANT_ENUM_CAST(TrainDoors::Controls)
VARIANT_ENUM_CAST(TrainDoors::Voltage)
VARIANT_ENUM_CAST(TrainDoors::Type)
