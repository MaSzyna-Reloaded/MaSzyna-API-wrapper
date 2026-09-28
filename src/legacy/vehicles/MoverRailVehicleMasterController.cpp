#include "legacy/vehicles/MoverBackend.hpp"
#include "MoverRailVehicleMasterController.hpp"

namespace godot {
    void MoverRailVehicleMasterController::_bind_methods() {}

    void MoverRailVehicleMasterController::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();

        p_mover->MainCtrlPosNo = get_main_position_count();
        p_mover->ScndCtrlPosNo = get_second_position_count();
        p_mover->MainCtrlMaxDirChangePos = get_direction_change_max_position();
        p_mover->CoupledCtrl = get_coupled_controllers();
        p_mover->InitialCtrlDelay = get_initial_delay();
        p_mover->CtrlDelay = get_step_delay();
        p_mover->CtrlDownDelay = get_step_down_delay();
    }

    void MoverRailVehicleMasterController::_fill_config_dictionary(Dictionary &p_config) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        VehicleComponent::_fill_config_dictionary(p_config);
        p_config["main_controller_position_max"] = mover->MainCtrlPosNo;
        p_config["second_controller_position_max"] = mover->ScndCtrlPosNo;
        // the cab's master controller: with a coupled controller its shaft goes on into the field
        // shunt past the last main position (Train.cpp:985, 1133; Mover.cpp:2335)
        p_config["master_controller_position_max"] =
                mover->CoupledCtrl ? mover->MainCtrlPosNo + mover->ScndCtrlPosNo : mover->MainCtrlPosNo;
    }

    /* Where the cab's master controller stands - the shunt steps counted on with a coupled
     * controller (Train.cpp:9410) */
    void MoverRailVehicleMasterController::_fill_state_dictionary(Dictionary &p_state) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        VehicleComponent::_fill_state_dictionary(p_state);
        p_state["master_controller_position"] =
                mover->CoupledCtrl ? mover->MainCtrlPos + mover->ScndCtrlPos : mover->MainCtrlPos;
    }
} // namespace godot
