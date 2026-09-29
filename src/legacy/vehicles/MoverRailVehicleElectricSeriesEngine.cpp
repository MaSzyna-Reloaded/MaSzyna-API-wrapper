#include "MoverRailVehicleElectricSeriesEngine.hpp"
#include "legacy/vehicles/MoverBackend.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>
#include <limits>

namespace godot {
    void MoverRailVehicleElectricSeriesEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverRailVehicleElectricSeriesEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverRailVehicleElectricSeriesEngine::get_circuit_imax);
        ClassDB::bind_method(
                D_METHOD("get_dynamic_brake_active"), &MoverRailVehicleElectricSeriesEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverRailVehicleElectricSeriesEngine::get_fuse_active);
        ClassDB::bind_method(
                D_METHOD("get_motor_connectors_open"),
                &MoverRailVehicleElectricSeriesEngine::get_motor_connectors_open);
        ClassDB::bind_method(
                D_METHOD("is_line_contactor_closed"), &MoverRailVehicleElectricSeriesEngine::is_line_contactor_closed);
        ClassDB::bind_method(
                D_METHOD("is_pressure_switch_tripped"),
                &MoverRailVehicleElectricSeriesEngine::is_pressure_switch_tripped);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverRailVehicleElectricSeriesEngine::fuse_reset);
        ClassDB::bind_method(
                D_METHOD("set_motor_connectors_open", "open"),
                &MoverRailVehicleElectricSeriesEngine::set_motor_connectors_open);
    }

    double MoverRailVehicleElectricSeriesEngine::get_motor_current() const {
        return traction.get_motor_current(this);
    }

    double MoverRailVehicleElectricSeriesEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(this);
    }

    bool MoverRailVehicleElectricSeriesEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(this);
    }

    bool MoverRailVehicleElectricSeriesEngine::get_fuse_active() const {
        return traction.get_fuse_active(this);
    }

    bool MoverRailVehicleElectricSeriesEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(this);
    }

    bool MoverRailVehicleElectricSeriesEngine::is_line_contactor_closed() const {
        return traction.is_line_contactor_closed(this);
    }

    bool MoverRailVehicleElectricSeriesEngine::is_pressure_switch_tripped() const {
        return traction.is_pressure_switch_tripped(this);
    }

    void MoverRailVehicleElectricSeriesEngine::fuse_reset() {
        traction.reset_fuse(this);
    }

    void MoverRailVehicleElectricSeriesEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(this, p_open);
    }

    void MoverRailVehicleElectricSeriesEngine::_register_commands() {
        RailVehicleElectricSeriesEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverRailVehicleElectricSeriesEngine::_unregister_commands() {
        RailVehicleElectricSeriesEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverRailVehicleElectricSeriesEngine::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        RailVehicleElectricSeriesEngine::_apply_configuration();
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
                    "[MoverRailVehicleElectricSeriesEngine]: relay_list has " + String::num(relay_list_size) +
                    " entries, exceeding the mover's limit of " + String::num(MAX_RELAY_LIST) + "; truncating.");
        }
        p_mover->RlistSize = std::min(MAX_RELAY_LIST, relay_list_size);
        for (int i = 0; i < p_mover->RlistSize; i++) {
            const Ref<RailVehicleRelayListItem> &row = get_relay_list()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[MoverRailVehicleElectricSeriesEngine]: relay_list property is null at index " +
                        String::num(i));
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

    double MoverRailVehicleElectricSeriesEngine::get_resistor_fan_rotation() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->RventRot : 0.0;
    }

    double MoverRailVehicleElectricSeriesEngine::get_circuit_imin() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->Imin : 0.0;
    }

    double MoverRailVehicleElectricSeriesEngine::get_engine_voltage() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->EngineVoltage : 0.0;
    }

    void MoverRailVehicleElectricSeriesEngine::_fill_config_dictionary(Dictionary &p_config) const {
        RailVehicleElectricSeriesEngine::_fill_config_dictionary(p_config);
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_config["resistor_fan_max_rpm"] = mover->RVentnmax;
    }

    // Original engine: TController::ESMVelocity() (Driver.cpp:2344-2382) - the current is iterated five
    // times towards the one the adhesion allows, then held under 90% of the relay's
    double MoverRailVehicleElectricSeriesEngine::get_next_position_velocity(const bool p_main_controller) const {
        constexpr double CURRENT_SHARE = 0.9;
        constexpr double FRICTION_SHARE = 0.85;
        constexpr int CURRENT_ITERATIONS = 5;
        /* RList[].ScndAct of a position that sets no field shunt of its own */
        constexpr int NO_SHUNT = 255;
        constexpr double SECONDS_PER_HOUR_PER_KILOMETRE = 3.6;
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return 0.0;
        }
        int main_position = mover->MainCtrlActualPos;
        int shunt_position = mover->ScndCtrlActualPos;
        if (p_main_controller) {
            main_position += 1;
        } else {
            shunt_position += 1;
        }
        const TScheme &step = mover->RList[main_position];
        if (step.ScndAct < NO_SHUNT && mover->ScndCtrlActualPos == 0) {
            shunt_position = step.ScndAct;
        }
        const double friction_max = mover->Mass * g * mover->Adhesive(mover->RunningTrack.friction) * FRICTION_SHARE;
        double current = mover->Imax;
        for (int i = 0; i < CURRENT_ITERATIONS; i++) {
            const double momentum = mover->MomentumF(current, current, shunt_position);
            const double force_max = momentum * step.Bn * step.Mn * 2 / mover->WheelDiameter * mover->Transmision.Ratio;
            if (force_max == 0.0) {
                current = std::numeric_limits<double>::max();
                break;
            }
            current = 0.5 * current * (1 + (friction_max / force_max));
        }
        current = std::min(current, mover->Imax * CURRENT_SHARE);
        const double resistance = step.R + mover->CircuitRes + (step.Mn * mover->WindingRes);
        const TMotorParameters &motor = mover->MotorParam[shunt_position];
        const double flux =
                motor.fi * std::max((std::abs(current) / (std::abs(current) + motor.Isat)) - motor.fi0, 0.0);
        const double voltage = std::abs(mover->EngineVoltage) - (current * resistance);
        const double revolutions = std::max(0.0, voltage / (flux * step.Mn));
        return revolutions * mover->WheelDiameter * Math::PI * SECONDS_PER_HOUR_PER_KILOMETRE /
               mover->Transmision.Ratio;
    }
} // namespace godot
