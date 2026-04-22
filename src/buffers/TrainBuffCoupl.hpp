#pragma once
#include "../core/GameLog.hpp"
#include "../core/TrainController.hpp"
#include "../core/TrainSystem.hpp"
#include "LegacyBufferCouplerModule.hpp"
#include <godot_cpp/classes/ref.hpp>

namespace godot {
    class TrainBuffCoupl: public LegacyBufferCouplerModule {
            GDCLASS(TrainBuffCoupl, LegacyBufferCouplerModule);
        private:
            Ref<TrainController> train_controller;
            Dictionary command_registry;
            bool collecting_commands = false;
            static void _bind_methods();
        protected:
            void _initialize() override;
            void _finalize() override;
            void _register_commands();
            void _unregister_commands();
        public:
            void couple();
            void decouple();
            Dictionary get_supported_commands() override;
    };
} //namespace godot
