#pragma once
#include <godot_cpp/core/binder_common.hpp>

#include "TrainCommand.hpp"
#include "TrainPart.hpp"

namespace godot {
    class GenericTrainPart : public TrainPart {
            GDCLASS(GenericTrainPart, TrainPart)

        private:
            static void _bind_methods();
            Dictionary internal_state;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;

        public:
            TrainController *get_train_controller_node();
            void _process_mover(double p_delta) override;
            virtual void _process_train_part(double p_delta);
            virtual Dictionary _get_train_part_state();
            virtual Array _get_supported_commands();
            TypedArray<TrainCommand> get_supported_commands() override;
            Dictionary get_train_state();
    };
} // namespace godot
