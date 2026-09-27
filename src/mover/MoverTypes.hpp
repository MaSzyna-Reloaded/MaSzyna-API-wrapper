#pragma once
#include "../core/TrainController.hpp"
#include "../core/VehicleController.hpp"
#include "../engines/VehicleEngine.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include <map>

/* The vehicle interfaces' own enums as the vendored Mover spells them. Only Mover* implementations
 * include this; the interfaces name no Mover type. */

namespace godot {
    inline Maszyna::start_t mover_start_mode(const VehicleEngine::StartMode p_mode) {
        static const std::map<VehicleEngine::StartMode, Maszyna::start_t> map = {
                {VehicleEngine::START_MODE_DISABLED, Maszyna::start_t::disabled},
                {VehicleEngine::START_MODE_MANUAL, Maszyna::start_t::manual},
                {VehicleEngine::START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                {VehicleEngine::START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                {VehicleEngine::START_MODE_CONVERTER, Maszyna::start_t::converter},
                {VehicleEngine::START_MODE_BATTERY, Maszyna::start_t::battery},
                {VehicleEngine::START_MODE_DIRECTION, Maszyna::start_t::direction},
        };
        return map.at(p_mode);
    }

    inline Maszyna::start_t mover_start_mode(const TrainController::StartMode p_mode) {
        static const std::map<TrainController::StartMode, Maszyna::start_t> map = {
                {TrainController::START_MODE_DISABLED, Maszyna::start_t::disabled},
                {TrainController::START_MODE_MANUAL, Maszyna::start_t::manual},
                {TrainController::START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                {TrainController::START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                {TrainController::START_MODE_CONVERTER, Maszyna::start_t::converter},
                {TrainController::START_MODE_BATTERY, Maszyna::start_t::battery},
                {TrainController::START_MODE_DIRECTION, Maszyna::start_t::direction},
        };
        return map.at(p_mode);
    }

    inline Maszyna::TEngineType mover_engine_type(const VehicleEngine::EngineType p_type) {
        static const std::map<VehicleEngine::EngineType, Maszyna::TEngineType> map = {
                {VehicleEngine::NONE, Maszyna::TEngineType::None},
                {VehicleEngine::DUMB, Maszyna::TEngineType::Dumb},
                {VehicleEngine::WHEELS_DRIVEN, Maszyna::TEngineType::WheelsDriven},
                {VehicleEngine::ELECTRIC_SERIES_MOTOR, Maszyna::TEngineType::ElectricSeriesMotor},
                {VehicleEngine::ELECTRIC_INDUCTION_MOTOR, Maszyna::TEngineType::ElectricInductionMotor},
                {VehicleEngine::DIESEL, Maszyna::TEngineType::DieselEngine},
                {VehicleEngine::STEAM, Maszyna::TEngineType::SteamEngine},
                {VehicleEngine::DIESEL_ELECTRIC, Maszyna::TEngineType::DieselElectric},
                {VehicleEngine::MAIN, Maszyna::TEngineType::Main},
        };
        return map.at(p_type);
    }

    inline Maszyna::TPowerSource mover_power_source(const TrainController::TrainPowerSource p_source) {
        static const std::map<TrainController::TrainPowerSource, Maszyna::TPowerSource> map = {
                {TrainController::POWER_SOURCE_NOT_DEFINED, Maszyna::TPowerSource::NotDefined},
                {TrainController::POWER_SOURCE_INTERNAL, Maszyna::TPowerSource::InternalSource},
                {TrainController::POWER_SOURCE_TRANSDUCER, Maszyna::TPowerSource::Transducer},
                {TrainController::POWER_SOURCE_GENERATOR, Maszyna::TPowerSource::Generator},
                {TrainController::POWER_SOURCE_ACCUMULATOR, Maszyna::TPowerSource::Accumulator},
                {TrainController::POWER_SOURCE_CURRENTCOLLECTOR, Maszyna::TPowerSource::CurrentCollector},
                {TrainController::POWER_SOURCE_POWERCABLE, Maszyna::TPowerSource::PowerCable},
                {TrainController::POWER_SOURCE_HEATER, Maszyna::TPowerSource::Heater},
                {TrainController::POWER_SOURCE_MAIN, Maszyna::TPowerSource::Main},
        };
        return map.at(p_source);
    }

    inline TrainController::TrainPowerSource power_source_of_mover(const Maszyna::TPowerSource p_source) {
        static const std::map<Maszyna::TPowerSource, TrainController::TrainPowerSource> map = {
                {Maszyna::TPowerSource::NotDefined, TrainController::POWER_SOURCE_NOT_DEFINED},
                {Maszyna::TPowerSource::InternalSource, TrainController::POWER_SOURCE_INTERNAL},
                {Maszyna::TPowerSource::Transducer, TrainController::POWER_SOURCE_TRANSDUCER},
                {Maszyna::TPowerSource::Generator, TrainController::POWER_SOURCE_GENERATOR},
                {Maszyna::TPowerSource::Accumulator, TrainController::POWER_SOURCE_ACCUMULATOR},
                {Maszyna::TPowerSource::CurrentCollector, TrainController::POWER_SOURCE_CURRENTCOLLECTOR},
                {Maszyna::TPowerSource::PowerCable, TrainController::POWER_SOURCE_POWERCABLE},
                {Maszyna::TPowerSource::Heater, TrainController::POWER_SOURCE_HEATER},
                {Maszyna::TPowerSource::Main, TrainController::POWER_SOURCE_MAIN},
        };
        return map.at(p_source);
    }

    inline Maszyna::TPowerType mover_power_type(const TrainController::TrainPowerType p_type) {
        static const std::map<TrainController::TrainPowerType, Maszyna::TPowerType> map = {
                {TrainController::POWER_TYPE_NONE, Maszyna::TPowerType::NoPower},
                {TrainController::POWER_TYPE_BIO, Maszyna::TPowerType::BioPower},
                {TrainController::POWER_TYPE_MECH, Maszyna::TPowerType::MechPower},
                {TrainController::POWER_TYPE_ELECTRIC, Maszyna::TPowerType::ElectricPower},
                {TrainController::POWER_TYPE_STEAM, Maszyna::TPowerType::SteamPower},
        };
        return map.at(p_type);
    }
} // namespace godot
