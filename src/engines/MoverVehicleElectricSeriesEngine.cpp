#include "../mover/MoverBackend.hpp"
#include "MoverVehicleElectricSeriesEngine.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleElectricSeriesEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverVehicleElectricSeriesEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverVehicleElectricSeriesEngine::get_circuit_imax);
        ClassDB::bind_method(
                D_METHOD("get_dynamic_brake_active"), &MoverVehicleElectricSeriesEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverVehicleElectricSeriesEngine::get_fuse_active);
        ClassDB::bind_method(
                D_METHOD("get_motor_connectors_open"), &MoverVehicleElectricSeriesEngine::get_motor_connectors_open);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverVehicleElectricSeriesEngine::fuse_reset);
        ClassDB::bind_method(
                D_METHOD("set_motor_connectors_open", "open"),
                &MoverVehicleElectricSeriesEngine::set_motor_connectors_open);
    }

    double MoverVehicleElectricSeriesEngine::get_motor_current() const {
        return traction.get_motor_current(this);
    }

    double MoverVehicleElectricSeriesEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(this);
    }

    bool MoverVehicleElectricSeriesEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(this);
    }

    bool MoverVehicleElectricSeriesEngine::get_fuse_active() const {
        return traction.get_fuse_active(this);
    }

    bool MoverVehicleElectricSeriesEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(this);
    }

    void MoverVehicleElectricSeriesEngine::fuse_reset() {
        traction.reset_fuse(this);
    }

    void MoverVehicleElectricSeriesEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(this, p_open);
    }

    void MoverVehicleElectricSeriesEngine::_register_commands() {
        VehicleElectricSeriesEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleElectricSeriesEngine::_unregister_commands() {
        VehicleElectricSeriesEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleElectricSeriesEngine::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleElectricSeriesEngine::_apply_configuration();
        p_mover->NominalVoltage = get_nominal_voltage();
        p_mover->WindingRes = get_winding_resistance();
        p_mover->nmax = get_max_rpm() / 60.0;

        p_mover->RVentType = static_cast<int>(get_resistor_fan_type());
        p_mover->RVentnmax = get_resistor_fan_max_rpm();
        p_mover->RVentCutOff = get_resistor_fan_cutoff_resistance();
        p_mover->RVentMinI = get_resistor_fan_min_current();
        p_mover->RVentSpeed = get_resistor_fan_speed();
        p_mover->DynamicBrakeRes = get_dynamic_brake_resistance();
        p_mover->DynamicBrakeRes1 = get_dynamic_brake_resistance_1();
        p_mover->DynamicBrakeRes2 = get_dynamic_brake_resistance_2();

        /* RList: lista rezystorow rozruchowych i polaczen silnikow (rozruch samoczynny) */
        constexpr int MAX_RELAY_LIST = Maszyna::ResArraySize + 1;
        const int relay_list_size = static_cast<int>(get_relay_list().size());
        if (relay_list_size > MAX_RELAY_LIST) {
            UtilityFunctions::push_warning(
                    "[MoverVehicleElectricSeriesEngine]: relay_list has " + String::num(relay_list_size) +
                    " entries, exceeding the mover's limit of " + String::num(MAX_RELAY_LIST) + "; truncating.");
        }
        p_mover->RlistSize = std::min(MAX_RELAY_LIST, relay_list_size);
        for (int i = 0; i < p_mover->RlistSize; i++) {
            const Ref<RelayListItem> &row = get_relay_list()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[MoverVehicleElectricSeriesEngine]: relay_list property is null at index " + String::num(i));
                continue;
            }
            p_mover->RList[i].Relay = row->get_relay_position();
            p_mover->RList[i].R = row->get_resistance();
            p_mover->RList[i].Bn = row->get_branch_count();
            p_mover->RList[i].Mn = row->get_motors_per_branch();
            p_mover->RList[i].AutoSwitch = row->get_auto_switch();
            p_mover->RList[i].ScndAct = row->get_shunt_index();
        }
    }

    double MoverVehicleElectricSeriesEngine::get_resistor_fan_rotation() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->RventRot : 0.0;
    }

    void MoverVehicleElectricSeriesEngine::_fill_config_dictionary(Dictionary &p_config) const {
        VehicleElectricSeriesEngine::_fill_config_dictionary(p_config);
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_config["resistor_fan_max_rpm"] = mover->RVentnmax;
    }
} // namespace godot
