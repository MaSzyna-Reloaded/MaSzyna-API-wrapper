#include "TrainDieselElectricEngine.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainDieselElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainDieselElectricEngine, Variant::ARRAY, wwlist, PROPERTY_HINT_TYPE_STRING, "WWListItem");
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::BOOL, generator_voltage_flat);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::FLOAT, hyperbolic_speed);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::FLOAT, additional_speed);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::FLOAT, rpm_change_rate);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::FLOAT, power_correction_ratio);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::INT, shunt_relay_type);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::BOOL, shunt_mode_allowed);
        BIND_PROPERTY(TrainDieselElectricEngine, Variant::FLOAT, heating_rpm);
    }

    TrainEngine::EngineType TrainDieselElectricEngine::get_engine_type() {
        return TrainEngine::EngineType::DIESEL_ELECTRIC;
    }

    void TrainDieselElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        TrainDieselEngine::_do_update_internal_mover(p_mover);

        p_mover->Flat = generator_voltage_flat;
        p_mover->Vhyp = hyperbolic_speed;
        p_mover->Vadd = additional_speed;
        p_mover->dizel_RevolutionsDecreaseRate = rpm_change_rate;
        p_mover->PowerCorRatio = power_correction_ratio;
        p_mover->RelayType = shunt_relay_type;
        p_mover->ShuntModeAllow = shunt_mode_allowed;
        p_mover->EngineHeatingRPM = heating_rpm;

        /* WWList: tablica rezystorow rozr. (eng. Starting resistor array) aka DEList aka TDESchemeTable */
        constexpr int MAX = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(wwlist.size());
        p_mover->MainCtrlPosNo = wwlist_size - 1;
        for (int i = 0; i < std::min(MAX, wwlist_size); i++) {
            const Ref<WWListItem> &row = wwlist[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning(
                        "[TrainDieselElectricEngine]: wwlist property is null at index " + String::num(i));
                continue;
            }

            p_mover->DElist[i].RPM = row->get_rpm();
            p_mover->DElist[i].GenPower = row->get_max_power();
            p_mover->DElist[i].Umax = row->get_max_voltage();
            p_mover->DElist[i].Imax = row->get_max_current();
            if (row->get_has_shunting()) {
                p_mover->SST[i].Umin = row->get_min_wakeup_voltage();
                p_mover->SST[i].Umax = row->get_max_wakeup_voltage();
                p_mover->SST[i].Pmax = row->get_max_wakeup_power();
                p_mover->SST[i].Pmin = std::sqrt(std::pow(p_mover->SST[i].Umin, 2) / 47.6);
                p_mover->SST[i].Pmax = std::min(p_mover->SST[i].Pmax, std::pow(p_mover->SST[i].Umax, 2) / 47.6);
            }
        }
    }
} // namespace godot
