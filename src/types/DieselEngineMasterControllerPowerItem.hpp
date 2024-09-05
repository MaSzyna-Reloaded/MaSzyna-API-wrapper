// diesel_engine_master_controller_power_item.hpp
#ifndef DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_HPP
#define DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_HPP

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class DieselEngineMasterControllerPowerItem : public Object {
        GDCLASS(DieselEngineMasterControllerPowerItem, Object);

    private:
        float rpm;
        float generator_power;
        float voltage_max;
        float current_max;

    public:
        DieselEngineMasterControllerPowerItem();

        float get_rpm() const;
        void set_rpm(float p_rpm);

        float get_generator_power() const;
        void set_generator_power(float p_generator_power);

        float get_voltage_max() const;
        void set_voltage_max(float p_voltage_max);

        float get_current_max() const;
        void set_current_max(float p_current_max);

    protected:
        static void _bind_methods();
};

#endif // DIESEL_ENGINE_MASTER_CONTROLLER_POWER_ITEM_HPP
