#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

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

        public:
            TrainController *get_train_controller_node();
            void _process_mover(double delta) override;
            virtual void _process_train_part(double delta);
            virtual Dictionary _get_train_part_state();
            Dictionary get_train_state();

            GenericTrainPart();
            ~GenericTrainPart() override = default;
    };
} // namespace godot
