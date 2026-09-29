#include "MoverElectricTraction.hpp"
#include "legacy/vehicles/MoverBackend.hpp"
#include "vehicles/rail/RailVehicleEngine.hpp"

namespace godot {
    double MoverElectricTraction::get_motor_current(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Im : 0.0;
    }

    double MoverElectricTraction::get_circuit_imax(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Imax : 0.0;
    }

    bool MoverElectricTraction::get_dynamic_brake_active(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr && p_mover->DynamicBrakeFlag;
    }

    bool MoverElectricTraction::get_fuse_active(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr && p_mover->FuseFlag;
    }

    bool MoverElectricTraction::get_motor_connectors_open(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr && p_mover->StLinSwitchOff;
    }

    bool MoverElectricTraction::is_line_contactor_closed(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr && p_mover->StLinFlag;
    }

    bool MoverElectricTraction::is_pressure_switch_tripped(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr && p_mover->ControlPressureSwitch;
    }

    /* Original engine: OnCommand_motoroverloadrelayreset (Train.cpp:4061) calls this same FuseOn()
     * on press - "zbij nadmiarowy", clearing the overload trip (FuseFlag) that blocks
     * Mains/converter/compressor from re-enabling. */
    void MoverElectricTraction::reset_fuse(const RailVehicleEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        if (p_mover == nullptr) {
            return;
        }
        p_mover->FuseOn();
    }

    /* Original engine: OnCommand_motorconnectorsopen/close (Train.cpp:3947-4008) - a plain field
     * flip, no dedicated setter exists on the vendored Mover for this one. */
    void MoverElectricTraction::open_motor_connectors(const RailVehicleEngine *p_engine, const bool p_open) const {
        TMoverParameters *p_mover = owner.get_mover();
        if (p_mover == nullptr) {
            return;
        }
        p_mover->StLinSwitchOff = p_open;
    }
} // namespace godot
