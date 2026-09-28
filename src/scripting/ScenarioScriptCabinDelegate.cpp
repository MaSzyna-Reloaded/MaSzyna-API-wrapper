#include "ScenarioScriptCabinDelegate.hpp"

namespace godot {
    const char *ScenarioScriptCabinDelegate::control_changed_signal = "control_changed";

    void ScenarioScriptCabinDelegate::_bind_methods() {
        GDVIRTUAL_BIND(_act, "vehicle", "cab", "control_id", "action", "value");
        GDVIRTUAL_BIND(_get_control, "vehicle", "cab", "control_id");
        GDVIRTUAL_BIND(_get_controls, "vehicle", "cab");
        GDVIRTUAL_BIND(_get_occupied_cab, "vehicle");

        ADD_SIGNAL(MethodInfo(
                control_changed_signal, PropertyInfo(Variant::RID, "vehicle"), PropertyInfo(Variant::INT, "cab"),
                PropertyInfo(Variant::STRING_NAME, "control_id"), PropertyInfo(Variant::NIL, "value")));
    }

    Variant ScenarioScriptCabinDelegate::act(
            const RID &p_vehicle, const int p_cab, const StringName &p_control_id, const StringName &p_action,
            const Variant &p_value) {
        Variant result;
        GDVIRTUAL_CALL(_act, p_vehicle, p_cab, p_control_id, p_action, p_value, result);
        return result;
    }

    Variant ScenarioScriptCabinDelegate::get_control(
            const RID &p_vehicle, const int p_cab, const StringName &p_control_id) const {
        Variant result;
        GDVIRTUAL_CALL(_get_control, p_vehicle, p_cab, p_control_id, result);
        return result;
    }

    Array ScenarioScriptCabinDelegate::get_controls(const RID &p_vehicle, const int p_cab) const {
        Array result;
        GDVIRTUAL_CALL(_get_controls, p_vehicle, p_cab, result);
        return result;
    }

    int ScenarioScriptCabinDelegate::get_occupied_cab(const RID &p_vehicle) const {
        int result = 0;
        GDVIRTUAL_CALL(_get_occupied_cab, p_vehicle, result);
        return result;
    }
} // namespace godot
