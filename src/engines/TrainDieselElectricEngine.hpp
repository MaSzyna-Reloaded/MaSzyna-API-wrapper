#include "TrainDieselEngine.hpp"

namespace godot {
    class TrainDieselElectricEngine : public TrainDieselEngine {
            GDCLASS(TrainDieselElectricEngine, TrainDieselEngine)

        private:
            static void _bind_methods();

        protected:
            TEngineType get_engine_type() override;

        public:
            TrainDieselElectricEngine();
            ~TrainDieselElectricEngine() override = default;
    };
} // namespace godot
