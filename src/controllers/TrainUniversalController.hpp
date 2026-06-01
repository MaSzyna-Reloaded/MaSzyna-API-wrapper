#pragma once
#include "../core/TrainPart.hpp"
#include "macros.hpp"
#include "resources/controllers/UCListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainUniversalController : public TrainPart {
            GDCLASS(TrainUniversalController, TrainPart);

        private:
            static void _bind_methods();

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;

        public:
            MAKE_MEMBER_GS(bool, integrated_brake_pn, true);
            MAKE_MEMBER_GS(bool, integrated_brake, true);
            MAKE_MEMBER_GS_DIRTY(int, selector_position, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<UCListItem>, uc_list)
    };
} // namespace godot
