#pragma once
#include <godot_cpp/core/binder_common.hpp>

#include "TrainPart.hpp"

namespace godot {
    class GenericTrainPart : public TrainPart {
            GDCLASS(GenericTrainPart, TrainPart)

        private:
            static void _bind_methods();
            Dictionary internal_state;
            // Captured by _process_mover_thread_safe() (real physics-tick delta from
            // TrainPhysicsServer::step_physics()) and consumed by _post_process_mover_sync(), which
            // runs afterwards in the same tick to forward it to the GDScript _process_train_part()
            // virtual.
            double last_delta = 0.0;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;

        public:
            TrainController *get_train_controller_node();
            void _process_mover_thread_safe(double p_delta) override;
            void _post_process_mover_sync() override;
            virtual void _process_train_part(double p_delta);
            virtual Dictionary _get_train_part_state();
            Dictionary get_train_state();
    };
} // namespace godot
