#pragma once
#include "../core/GameLog.hpp"
#include "../core/TrainController.hpp"
#include "../core/TrainSystem.hpp"
#include "LegacyBufferCouplerModule.hpp"

namespace godot {
    class TrainBuffCoupl: public LegacyBufferCouplerModule {
            GDCLASS(TrainBuffCoupl, LegacyBufferCouplerModule);
        private:
            TrainController *train_controller_node = nullptr;
            static void _bind_methods();
        protected:
            void _notification(int p_what);
            void _register_commands();
            void _unregister_commands();
        public:
            void couple();
            void decouple();
    };
} //namespace godot
