#ifndef TRAINSET_HPP
#define TRAINSET_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "TrainController.hpp"

namespace godot {

    class TrainSet : public RefCounted {
            GDCLASS(TrainSet, RefCounted);

        private:
            TrainController *start_vehicle = nullptr;

        protected:
            static void _bind_methods();

        public:
            TrainSet();
            void _init(TrainController *_start_vehicle);

            TrainController *get_by_index(int index);
            Array to_array() const;

            TrainController *get_head() const;
            TrainController *get_tail() const;

            void attach_to_head(TrainController *vehicle, TrainController::Side side);
            void attach_to_tail(TrainController *vehicle, TrainController::Side side);
    };
} // namespace godot

#endif // TRAINSET_HPP
