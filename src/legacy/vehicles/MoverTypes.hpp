#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "vehicles/base/VehicleController.hpp"
#include "vehicles/rail/RailVehicleController.hpp"
#include "vehicles/rail/RailVehicleEngine.hpp"
#include <map>

/* The vehicle interfaces' own enums as the vendored Mover spells them. Only Mover* implementations
 * include this; the interfaces name no Mover type. */

namespace godot {
    inline Maszyna::start_t mover_start_mode(const RailVehicleController::StartMode p_mode) {
        static const std::map<RailVehicleController::StartMode, Maszyna::start_t> map = {
                {RailVehicleController::START_MODE_DISABLED, Maszyna::start_t::disabled},
                {RailVehicleController::START_MODE_MANUAL, Maszyna::start_t::manual},
                {RailVehicleController::START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                {RailVehicleController::START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                {RailVehicleController::START_MODE_CONVERTER, Maszyna::start_t::converter},
                {RailVehicleController::START_MODE_BATTERY, Maszyna::start_t::battery},
                {RailVehicleController::START_MODE_DIRECTION, Maszyna::start_t::direction},
        };
        return map.at(p_mode);
    }

    inline Maszyna::TEngineType mover_engine_type(const RailVehicleEngine::EngineType p_type) {
        static const std::map<RailVehicleEngine::EngineType, Maszyna::TEngineType> map = {
                {RailVehicleEngine::NONE, Maszyna::TEngineType::None},
                {RailVehicleEngine::DUMB, Maszyna::TEngineType::Dumb},
                {RailVehicleEngine::WHEELS_DRIVEN, Maszyna::TEngineType::WheelsDriven},
                {RailVehicleEngine::ELECTRIC_SERIES_MOTOR, Maszyna::TEngineType::ElectricSeriesMotor},
                {RailVehicleEngine::ELECTRIC_INDUCTION_MOTOR, Maszyna::TEngineType::ElectricInductionMotor},
                {RailVehicleEngine::DIESEL, Maszyna::TEngineType::DieselEngine},
                {RailVehicleEngine::STEAM, Maszyna::TEngineType::SteamEngine},
                {RailVehicleEngine::DIESEL_ELECTRIC, Maszyna::TEngineType::DieselElectric},
                {RailVehicleEngine::MAIN, Maszyna::TEngineType::Main},
        };
        return map.at(p_type);
    }

    inline Maszyna::TPowerSource mover_power_source(const RailVehicleController::TrainPowerSource p_source) {
        static const std::map<RailVehicleController::TrainPowerSource, Maszyna::TPowerSource> map = {
                {RailVehicleController::POWER_SOURCE_NOT_DEFINED, Maszyna::TPowerSource::NotDefined},
                {RailVehicleController::POWER_SOURCE_INTERNAL, Maszyna::TPowerSource::InternalSource},
                {RailVehicleController::POWER_SOURCE_TRANSDUCER, Maszyna::TPowerSource::Transducer},
                {RailVehicleController::POWER_SOURCE_GENERATOR, Maszyna::TPowerSource::Generator},
                {RailVehicleController::POWER_SOURCE_ACCUMULATOR, Maszyna::TPowerSource::Accumulator},
                {RailVehicleController::POWER_SOURCE_CURRENTCOLLECTOR, Maszyna::TPowerSource::CurrentCollector},
                {RailVehicleController::POWER_SOURCE_POWERCABLE, Maszyna::TPowerSource::PowerCable},
                {RailVehicleController::POWER_SOURCE_HEATER, Maszyna::TPowerSource::Heater},
                {RailVehicleController::POWER_SOURCE_MAIN, Maszyna::TPowerSource::Main},
        };
        return map.at(p_source);
    }

    inline RailVehicleController::TrainPowerSource power_source_of_mover(const Maszyna::TPowerSource p_source) {
        static const std::map<Maszyna::TPowerSource, RailVehicleController::TrainPowerSource> map = {
                {Maszyna::TPowerSource::NotDefined, RailVehicleController::POWER_SOURCE_NOT_DEFINED},
                {Maszyna::TPowerSource::InternalSource, RailVehicleController::POWER_SOURCE_INTERNAL},
                {Maszyna::TPowerSource::Transducer, RailVehicleController::POWER_SOURCE_TRANSDUCER},
                {Maszyna::TPowerSource::Generator, RailVehicleController::POWER_SOURCE_GENERATOR},
                {Maszyna::TPowerSource::Accumulator, RailVehicleController::POWER_SOURCE_ACCUMULATOR},
                {Maszyna::TPowerSource::CurrentCollector, RailVehicleController::POWER_SOURCE_CURRENTCOLLECTOR},
                {Maszyna::TPowerSource::PowerCable, RailVehicleController::POWER_SOURCE_POWERCABLE},
                {Maszyna::TPowerSource::Heater, RailVehicleController::POWER_SOURCE_HEATER},
                {Maszyna::TPowerSource::Main, RailVehicleController::POWER_SOURCE_MAIN},
        };
        return map.at(p_source);
    }

    inline Maszyna::TPowerType mover_power_type(const RailVehicleController::TrainPowerType p_type) {
        static const std::map<RailVehicleController::TrainPowerType, Maszyna::TPowerType> map = {
                {RailVehicleController::POWER_TYPE_NONE, Maszyna::TPowerType::NoPower},
                {RailVehicleController::POWER_TYPE_BIO, Maszyna::TPowerType::BioPower},
                {RailVehicleController::POWER_TYPE_MECH, Maszyna::TPowerType::MechPower},
                {RailVehicleController::POWER_TYPE_ELECTRIC, Maszyna::TPowerType::ElectricPower},
                {RailVehicleController::POWER_TYPE_STEAM, Maszyna::TPowerType::SteamPower},
        };
        return map.at(p_type);
    }
} // namespace godot
