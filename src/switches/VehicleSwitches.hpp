#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include "resources/switches/DimmerListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;

    /* Wraps the FIZ Switches: and DimmerList: sections.
     *
     * NOTE: the simulation keeps the pantograph, converter and line contactor switch types as
     * fields nothing reads, so setting them currently has no observable effect on it.
     * RelayResetButtonX=, PantographPresets=, PantographPresetDefault=, ModernDimmer= and
     * DimmerList: have no counterpart in the simulation at all - they are stored on this
     * component only, ready to be wired up if the simulation ever supports them. */
    class VehicleSwitches : public VehicleComponent {
            GDCLASS(VehicleSwitches, VehicleComponent);


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_SWITCHES;
            }

        private:
            static void _bind_methods();

        protected:
            void _fill_config_dictionary(Dictionary &p_config) const override;

        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual bool get_sand_active() const = 0;
            MAKE_MEMBER_GS(bool, pantograph_impulse, false);
            MAKE_MEMBER_GS(bool, converter_impulse, false);
            MAKE_MEMBER_GS(bool, motor_connectors_impulse, true);
            MAKE_MEMBER_GS(int, relay_reset_button_1, 0);
            MAKE_MEMBER_GS(int, relay_reset_button_2, 0);
            MAKE_MEMBER_GS(int, relay_reset_button_3, 0);
            MAKE_MEMBER_GS(PackedInt32Array, pantograph_presets, PackedInt32Array());
            MAKE_MEMBER_GS(int, pantograph_preset_default, 0);
            MAKE_MEMBER_GS(bool, modern_dimmer, false);
            MAKE_MEMBER_GS(bool, dimmer_list_cycle, false);
            MAKE_MEMBER_GS(int, dimmer_list_default_position, 0);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<DimmerListItem>, dimmer_list_positions)
            virtual void sand(bool p_active) = 0;
            void _register_commands() override;
            void _unregister_commands() override;
    };
} // namespace godot
