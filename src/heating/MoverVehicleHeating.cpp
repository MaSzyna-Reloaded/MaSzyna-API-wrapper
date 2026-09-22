#include "MoverVehicleHeating.hpp"
#include "../mover/MoverBackend.hpp"
#include "../core/VehicleController.hpp"

namespace godot {
    void MoverVehicleHeating::_bind_methods() {}

    void MoverVehicleHeating::_apply_configuration() {
        TMoverParameters *p_mover = mover_of(this);
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();

        p_mover->HeatingPowerSource.SourceType = train_controller_node->power_source_map.at(get_heating_source());
        p_mover->HeatingPowerSource.MaxVoltage = get_heating_max_voltage();

        switch (get_heating_source()) {
            case VehicleController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551); HeatingCheck() dereferences it unconditionally whenever
                // SourceType == Generator, so it must be pointed at a real double before that can
                // run safely. enrot is the vehicle's own engine revolutions counter.
                p_mover->HeatingPowerSource.EngineGenerator.engine_revolutions = &p_mover->enrot;
                p_mover->HeatingPowerSource.EngineGenerator.revolutions_min = get_heating_generator_min_rpm() / 60.0;
                p_mover->HeatingPowerSource.EngineGenerator.revolutions_max = get_heating_generator_max_rpm() / 60.0;
                p_mover->HeatingPowerSource.EngineGenerator.voltage_min = get_heating_generator_min_voltage();
                p_mover->HeatingPowerSource.EngineGenerator.voltage_max = get_heating_generator_max_voltage();
                break;
            }
            case VehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->HeatingPowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(get_heating_power_cable_type());
                break;
            }
            default:
                break;
        }
    }

    bool MoverVehicleHeating::get_active() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->Heating : false;
    }

    double MoverVehicleHeating::get_power() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->HeatingPower : 0.0;
    }

    void MoverVehicleHeating::heating(const bool p_enabled) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        mover->HeatingSwitch(p_enabled);
    }

    void MoverVehicleHeating::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (mover_of(this) == nullptr) {
            return;
        }
        p_state["heating_enabled"] = get_active();
        p_state["heating_power"] = get_power();
    }
} // namespace godot
