#include "DieselEngineMasterControllerPowerItemEditor.hpp"


DieselEngineMasterControllerPowerItemEditor::DieselEngineMasterControllerPowerItemEditor() {
    main_container = memnew(VBoxContainer);
    add_child(main_container);

    // RPM SpinBox setup
    HBoxContainer *rpm_container = memnew(HBoxContainer);
    main_container->add_child(rpm_container);

    Label *rpm_label = memnew(Label);
    rpm_label->set_text("RPM:");
    rpm_container->add_child(rpm_label);

    rpm_spin_box = memnew(SpinBox);
    rpm_spin_box->set_step(0.1);
    rpm_spin_box->connect("value_changed", Callable(this, "_on_rpm_changed"));
    rpm_container->add_child(rpm_spin_box);

    // Generator Power SpinBox setup
    HBoxContainer *generator_power_container = memnew(HBoxContainer);
    main_container->add_child(generator_power_container);

    Label *generator_power_label = memnew(Label);
    generator_power_label->set_text("Generator Power:");
    generator_power_container->add_child(generator_power_label);

    generator_power_spin_box = memnew(SpinBox);
    generator_power_spin_box->set_step(0.1);
    generator_power_spin_box->connect("value_changed", Callable(this, "_on_generator_power_changed"));
    generator_power_container->add_child(generator_power_spin_box);

    // Voltage Max SpinBox setup
    HBoxContainer *voltage_max_container = memnew(HBoxContainer);
    main_container->add_child(voltage_max_container);

    Label *voltage_max_label = memnew(Label);
    voltage_max_label->set_text("Voltage Max:");
    voltage_max_container->add_child(voltage_max_label);

    voltage_max_spin_box = memnew(SpinBox);
    voltage_max_spin_box->set_step(0.1);
    voltage_max_spin_box->connect("value_changed", Callable(this, "_on_voltage_max_changed"));
    voltage_max_container->add_child(voltage_max_spin_box);

    // Current Max SpinBox setup
    HBoxContainer *current_max_container = memnew(HBoxContainer);
    main_container->add_child(current_max_container);

    Label *current_max_label = memnew(Label);
    current_max_label->set_text("Current Max:");
    current_max_container->add_child(current_max_label);

    current_max_spin_box = memnew(SpinBox);
    current_max_spin_box->set_step(0.1);
    current_max_spin_box->connect("value_changed", Callable(this, "_on_current_max_changed"));
    current_max_container->add_child(current_max_spin_box);
}

void DieselEngineMasterControllerPowerItemEditor::_set_read_only(bool p_read_only) {
    voltage_max_spin_box->set_editable(!p_read_only);
    current_max_spin_box->set_editable(!p_read_only);
    rpm_spin_box->set_editable(!p_read_only);
    generator_power_spin_box->set_editable(!p_read_only);
}

DieselEngineMasterControllerPowerItemEditor::~DieselEngineMasterControllerPowerItemEditor() {
    // Destructor
}

void DieselEngineMasterControllerPowerItemEditor::_on_rpm_changed(double value) {
    emit_changed(get_edited_property(), value);
}

void DieselEngineMasterControllerPowerItemEditor::_on_generator_power_changed(double value) {
    emit_changed(get_edited_property(), value);
}

void DieselEngineMasterControllerPowerItemEditor::_on_voltage_max_changed(double value) {
    emit_changed(get_edited_property(), value);
}

void DieselEngineMasterControllerPowerItemEditor::_on_current_max_changed(double value) {
    emit_changed(get_edited_property(), value);
}

void DieselEngineMasterControllerPowerItemEditor::_update_property() {
    Variant edited_object = get_edited_object();
    if (edited_object.get_type() == Variant::OBJECT) {
        Object *obj = edited_object.operator Object *();
        if (obj) {
            Dictionary data = obj->get(get_edited_property());
            rpm_spin_box->set_value(data["rpm"]);
            generator_power_spin_box->set_value(data["generator_power"]);
            voltage_max_spin_box->set_value(data["voltage_max"]);
            current_max_spin_box->set_value(data["current_max"]);
        }
    }
}

/*
void DieselEngineMasterControllerPowerItemEditor::_set_property(const Variant &p_value) {
    Dictionary data = p_value;
    rpm_spin_box->set_value(data["rpm"]);
    generator_power_spin_box->set_value(data["generator_power"]);
    voltage_max_spin_box->set_value(data["voltage_max"]);
    current_max_spin_box->set_value(data["current_max"]);
}

Variant DieselEngineMasterControllerPowerItemEditor::_get_property() const {
    Dictionary data;
    data["rpm"] = rpm_spin_box->get_value();
    data["generator_power"] = generator_power_spin_box->get_value();
    data["voltage_max"] = voltage_max_spin_box->get_value();
    data["current_max"] = current_max_spin_box->get_value();
    return data;
}*/

void DieselEngineMasterControllerPowerItemEditor::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_on_rpm_changed"), &DieselEngineMasterControllerPowerItemEditor::_on_rpm_changed);
    ClassDB::bind_method(
            D_METHOD("_on_generator_power_changed"),
            &DieselEngineMasterControllerPowerItemEditor::_on_generator_power_changed);
    ClassDB::bind_method(
            D_METHOD("_on_voltage_max_changed"), &DieselEngineMasterControllerPowerItemEditor::_on_voltage_max_changed);
    ClassDB::bind_method(
            D_METHOD("_on_current_max_changed"), &DieselEngineMasterControllerPowerItemEditor::_on_current_max_changed);
}
