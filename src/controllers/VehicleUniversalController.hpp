#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include "resources/controllers/UniversalControllerListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleUniversalController : public VehicleComponent {
            GDCLASS(VehicleUniversalController, VehicleComponent);

        private:
            static void _bind_methods();

        private:
            int state_base_index = 0;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;

            enum StateProperty {
                STATE_SELECTOR_POSITION,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            MAKE_MEMBER_GS(bool, integrated_brake_pn, true);
            MAKE_MEMBER_GS(bool, integrated_brake, true);
            MAKE_MEMBER_GS_DIRTY(int, selector_position, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<UniversalControllerListItem>, positions)
    };
} // namespace godot
