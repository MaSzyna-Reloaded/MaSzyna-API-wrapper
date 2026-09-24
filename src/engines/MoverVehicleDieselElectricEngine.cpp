#include "MoverVehicleDieselElectricEngine.hpp"
#include "../mover/MoverBackend.hpp"
#include <algorithm>
#include <cmath>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleDieselElectricEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverVehicleDieselElectricEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverVehicleDieselElectricEngine::get_circuit_imax);
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &MoverVehicleDieselElectricEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverVehicleDieselElectricEngine::get_fuse_active);
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &MoverVehicleDieselElectricEngine::get_motor_connectors_open);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverVehicleDieselElectricEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("set_motor_connectors_open", "open"), &MoverVehicleDieselElectricEngine::set_motor_connectors_open);
    }

    double MoverVehicleDieselElectricEngine::get_motor_current() const {
        return traction.get_motor_current(this);
    }

    double MoverVehicleDieselElectricEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(this);
    }

    bool MoverVehicleDieselElectricEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(this);
    }

    bool MoverVehicleDieselElectricEngine::get_fuse_active() const {
        return traction.get_fuse_active(this);
    }

    bool MoverVehicleDieselElectricEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(this);
    }

    void MoverVehicleDieselElectricEngine::fuse_reset() {
        traction.reset_fuse(this);
    }

    void MoverVehicleDieselElectricEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(this, p_open);
    }

    void MoverVehicleDieselElectricEngine::_register_commands() {
        VehicleDieselElectricEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleDieselElectricEngine::_unregister_commands() {
        VehicleDieselElectricEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleDieselElectricEngine::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleDieselElectricEngine::_apply_configuration();

        p_mover->Flat = get_generator_voltage_flat();
        p_mover->Vhyp = get_hyperbolic_speed();
        p_mover->Vadd = get_additional_speed();
        p_mover->dizel_RevolutionsDecreaseRate = get_rpm_change_rate();
        p_mover->PowerCorRatio = get_power_correction_ratio();
        p_mover->RelayType = get_shunt_relay_type();
        p_mover->ShuntModeAllow = get_shunt_mode_allowed();
        p_mover->EngineHeatingRPM = get_heating_rpm();

        /* WWList: tablica rezystorow rozr. (eng. Starting resistor array) aka DEList aka TDESchemeTable */
        constexpr int MAX = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(get_wwlist().size());
        p_mover->MainCtrlPosNo = wwlist_size - 1;
        for (int i = 0; i < std::min(MAX, wwlist_size); i++) {
            const Ref<WWListItem> &row = get_wwlist()[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning(
                        "[MoverVehicleDieselElectricEngine]: wwlist property is null at index " + String::num(i));
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
                p_mover->SST[i].Pmin = std::sqrt(std::pow(p_mover->SST[i].Umin, 2) / WWLIST_SHUNT_POWER_DIVISOR);
                p_mover->SST[i].Pmax = std::min(p_mover->SST[i].Pmax, std::pow(p_mover->SST[i].Umax, 2) / WWLIST_SHUNT_POWER_DIVISOR);
            }
        }
    }
} // namespace godot
