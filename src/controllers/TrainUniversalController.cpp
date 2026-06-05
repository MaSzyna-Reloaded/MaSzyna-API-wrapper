#include "TrainUniversalController.hpp"
namespace godot {
    void TrainUniversalController::_bind_methods() {
        BIND_PROPERTY(
                Variant::BOOL, "integrated_brake_pn", "integrated_brake_pn",
                &TrainUniversalController::set_integrated_brake_pn, &TrainUniversalController::get_integrated_brake_pn,
                "enabled");
        BIND_PROPERTY(
                Variant::BOOL, "integrated_brake", "integrated_brake", &TrainUniversalController::set_integrated_brake,
                &TrainUniversalController::get_integrated_brake, "enabled");
        BIND_PROPERTY(
                Variant::INT, "selector_position", "selector_position",
                &TrainUniversalController::set_selector_position, &TrainUniversalController::get_selector_position,
                "position");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                Variant::ARRAY, "uc_list", "uc_list", &TrainUniversalController::set_uc_list,
                &TrainUniversalController::get_uc_list, "uc_list", PROPERTY_HINT_TYPE_STRING, "UCListItem");
    }

    void TrainUniversalController::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        TrainPart::_do_update_internal_mover(p_mover);

        p_mover->UniCtrlIntegratedBrakePNCtrl = integrated_brake_pn;
        p_mover->UniCtrlIntegratedBrakeCtrl = integrated_brake;
        p_mover->UniCtrlListSize = static_cast<int>(uc_list.size());

        for (int i = 0; i < p_mover->UniCtrlListSize; ++i) {
            Ref<UCListItem> item = uc_list[i];
            if (item.is_valid() && i < sizeof(p_mover->UniCtrlList) / sizeof(p_mover->UniCtrlList[0])) {
                p_mover->UniCtrlList[i].mode = item->get_pneumatic_brake_position();
                p_mover->UniCtrlList[i].MinCtrlVal = item->get_min_percentage();
                p_mover->UniCtrlList[i].MaxCtrlVal = item->get_max_percentage();
                p_mover->UniCtrlList[i].SetCtrlVal = item->get_target_value();
                p_mover->UniCtrlList[i].SpeedUp = item->get_increase_speed();
                p_mover->UniCtrlList[i].SpeedDown = item->get_decrease_speed();
                p_mover->UniCtrlList[i].ReturnPosition = item->get_bounce_back_position();
                p_mover->UniCtrlList[i].NextPosFastInc = item->get_nearest_stable_up();
                p_mover->UniCtrlList[i].PrevPosFastDec = item->get_nearest_stable_down();
            }
        }

        p_mover->MainCtrlPos = selector_position; // Or whichever variable controls current position
        // Also possibly p_mover->MainCtrlPosNo?
    }

    void TrainUniversalController::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        ASSERT_MOVER(p_mover);
        p_state["selector_position"] = p_mover->MainCtrlPos;
    }

    void TrainUniversalController::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        TrainPart::_do_fetch_config_from_mover(p_mover, p_config);
    }
} // namespace godot
