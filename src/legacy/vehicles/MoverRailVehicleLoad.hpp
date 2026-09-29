#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleLoad.hpp"

namespace godot {
    /* RailVehicleLoad on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleLoad : public RailVehicleLoad, public MoverComponent {
            GDCLASS(MoverRailVehicleLoad, RailVehicleLoad);

        private:
            static void _bind_methods();

            TypedArray<RailVehicleLoadListItem> load_list;

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;

        public:
            void set_load_list(const TypedArray<RailVehicleLoadListItem> &p_load_list) override {
                load_list.clear();
                load_list.append_array(p_load_list);
            }
            TypedArray<RailVehicleLoadListItem> get_load_list() override {
                return load_list;
            }
    };
} // namespace godot
