#pragma once
#include <godot_cpp/classes/node.hpp>
#include "TrainController.hpp"

namespace godot {
    class TrainPart : public Node {
            GDCLASS(TrainPart, Node)
        public:
            static void _bind_methods();
            Dictionary state;

        protected:
            bool _dirty = false;

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do _do_update_internal_mover(),
             * i powinna byc wywolywana przez TrainPart::initialize_mover() */
            // virtual void _do_initialize_internal_mover(TMoverParameters *mover) = 0;

            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            /* Transfers data from Godot's node to original/internal Mover instance.
             * `mover` is always set */

            virtual void _do_update_internal_mover(TMoverParameters *mover) = 0;

            /* Transfers state from the original/internal Mover instance to Godot's Dictionary.
             * `mover` and `state` are always set
             * */

            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;

        public:
            void _ready() override;
            void _process(double delta) override;

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do update_mover(),
             * i powinna byc wywolywana z poziomu TrainController::initialize_mover() */
            // void initialize_mover(TrainController *train_controller_node);

            /* High level method for updating the state of the Mover */
            void update_mover(const TrainController *train_controller_node);

            /* High level method for getting the state of the Mover */
            Dictionary get_mover_state(const TrainController *train_controller_node);
            TrainPart();
            ~TrainPart() override = default;
            void emit_config_changed_signal();
    };
} // namespace godot
