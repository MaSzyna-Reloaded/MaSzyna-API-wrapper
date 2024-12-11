#pragma once
#include "../core/GameLog.hpp"
#include "../core/TrainCommand.hpp"
#include "../core/TrainController.hpp"
#include "LegacyBufferCouplerModule.hpp"

namespace godot {
    class TrainBuffCoupl: public LegacyBufferCouplerModule {
            GDCLASS(TrainBuffCoupl, LegacyBufferCouplerModule);
        private:
            static void _bind_methods();
        protected:
            TypedArray<TrainCommand> get_supported_commands() override;

        public:
            void couple();
            void decouple();
    };
} //namespace godot
