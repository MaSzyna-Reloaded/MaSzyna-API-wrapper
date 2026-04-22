#ifndef RAILVEHICLEMODULE_HPP
#define RAILVEHICLEMODULE_HPP

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class RailVehicle;

    class RailVehicleModule : public Resource {
            GDCLASS(RailVehicleModule, Resource);

        protected:
            static void _bind_methods();
            RailVehicle *rail_vehicle = nullptr;
            bool initialized = false;

            virtual void _initialize();
            virtual void _deinitialize();
            virtual void _update(double delta);

        public:
            ~RailVehicleModule() override = default;

            virtual void set_rail_vehicle(RailVehicle *p_rail_vehicle);
            RailVehicle *get_rail_vehicle() const;
            bool is_initialized() const;
            void initialize();
            void deinitialize();
            virtual void update(double delta);
            virtual Dictionary get_supported_commands();
    };
} // namespace godot

#endif // RAILVEHICLEMODULE_HPP
