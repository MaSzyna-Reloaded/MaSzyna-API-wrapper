#pragma once
#include "../core/GameLog.hpp"
#include "../core/TrainController.hpp"
#include "../core/TrainSystem.hpp"
#include "LegacyBufferCouplerModule.hpp"

namespace godot {
    class TrainBuffCoupl: public LegacyBufferCouplerModule {
            GDCLASS(TrainBuffCoupl, LegacyBufferCouplerModule);
        private:
            TrainController *train_controller = nullptr;
            Dictionary command_registry;
            bool collecting_commands = false;
            static void _bind_methods();
        protected:
            void _register_commands();
            void _unregister_commands();
        public:
            void set_rail_vehicle(RailVehicle *p_rail_vehicle) override;
            void couple();
            void decouple();
            Dictionary get_supported_commands() override;
    };
} //namespace godot
