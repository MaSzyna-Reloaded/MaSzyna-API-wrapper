#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include "resources/wipers/WiperListItem.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <vector>

namespace godot {
    class VehicleController;

    /* Wraps the FIZ WiperList: section and simulates the wipers.
     *
     * The vendored Mover has no wiper switch or wiper state at all (they were added to the original
     * later: MOVER.h wiperSwitchPos, DynObj.h dWiperPos), so both live in this node. The movement is
     * a port of TDynamicObject::update() (DynObj.cpp:4048-4115), the switch of
     * TTrain::OnCommand_wiperswitchincrease/decrease (Train.cpp:2638-2661). */
    class VehicleWipers : public VehicleComponent {
            GDCLASS(VehicleWipers, VehicleComponent);

        private:
            struct Wiper {
                    double position = 0.0;  // dWiperPos: 0 parked, 1 fully out
                    bool returning = false; // wiperDirection
                    double out_timer = 0.0;
                    double park_timer = 0.0;
                    int working_switch_position = 0;
            };

            static void _bind_methods();
            std::vector<Wiper> wipers;
            int switch_position = 0;
            bool switch_initialized = false;
            void _set_switch_position(int p_position);

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
            void _register_commands() override;
            void _unregister_commands() override;

            
        private:

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            /* Live state, read straight from the backend - nothing is stored. */
            int get_switch_position() const;
            PackedFloat64Array get_sweep_positions() const;

            void switch_increase();
            void switch_decrease();

            MAKE_MEMBER_GS(double, angle, 0.0);
            MAKE_MEMBER_GS(int, default_position, 0);
            // Number of wipers of the vehicle model (the original counts its animated submodels,
            // DynObj.cpp:5842), set by the vehicle factory. 0: the highest wiper the list switches on.
            MAKE_MEMBER_GS(int, wiper_count, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<WiperListItem>, positions)
    };
} // namespace godot
