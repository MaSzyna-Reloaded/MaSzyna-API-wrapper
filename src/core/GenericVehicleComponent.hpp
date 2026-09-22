#pragma once
#include <godot_cpp/core/binder_common.hpp>

#include "VehicleComponent.hpp"

namespace godot {
    class GenericVehicleComponent : public VehicleComponent {
            GDCLASS(GenericVehicleComponent, VehicleComponent)

        private:
            static void _bind_methods();
            Dictionary internal_state;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;

        public:
            VehicleController *get_train_controller_node();
            void _process_mover(double p_delta) override;
            Dictionary get_state() override;
            virtual void _process_train_part(double p_delta);
            virtual Dictionary _get_train_part_state();
            virtual Dictionary _get_train_part_config();
            Dictionary get_train_state();
    };
} // namespace godot
