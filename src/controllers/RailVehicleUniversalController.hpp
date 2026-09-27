#pragma once
#include "../core/RailVehicleComponent.hpp"
#include "macros.hpp"
#include "resources/controllers/UniversalControllerListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class RailVehicleUniversalController : public RailVehicleComponent {
            GDCLASS(RailVehicleUniversalController, RailVehicleComponent);


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_UNIVERSAL_CONTROLLER;
            }

        private:
            static void _bind_methods();

        public:
            MAKE_MEMBER_GS(bool, integrated_brake_pn, false);
            MAKE_MEMBER_GS(bool, integrated_brake, false);
            /* IntegratedLocBrake: the controller's braking positions work the local brake */
            MAKE_MEMBER_GS(bool, integrated_local_brake, false);
            MAKE_MEMBER_GS(int, selector_position, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<UniversalControllerListItem>, positions)
    };
} // namespace godot
