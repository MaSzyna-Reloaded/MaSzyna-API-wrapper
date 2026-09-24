#pragma once
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

    inline Maszyna::start_t mover_start_mode(const VehicleController::StartMode p_mode) {
        static const std::map<VehicleController::StartMode, Maszyna::start_t> map = {
                {VehicleController::START_MODE_DISABLED, Maszyna::start_t::disabled},
                {VehicleController::START_MODE_MANUAL, Maszyna::start_t::manual},
                {VehicleController::START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                {VehicleController::START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                {VehicleController::START_MODE_CONVERTER, Maszyna::start_t::converter},
                {VehicleController::START_MODE_BATTERY, Maszyna::start_t::battery},
                {VehicleController::START_MODE_DIRECTION, Maszyna::start_t::direction},
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

    inline Maszyna::TPowerSource mover_power_source(const VehicleController::TrainPowerSource p_source) {
        static const std::map<VehicleController::TrainPowerSource, Maszyna::TPowerSource> map = {
                {VehicleController::POWER_SOURCE_NOT_DEFINED, Maszyna::TPowerSource::NotDefined},
                {VehicleController::POWER_SOURCE_INTERNAL, Maszyna::TPowerSource::InternalSource},
                {VehicleController::POWER_SOURCE_TRANSDUCER, Maszyna::TPowerSource::Transducer},
                {VehicleController::POWER_SOURCE_GENERATOR, Maszyna::TPowerSource::Generator},
                {VehicleController::POWER_SOURCE_ACCUMULATOR, Maszyna::TPowerSource::Accumulator},
                {VehicleController::POWER_SOURCE_CURRENTCOLLECTOR, Maszyna::TPowerSource::CurrentCollector},
                {VehicleController::POWER_SOURCE_POWERCABLE, Maszyna::TPowerSource::PowerCable},
                {VehicleController::POWER_SOURCE_HEATER, Maszyna::TPowerSource::Heater},
                {VehicleController::POWER_SOURCE_MAIN, Maszyna::TPowerSource::Main},
        };
        return map.at(p_source);
    }

    inline VehicleController::TrainPowerSource power_source_of_mover(const Maszyna::TPowerSource p_source) {
        static const std::map<Maszyna::TPowerSource, VehicleController::TrainPowerSource> map = {
                {Maszyna::TPowerSource::NotDefined, VehicleController::POWER_SOURCE_NOT_DEFINED},
                {Maszyna::TPowerSource::InternalSource, VehicleController::POWER_SOURCE_INTERNAL},
                {Maszyna::TPowerSource::Transducer, VehicleController::POWER_SOURCE_TRANSDUCER},
                {Maszyna::TPowerSource::Generator, VehicleController::POWER_SOURCE_GENERATOR},
                {Maszyna::TPowerSource::Accumulator, VehicleController::POWER_SOURCE_ACCUMULATOR},
                {Maszyna::TPowerSource::CurrentCollector, VehicleController::POWER_SOURCE_CURRENTCOLLECTOR},
                {Maszyna::TPowerSource::PowerCable, VehicleController::POWER_SOURCE_POWERCABLE},
                {Maszyna::TPowerSource::Heater, VehicleController::POWER_SOURCE_HEATER},
                {Maszyna::TPowerSource::Main, VehicleController::POWER_SOURCE_MAIN},
        };
        return map.at(p_source);
    }

    inline Maszyna::TPowerType mover_power_type(const VehicleController::TrainPowerType p_type) {
        static const std::map<VehicleController::TrainPowerType, Maszyna::TPowerType> map = {
                {VehicleController::POWER_TYPE_NONE, Maszyna::TPowerType::NoPower},
                {VehicleController::POWER_TYPE_BIO, Maszyna::TPowerType::BioPower},
                {VehicleController::POWER_TYPE_MECH, Maszyna::TPowerType::MechPower},
                {VehicleController::POWER_TYPE_ELECTRIC, Maszyna::TPowerType::ElectricPower},
                {VehicleController::POWER_TYPE_STEAM, Maszyna::TPowerType::SteamPower},
        };
        return map.at(p_type);
    }
} // namespace godot
