#include "MoverVehicleElectricSeriesEngine.hpp"
#include "../mover/MoverBackend.hpp"

namespace godot {
    void MoverVehicleElectricSeriesEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverVehicleElectricSeriesEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverVehicleElectricSeriesEngine::get_circuit_imax);
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &MoverVehicleElectricSeriesEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverVehicleElectricSeriesEngine::get_fuse_active);
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &MoverVehicleElectricSeriesEngine::get_motor_connectors_open);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverVehicleElectricSeriesEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("set_motor_connectors_open", "open"), &MoverVehicleElectricSeriesEngine::set_motor_connectors_open);
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
} // namespace godot
