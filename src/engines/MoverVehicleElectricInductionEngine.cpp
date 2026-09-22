#include "MoverVehicleElectricInductionEngine.hpp"

namespace godot {
    void MoverVehicleElectricInductionEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_motor_current"), &MoverVehicleElectricInductionEngine::get_motor_current);
        ClassDB::bind_method(D_METHOD("get_circuit_imax"), &MoverVehicleElectricInductionEngine::get_circuit_imax);
        ClassDB::bind_method(D_METHOD("get_dynamic_brake_active"), &MoverVehicleElectricInductionEngine::get_dynamic_brake_active);
        ClassDB::bind_method(D_METHOD("get_fuse_active"), &MoverVehicleElectricInductionEngine::get_fuse_active);
        ClassDB::bind_method(D_METHOD("get_motor_connectors_open"), &MoverVehicleElectricInductionEngine::get_motor_connectors_open);
        ClassDB::bind_method(D_METHOD("fuse_reset"), &MoverVehicleElectricInductionEngine::fuse_reset);
        ClassDB::bind_method(D_METHOD("set_motor_connectors_open", "open"), &MoverVehicleElectricInductionEngine::set_motor_connectors_open);
    }

    double MoverVehicleElectricInductionEngine::get_motor_current() const {
        return traction.get_motor_current(get_mover());
    }

    double MoverVehicleElectricInductionEngine::get_circuit_imax() const {
        return traction.get_circuit_imax(get_mover());
    }

    bool MoverVehicleElectricInductionEngine::get_dynamic_brake_active() const {
        return traction.get_dynamic_brake_active(get_mover());
    }

    bool MoverVehicleElectricInductionEngine::get_fuse_active() const {
        return traction.get_fuse_active(get_mover());
    }

    bool MoverVehicleElectricInductionEngine::get_motor_connectors_open() const {
        return traction.get_motor_connectors_open(get_mover());
    }

    void MoverVehicleElectricInductionEngine::fuse_reset() {
        traction.reset_fuse(get_mover());
    }

    void MoverVehicleElectricInductionEngine::set_motor_connectors_open(const bool p_open) {
        traction.open_motor_connectors(get_mover(), p_open);
    }

    void MoverVehicleElectricInductionEngine::_register_commands() {
        VehicleElectricInductionEngine::_register_commands();
        register_command("fuse_reset", Callable(this, "fuse_reset"));
        register_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }

    void MoverVehicleElectricInductionEngine::_unregister_commands() {
        VehicleElectricInductionEngine::_unregister_commands();
        unregister_command("fuse_reset", Callable(this, "fuse_reset"));
        unregister_command("motor_connectors_open", Callable(this, "set_motor_connectors_open"));
    }
} // namespace godot
