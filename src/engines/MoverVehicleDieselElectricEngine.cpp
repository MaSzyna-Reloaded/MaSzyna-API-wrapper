#include "MoverVehicleDieselElectricEngine.hpp"

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
        return traction.get_motor_current(get_mover());
    }

    double MoverVehicleDieselElectricEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(get_mover());
    }

    bool MoverVehicleDieselElectricEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(get_mover());
    }

    bool MoverVehicleDieselElectricEngine::get_fuse_active() const {
        return traction.get_fuse_active(get_mover());
    }

    bool MoverVehicleDieselElectricEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(get_mover());
    }

    void MoverVehicleDieselElectricEngine::fuse_reset() {
        traction.reset_fuse(get_mover());
    }

    void MoverVehicleDieselElectricEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(get_mover(), p_open);
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
