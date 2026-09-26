#include "../mover/MoverBackend.hpp"
#include "../mover/MoverTypes.hpp"
#include "MoverVehicleLighting.hpp"
#include "maszyna/utilities.h"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleLighting::_bind_methods() {}


    void MoverVehicleLighting::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();
        // readLightsList (Mover.cpp:8558) - one row per preset, the table holds LIGHTS_LIST_CAPACITY
        const int presets = std::min(static_cast<int>(light_position_list.size()), LIGHTS_LIST_CAPACITY);
        for (int preset = 0; preset < presets; ++preset) {
            const Ref<LightListItem> item = light_position_list[preset];
            if (item.is_null()) {
                continue;
            }
            // the row as the Mover's light bits (enum light, MOVER.h:189), cabin A's end then cabin B's
            p_mover->Lights[Maszyna::end::front][preset] =
                    (item->get_cabin_a_head_light() ? Maszyna::light::headlight_upper : 0) |
                    (item->get_cabin_a_left_white_signal() ? Maszyna::light::headlight_left : 0) |
                    (item->get_cabin_a_left_red_signal() ? Maszyna::light::redmarker_left : 0) |
                    (item->get_cabin_a_right_white_signal() ? Maszyna::light::headlight_right : 0) |
                    (item->get_cabin_a_right_red_signal() ? Maszyna::light::redmarker_right : 0) |
                    (item->get_cabin_a_end_signals() ? Maszyna::light::rearendsignals : 0) |
                    (item->get_cabin_a_left_auxiliary_light() ? Maszyna::light::auxiliary_left : 0) |
                    (item->get_cabin_a_right_auxiliary_light() ? Maszyna::light::auxiliary_right : 0);
            p_mover->Lights[Maszyna::end::rear][preset] =
                    (item->get_cabin_b_head_light() ? Maszyna::light::headlight_upper : 0) |
                    (item->get_cabin_b_left_white_signal() ? Maszyna::light::headlight_left : 0) |
                    (item->get_cabin_b_left_red_signal() ? Maszyna::light::redmarker_left : 0) |
                    (item->get_cabin_b_right_white_signal() ? Maszyna::light::headlight_right : 0) |
                    (item->get_cabin_b_right_red_signal() ? Maszyna::light::redmarker_right : 0) |
                    (item->get_cabin_b_end_signals() ? Maszyna::light::rearendsignals : 0) |
                    (item->get_cabin_b_left_auxiliary_light() ? Maszyna::light::auxiliary_left : 0) |
                    (item->get_cabin_b_right_auxiliary_light() ? Maszyna::light::auxiliary_right : 0);
        }
        p_mover->LightsPosNo = presets;
        p_mover->LightsWrap = get_lights_wrap_selector();
        // the selector starts at LightsDefPos, set by CheckLocomotiveParameters (Mover.cpp:8885)
        p_mover->LightsDefPos = get_lights_default_selector_position();
        p_mover->LightPowerSource.SourceType = mover_power_source(get_light_source());
        p_mover->AlterLightPowerSource.SourceType = mover_power_source(get_light_alternative_source());
    }

    bool MoverVehicleLighting::_light_enabled(
            const TMoverParameters *p_mover, const LightEnd p_end, const LightType p_type) const {
        const int lights = p_mover->iLights[static_cast<int>(light_end_map.at(p_end))];
        return (lights & light_type_mask_map.at(p_type)) != 0;
    }

    MoverVehicleLighting::LightEnd MoverVehicleLighting::_active_end(const TMoverParameters *p_mover) {
        return p_mover->CabActive < 0 ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    MoverVehicleLighting::LightEnd MoverVehicleLighting::_opposite_end(const TMoverParameters *p_mover) {
        return _active_end(p_mover) == LIGHT_END_FRONT ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    bool MoverVehicleLighting::_is_powered(const TMoverParameters *p_mover) {
        return p_mover->Power24vIsAvailable || p_mover->Power110vIsAvailable;
    }


    bool MoverVehicleLighting::get_roof_light_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr && roof_light_active && _is_powered(mover);
    }

    int MoverVehicleLighting::get_position() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->LightsPos : 0;
    }

    double MoverVehicleLighting::get_power() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? mover->LightPower : 0.0;
    }

    int MoverVehicleLighting::get_power_source() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? power_source_of_mover(mover->LightPowerSource.SourceType) : 0;
    }

    bool MoverVehicleLighting::get_front_headlight_upper_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_front_headlight_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_front_headlight_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_front_redmarker_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_front_redmarker_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_upper_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_rear_redmarker_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_rear_redmarker_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_upper_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_active_redmarker_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_active_redmarker_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_upper_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_opposite_redmarker_left_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_opposite_redmarker_right_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_devices_light_enabled() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr ? devices_light_active && _is_powered(mover) : false;
    }

    double MoverVehicleLighting::get_roof_light_level() const {
        const TMoverParameters *mover = get_mover();
        return mover != nullptr
                       ? roof_light_active && _is_powered(mover) ? (mover->Power110vIsAvailable ? 1.0 : 0.5) : 0.0
                       : 0.0;
    }

    void MoverVehicleLighting::_fill_state_dictionary(Dictionary &p_state) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["light_position"] = get_position();
        // what lights_sw shows - LightsPos runs from 1 (Train.cpp:5220)
        p_state["light_selector_position"] = std::max(0, get_position() - 1);
        p_state["headlights_dimmed"] = get_headlights_dimmed();
        p_state["light_power"] = get_power();
        p_state["light_power_source"] = get_power_source();
        p_state["lights/front_headlight_upper_enabled"] = get_front_headlight_upper_enabled();
        p_state["lights/front_headlight_left_enabled"] = get_front_headlight_left_enabled();
        p_state["lights/front_headlight_right_enabled"] = get_front_headlight_right_enabled();
        p_state["lights/front_redmarker_left_enabled"] = get_front_redmarker_left_enabled();
        p_state["lights/front_redmarker_right_enabled"] = get_front_redmarker_right_enabled();
        p_state["lights/rear_headlight_upper_enabled"] = get_rear_headlight_upper_enabled();
        p_state["lights/rear_headlight_left_enabled"] = get_rear_headlight_left_enabled();
        p_state["lights/rear_headlight_right_enabled"] = get_rear_headlight_right_enabled();
        p_state["lights/rear_redmarker_left_enabled"] = get_rear_redmarker_left_enabled();
        p_state["lights/rear_redmarker_right_enabled"] = get_rear_redmarker_right_enabled();
        p_state["lights/active_headlight_upper_enabled"] = get_active_headlight_upper_enabled();
        p_state["lights/active_headlight_left_enabled"] = get_active_headlight_left_enabled();
        p_state["lights/active_headlight_right_enabled"] = get_active_headlight_right_enabled();
        p_state["lights/active_redmarker_left_enabled"] = get_active_redmarker_left_enabled();
        p_state["lights/active_redmarker_right_enabled"] = get_active_redmarker_right_enabled();
        p_state["lights/opposite_headlight_upper_enabled"] = get_opposite_headlight_upper_enabled();
        p_state["lights/opposite_headlight_left_enabled"] = get_opposite_headlight_left_enabled();
        p_state["lights/opposite_headlight_right_enabled"] = get_opposite_headlight_right_enabled();
        p_state["lights/opposite_redmarker_left_enabled"] = get_opposite_redmarker_left_enabled();
        p_state["lights/opposite_redmarker_right_enabled"] = get_opposite_redmarker_right_enabled();
        p_state["roof_light_enabled"] = get_roof_light_enabled();
        p_state["devices_light_enabled"] = get_devices_light_enabled();
        p_state["roof_light_level"] = get_roof_light_level();
    }

    void MoverVehicleLighting::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        VehicleComponent::_fill_config_dictionary(p_config);
        // lights_sw: shows LightsPos - 1 (Train.cpp:5220), one position per preset
        p_config["light_position_max"] = std::max(0, mover->LightsPosNo - 1);
    }

    // Original engine: TTrain::OnCommand_lightspresetactivatenext (Train.cpp:5193) - LightsPos runs
    // 1..LightsPosNo, and wraps around only with LightsWrap
    void MoverVehicleLighting::increase_light_selector_position() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (mover->LightsPosNo == 0) {
            return;
        }
        if (mover->LightsPos < mover->LightsPosNo || mover->LightsWrap) {
            mover->LightsPos = mover->LightsPos >= mover->LightsPosNo ? 1 : mover->LightsPos + 1;
            _set_lights(mover);
        }
    }

    // Original engine: TTrain::OnCommand_lightspresetactivateprevious (Train.cpp:5231)
    void MoverVehicleLighting::decrease_light_selector_position() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        if (mover->LightsPosNo == 0) {
            return;
        }
        if (mover->LightsPos > 1 || mover->LightsWrap) {
            mover->LightsPos = mover->LightsPos <= 1 ? mover->LightsPosNo : mover->LightsPos - 1;
            _set_lights(mover);
        }
    }

    /* Original engine: TDynamicObject::SetLights (DynObj.cpp:7322) with the iLights part of
     * RaLightsSet (DynObj.cpp:7350). The preset of the occupied cab lights every vehicle joined to
     * it by a control coupling: the head preset on the end facing the head of the train, the rear
     * one on the other, and nothing on an end coupled to another vehicle. The model's own lamp
     * inventory (iInventory) is not known here, so a rear end that could show either red markers
     * or plates shows the markers. */
    void MoverVehicleLighting::_set_lights(TMoverParameters *p_mover) const {
        const int lead_end = p_mover->CabOccupied >= 0 ? Maszyna::end::front : Maszyna::end::rear;
        const int preset = p_mover->LightsPos - 1;
        const int automatic_markers =
                p_mover->CabActive == 0 && (p_mover->InactiveCabFlag & Maszyna::activation::redmarkers) != 0
                        ? Maszyna::light::redmarker_left + Maszyna::light::redmarker_right
                        : 0;
        const int head_lights = automatic_markers > 0 ? automatic_markers : p_mover->Lights[lead_end][preset];
        const int rear_lights = automatic_markers > 0 ? automatic_markers : p_mover->Lights[1 - lead_end][preset];
        const int end_of_train =
                Maszyna::light::redmarker_left | Maszyna::light::redmarker_right | Maszyna::light::rearendsignals;

        // GetFirstDynamic(): out through the head end to the last vehicle joined by control
        TMoverParameters *vehicle = p_mover;
        int head_end = lead_end;
        while (vehicle->Couplers[head_end].Connected != nullptr &&
               Maszyna::TestFlag(vehicle->Couplers[head_end].CouplingFlag, Maszyna::coupling::control)) {
            const int entered = vehicle->Couplers[head_end].ConnectedNr;
            vehicle = vehicle->Couplers[head_end].Connected;
            head_end = 1 - entered;
        }
        while (vehicle != nullptr) {
            const int tail_end = 1 - head_end;
            // Original engine: LightsPos 18 lights both ends whatever is coupled (DynObj.cpp:7337)
            const bool lights_all_ends = p_mover->LightsPos == LIGHTS_POSITION_ALL_ENDS;
            const bool head_free =
                    lights_all_ends || vehicle->Couplers[head_end].Connected == nullptr ||
                    !Maszyna::TestFlag(vehicle->Couplers[head_end].CouplingFlag, Maszyna::coupling::coupler);
            const bool tail_free =
                    lights_all_ends || vehicle->Couplers[tail_end].Connected == nullptr ||
                    !Maszyna::TestFlag(vehicle->Couplers[tail_end].CouplingFlag, Maszyna::coupling::coupler);
            int rear = tail_free ? rear_lights : 0;
            // a powered vehicle with no direction set shows plates, not lights (DynObj.cpp:7360)
            if (rear == end_of_train && vehicle->Power > 1.0 && vehicle->DirActive == 0) {
                rear = Maszyna::light::rearendsignals;
            }
            if (rear == end_of_train) {
                rear = Maszyna::light::redmarker_left | Maszyna::light::redmarker_right;
            }
            vehicle->iLights[head_end] = head_free ? head_lights : 0;
            vehicle->iLights[tail_end] = rear;
            if (vehicle->Couplers[tail_end].Connected == nullptr ||
                !Maszyna::TestFlag(vehicle->Couplers[tail_end].CouplingFlag, Maszyna::coupling::control)) {
                break;
            }
            const int entered = vehicle->Couplers[tail_end].ConnectedNr;
            vehicle = vehicle->Couplers[tail_end].Connected;
            head_end = entered;
        }
    }

    // Original engine: TTrain::OnCommand_headlightsdimenable/disable (Train.cpp:6146-6195)
    void MoverVehicleLighting::headlights_dim(const bool p_enabled) {
        headlights_dimmed = p_enabled;
    }

    bool MoverVehicleLighting::get_headlights_dimmed() const {
        return headlights_dimmed;
    }

    namespace {
        struct LightBit {
                const char *name;
                MoverVehicleLighting::LightEnd end;
                MoverVehicleLighting::LightType type;
        };

        const LightBit LIGHT_BITS[] = {
                {"front_headlight_upper", MoverVehicleLighting::LIGHT_END_FRONT,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"front_headlight_left", MoverVehicleLighting::LIGHT_END_FRONT,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"front_headlight_right", MoverVehicleLighting::LIGHT_END_FRONT,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"front_redmarker_left", MoverVehicleLighting::LIGHT_END_FRONT,
                 MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"front_redmarker_right", MoverVehicleLighting::LIGHT_END_FRONT,
                 MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
                {"rear_headlight_upper", MoverVehicleLighting::LIGHT_END_REAR,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"rear_headlight_left", MoverVehicleLighting::LIGHT_END_REAR,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"rear_headlight_right", MoverVehicleLighting::LIGHT_END_REAR,
                 MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"rear_redmarker_left", MoverVehicleLighting::LIGHT_END_REAR,
                 MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rear_redmarker_right", MoverVehicleLighting::LIGHT_END_REAR,
                 MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    void MoverVehicleLighting::light(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);

        for (const LightBit &bit: LIGHT_BITS) {
            if (p_light == bit.name) {
                const int end = static_cast<int>(light_end_map.at(bit.end));
                const int mask = light_type_mask_map.at(bit.type);
                if (p_enabled) {
                    mover->iLights[end] |= mask;
                } else {
                    mover->iLights[end] &= ~mask;
                }
                return;
            }
        }
        UtilityFunctions::push_warning("MoverVehicleLighting::light() unknown light name: " + p_light);
    }

    namespace {
        struct LightSwitchMask {
                const char *suffix;
                MoverVehicleLighting::LightType type;
        };

        const LightSwitchMask LIGHT_SWITCH_MASKS[] = {
                {"upper", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"left", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"right", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"leftend", MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rightend", MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    // Confirmed against vehicle/Train.cpp:5267-5316 (OnCommand_headlighttoggleleft/enableleft) -
    // upperlight_sw:/leftlight_sw:/rightlight_sw:/leftend_sw:/rightend_sw: (p_light without a
    // "rear" prefix) toggle whichever end is the CURRENTLY ACTIVE cab's own front
    // (Train->cab_to_end()); rearupperlight_sw:/etc. (p_light with a "rear" prefix, stripped
    // here) toggle the opposite end - see this class's own _do_fetch_state_from_mover() for the
    // matching active_end/opposite_end state this mirrors. NOTE: a real nuance from the original
    // is deliberately NOT reproduced here - OnCommand_headlightenableleft also clears the
    // matching redmarker when the vehicle has no separate *end_sw: declared (a 3-way switch
    // subsuming the marker light), which would require cross-widget awareness this catalog-driven
    // instancer doesn't have.
    void MoverVehicleLighting::light_switch(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);

        const bool is_rear = p_light.begins_with("rear");
        const String suffix = is_rear ? p_light.substr(4) : p_light;
        const LightEnd active_end = (mover->CabActive < 0) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const LightEnd opposite_end = (active_end == LIGHT_END_FRONT) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const LightEnd target_end = is_rear ? opposite_end : active_end;

        for (const LightSwitchMask &entry: LIGHT_SWITCH_MASKS) {
            if (suffix == entry.suffix) {
                const int end = static_cast<int>(light_end_map.at(target_end));
                const int mask = light_type_mask_map.at(entry.type);
                if (p_enabled) {
                    mover->iLights[end] |= mask;
                } else {
                    mover->iLights[end] &= ~mask;
                }
                return;
            }
        }
        UtilityFunctions::push_warning("MoverVehicleLighting::light_switch() unknown light name: " + p_light);
    }

    void MoverVehicleLighting::roof_light(const bool p_enabled) {
        roof_light_active = p_enabled;
    }

    void MoverVehicleLighting::devices_light(const bool p_enabled) {
        devices_light_active = p_enabled;
    }

} // namespace godot
