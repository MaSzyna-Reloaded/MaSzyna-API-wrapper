#pragma once
#include <godot_cpp/core/binder_common.hpp>

#include "TrainPart.hpp"

namespace godot {
    class GenericTrainPart : public TrainPart {
            GDCLASS(GenericTrainPart, TrainPart)

        private:
            static void _bind_methods();
            Dictionary internal_state;

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _do_process_mover(TMoverParameters *mover, double delta) override;
            void _initialize() override;
            void _finalize() override;
            void _update(double delta) override;

        public:
            Ref<TrainController> get_train_controller() const;
            void _process_mover(double delta) override;
            virtual Dictionary _get_train_part_state();
            Dictionary get_train_state();
            virtual Dictionary _get_supported_commands();
            Dictionary get_supported_commands() override;
            Dictionary get_mover_state() override;
    };
} // namespace godot
