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


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_WIPERS;
            }

        private:
            static void _bind_methods();
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual int get_switch_position() const = 0;
            virtual PackedFloat64Array get_sweep_positions() const = 0;
            virtual void switch_increase() = 0;
            virtual void switch_decrease() = 0;
            MAKE_MEMBER_GS(double, angle, 0.0);
            MAKE_MEMBER_GS(int, default_position, 0);
            // Number of wipers of the vehicle model (the original counts its animated submodels,
            // DynObj.cpp:5842), set by the vehicle factory. 0: the highest wiper the list switches on.
            MAKE_MEMBER_GS(int, wiper_count, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<WiperListItem>, positions)
    };
} // namespace godot
