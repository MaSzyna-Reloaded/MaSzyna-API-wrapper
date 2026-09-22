#include "VehicleWipers.hpp"

#include <algorithm>

namespace godot {
    void VehicleWipers::_bind_methods() {
        ClassDB::bind_method(D_METHOD("switch_increase"), &VehicleWipers::switch_increase);
        ClassDB::bind_method(D_METHOD("switch_decrease"), &VehicleWipers::switch_decrease);
        BIND_PROPERTY(VehicleWipers, Variant::FLOAT, angle);
        BIND_PROPERTY(VehicleWipers, Variant::INT, default_position);
        BIND_PROPERTY(VehicleWipers, Variant::INT, wiper_count);
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleWipers, Variant::ARRAY, positions, PROPERTY_HINT_TYPE_STRING, "WiperListItem");
    }

    void VehicleWipers::_register_commands() {
        register_command("wipers_switch_increase", Callable(this, "switch_increase"));
        register_command("wipers_switch_decrease", Callable(this, "switch_decrease"));
    }

    void VehicleWipers::_unregister_commands() {
        unregister_command("wipers_switch_increase", Callable(this, "switch_increase"));
        unregister_command("wipers_switch_decrease", Callable(this, "switch_decrease"));
    }

    void VehicleWipers::_do_update_internal_mover(TMoverParameters *p_mover) {
        size_t count = static_cast<size_t>(std::max(0, wiper_count));
        if (count == 0) {
            int mask = 0;
            for (int i = 0; i < positions.size(); i++) {
                const Ref<WiperListItem> item = positions[i];
                if (item.is_valid()) {
                    mask |= item->get_wiper_mask();
                }
            }
            while (mask > 0) {
                count++;
                mask >>= 1;
            }
        }
        if (count != wipers.size()) {
            wipers.assign(count, Wiper());
        }
        if (!switch_initialized) {
            // Mover.cpp:11942
            _set_switch_position(default_position);
            switch_initialized = positions.size() > 0;
        }
    }

    void VehicleWipers::_set_switch_position(const int p_position) {
        switch_position = std::clamp(p_position, 0, std::max(0, static_cast<int>(positions.size()) - 1));
    }

    void VehicleWipers::switch_increase() {
        _set_switch_position(switch_position + 1);
    }

    void VehicleWipers::switch_decrease() {
        _set_switch_position(switch_position - 1);
    }

    // DynObj.cpp:4048-4115
    void VehicleWipers::_do_process_mover(TMoverParameters *p_mover, const double p_delta) {
        if (positions.size() == 0) {
            return;
        }
        const Ref<WiperListItem> switched = positions[switch_position];
        const int count = static_cast<int>(wipers.size());
        for (int i = 0; i < count; i++) {
            Wiper &wiper = wipers[i];
            // the wipers are numbered from the side of the active cab
            bool active = false;
            if (switched.is_valid() && p_mover->Battery) {
                if (p_mover->CabActive == 1) {
                    active = (switched->get_wiper_mask() & (1 << i)) != 0;
                } else if (p_mover->CabActive == -1) {
                    active = (switched->get_wiper_mask() & (1 << (count - 1 - i))) != 0;
                }
            }

            wiper.out_timer += p_delta;
            wiper.park_timer += p_delta;
            if (!active && wiper.position <= 0.0) {
                continue;
            }

            // the parameters of the position the wiper started its sweep with
            const Ref<WiperListItem> working = positions[wiper.working_switch_position];
            if (working.is_null() || working->get_transit_time() <= 0.0) {
                continue;
            }
            const double step = p_delta / working->get_transit_time();

            if (!active) {
                // switched off on the way: back to the parked position at once
                wiper.position = std::max(0.0, wiper.position - step);
                continue;
            }
            if (wiper.position < 1.0 && !wiper.returning && wiper.park_timer > working->get_period()) {
                wiper.position = std::min(1.0, wiper.position + step);
            }
            if (wiper.position > 0.0 && wiper.returning && wiper.out_timer > working->get_return_delay()) {
                wiper.position = std::max(0.0, wiper.position - step);
            }
            if (wiper.position >= 1.0) {
                wiper.park_timer = 0.0;
                wiper.returning = true;
            }
            if (wiper.position <= 0.0) {
                wiper.out_timer = 0.0;
                wiper.returning = false;
                wiper.working_switch_position = switch_position;
            }
        }
    }

    void VehicleWipers::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        PackedFloat64Array wiper_positions;
        // 0..1 sweeping out, 1..2 on the way back - wiper_pos of the original scene uniforms
        // (opengl33renderer.cpp:762-766)
        for (const Wiper &wiper: wipers) {
            wiper_positions.push_back(wiper.position > 0.0 && wiper.returning ? wiper.position + 1.0 : wiper.position);
        }
        p_state["wipers_switch_position"] = switch_position;
        p_state["wiper_positions"] = wiper_positions;
    }

    void VehicleWipers::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config["wipers_switch_position_max"] = std::max(0, static_cast<int>(positions.size()) - 1);
        p_config["wipers_angle"] = angle;
    }
} // namespace godot
