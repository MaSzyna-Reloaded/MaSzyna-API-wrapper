#ifndef DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_EDITOR_H
#define DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_EDITOR_H

#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

class DieselEngineMasterControllerPowerItemEditor : public EditorProperty {
        GDCLASS(DieselEngineMasterControllerPowerItemEditor, EditorProperty);

    private:
        VBoxContainer *main_container;

        SpinBox *rpm_spin_box;
        SpinBox *generator_power_spin_box;
        SpinBox *voltage_max_spin_box;
        SpinBox *current_max_spin_box;

        void _on_rpm_changed(double value);
        void _on_generator_power_changed(double value);
        void _on_voltage_max_changed(double value);
        void _on_current_max_changed(double value);

    protected:
        static void _bind_methods();

    public:
        DieselEngineMasterControllerPowerItemEditor();
        ~DieselEngineMasterControllerPowerItemEditor();

        void _update_property() override;
        void _set_read_only(bool p_read_only) override;
};

#endif // DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_EDITOR_H
