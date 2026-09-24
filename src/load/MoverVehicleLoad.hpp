#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleLoad.hpp"

namespace godot {
    /* VehicleLoad on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleLoad : public VehicleLoad, public MoverComponent {
            GDCLASS(MoverVehicleLoad, VehicleLoad);

        private:
            static void _bind_methods();
        private:
            TypedArray<LoadListItem> load_list;
        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
        public:
            void set_load_list(const TypedArray<LoadListItem> &p_load_list) override {
                load_list.clear();
                load_list.append_array(p_load_list);
            }
            TypedArray<LoadListItem> get_load_list() override {
                return load_list;
            }
    };
} // namespace godot
