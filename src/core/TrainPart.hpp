#pragma once

#include "../maszyna/McZapkie/MOVER.h"
#include "./TrainCommand.hpp"
#include "TrainController.hpp"
#include <functional>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#define ASSERT_MOVER(mover_ptr)                                                                                        \
    if ((mover_ptr) == nullptr) {                                                                                      \
        return;                                                                                                        \
    }

namespace godot {
    class TrainPart : public Node {
            GDCLASS(TrainPart, Node)
        private:
            Dictionary state;

        protected:
            bool _dirty = false;
            bool enabled = true;
            bool enabled_changed = false;
            TrainController *train_controller_node = nullptr;

            void _notification(int p_what);

            virtual void _do_update_internal_mover(TMoverParameters *mover);
            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);
            virtual void _do_process_mover(TMoverParameters *mover, double delta);
            virtual void _process_mover(double delta);

            TMoverParameters *get_mover();

        public:
            ~TrainPart() override = default;
            void _process(double delta) override;

            virtual TypedArray<TrainCommand> get_supported_commands();
            void update_mover();
            Dictionary get_mover_state();
            TrainController *get_train_controller_node() const;

            void set_enabled(bool p_value);
            bool get_enabled();
            void emit_config_changed_signal();

            static void _bind_methods();
    };
} // namespace godot
