#pragma once
#include "../macros.hpp"
#include "../core/TrainPart.hpp"
#include "../resources/load/LoadListItem.hpp"

namespace godot {
    class TrainLoad : public TrainPart {
        GDCLASS(TrainLoad, TrainPart)
        private:
            static void _bind_methods();
            TypedArray<LoadListItem> load_list;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            enum LoadUnit {
                LOAD_UNIT_TONS,
                LOAD_UNIT_PIECES
            };

            MAKE_MEMBER_GS_NR(LoadUnit, load_unit, LoadUnit::LOAD_UNIT_TONS);
            MAKE_MEMBER_GS(double, overload_factor, 0.0f);
            MAKE_MEMBER_GS(float, load_speed, 0.0f);
            MAKE_MEMBER_GS(float, unload_speed, 0.0f);

            void set_load_list(const TypedArray<LoadListItem> &p_load_list) {
                load_list.clear();
                load_list.append_array(p_load_list);
            }

            TypedArray<LoadListItem> get_load_list() {
                return load_list;
            }
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainLoad::LoadUnit)