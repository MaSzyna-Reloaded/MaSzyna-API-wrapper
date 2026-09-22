#pragma once
#include "VehicleDoors.hpp"

namespace godot {
    /* VehicleDoors on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleDoors : public VehicleDoors {
            GDCLASS(MoverVehicleDoors, VehicleDoors);

        private:
            static void _bind_methods();
        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_locked() const override;
            bool get_lock_enabled() const override;
            bool get_step_enabled() const override;
            int get_open_control() const override;
            bool get_left_open() const override;
            bool get_left_open_permit() const override;
            bool get_left_local_open() const override;
            bool get_left_remote_open() const override;
            double get_left_position() const override;
            double get_left_position_normalized() const override;
            bool get_left_operating() const override;
            double get_left_step_position() const override;
            bool get_left_step_operating() const override;
            bool get_right_open() const override;
            bool get_right_open_permit() const override;
            bool get_right_local_open() const override;
            bool get_right_remote_open() const override;
            double get_right_position() const override;
            double get_right_position_normalized() const override;
            bool get_right_operating() const override;
            double get_right_step_position() const override;
            bool get_right_step_operating() const override;
            void permit_step(bool p_state) override;
            void permit_doors(Side p_side, bool p_state) override;
            void permit_left_doors(bool p_state) override;
            void permit_right_doors(bool p_state) override;
            void operate_doors(Side p_side, bool p_state) override;
            void operate_left_doors(bool p_state) override;
            void operate_right_doors(bool p_state) override;
            void door_lock(bool p_state) override;
            void door_remote_control(bool p_state) override;
            void next_permit_preset() override;
            void previous_permit_preset() override;
        private:
            const std::map<Voltage, float> voltage_map = {
                    {VOLTAGE_0, 0.0f}, {VOLTAGE_12, 12.0f}, {VOLTAGE_24, 24.0f}, {VOLTAGE_112, 112.0f}};
            const std::map<Type, int> door_type_map = {
                    {TYPE_SHIFT, 1}, {TYPE_ROTATE, 2}, {TYPE_FOLD, 3}, {TYPE_PLUG, 4}};
            const std::map<PlatformType, int> door_platform_type_map = {
                    {PLATFORM_TYPE_SHIFT, 1}, {PLATFORM_TYPE_ROTATE, 2}};
            const std::map<PermitLight, int> door_permit_light_map = {
                    {PERMIT_LIGHT_CONTINUOUS, 0},
                    {PERMIT_LIGHT_FLASHING_ON_PERMISSION_WITH_STEP, 1},
                    {PERMIT_LIGHT_FLASHING_ON_PERMISSION, 2},
                    {PERMIT_LIGHT_FLASHING_ALWAYS, 3}};
            const std::unordered_map<Controls, Maszyna::control_t> door_controls_map = {
                    {Controls::CONTROLS_PASSENGER, Maszyna::control_t::passenger},
                    {Controls::CONTROLS_AUTOMATIC, Maszyna::control_t::autonomous},
                    {Controls::CONTROLS_DRIVER, Maszyna::control_t::driver},
                    {Controls::CONTROLS_CONDUCTOR, Maszyna::control_t::conductor},
                    {Controls::CONTROLS_MIXED, Maszyna::control_t::mixed},
            };
    };
} // namespace godot
