#include "MoverVehicleDieselElectricEngine.hpp"
#include "../mover/MoverBackend.hpp"

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
        return traction.get_motor_current(mover_of(this));
    }

    double MoverVehicleDieselElectricEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(mover_of(this));
    }

    bool MoverVehicleDieselElectricEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(mover_of(this));
    }

    bool MoverVehicleDieselElectricEngine::get_fuse_active() const {
        return traction.get_fuse_active(mover_of(this));
    }

    bool MoverVehicleDieselElectricEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(mover_of(this));
    }

    void MoverVehicleDieselElectricEngine::fuse_reset() {
        traction.reset_fuse(mover_of(this));
    }

    void MoverVehicleDieselElectricEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(mover_of(this), p_open);
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
} // namespace godot
