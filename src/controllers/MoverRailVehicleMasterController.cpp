#include "../mover/MoverBackend.hpp"
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
    }
} // namespace godot
