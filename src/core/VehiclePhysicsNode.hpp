#pragma once
#include "VehicleController.hpp"
#include "VehicleModel.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    /* A vehicle's presence in the scene tree.
     *
     * The vehicle itself is an object of the server, addressed by a RID; this node owns that
     * handle, builds the vehicle from a VehicleModel and frees it. It is the anchor everything in
     * the tree hangs off - a scripted component a modder adds, a cabin, a sound bank - so that
     * "which vehicle am I part of" is answered by where a node sits, not by a path it carries.
     *
     * It knows nothing about where the model came from. A subclass brings one: FizVehiclePhysicsNode
     * asks the .fiz builder for it, and another format would be another subclass. */
    class VehiclePhysicsNode : public Node {
            GDCLASS(VehiclePhysicsNode, Node)

        private:
            RID vehicle_rid;
            VehicleController *controller = nullptr;
            Ref<VehicleModel> model;
            void _build(const Ref<VehicleModel> &p_model);
            String train_id;
            double initial_velocity = 0.0;
            VehicleController::DriverType driver_type = VehicleController::DRIVER_NOBODY;
            String load_name;
            double load_amount = 0.0;

        protected:
            static void _bind_methods();
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)

        public:
            static const char *vehicle_changed_signal;

            /* Builds the vehicle this model describes, replacing whatever this node held. */
            void set_model(const Ref<VehicleModel> &p_model);
            Ref<VehicleModel> get_model() const;

            /* This vehicle's handle, for anything that talks to the servers */
            RID get_vehicle_rid() const;
            /* The vehicle in the simulation. Null until a model is set. */
            VehicleController *get_controller() const;

            /* Adds a component to this vehicle - what a proxy node in the tree calls when it
             * joins, so a modder's component reaches the vehicle it sits under. */
            void add_component(VehicleComponent *p_component);

            /* Set on the vehicle every time one is built. Not derived from whatever file the
             * model came from - a scenery names its vehicles, a .fiz does not. */
            void set_train_id(const String &p_train_id);
            String get_train_id() const;
            void set_initial_velocity(double p_velocity);
            double get_initial_velocity() const;
            void set_driver_type(VehicleController::DriverType p_driver_type);
            VehicleController::DriverType get_driver_type() const;
            void set_load_name(const String &p_load_name);
            String get_load_name() const;
            void set_load_amount(double p_load_amount);
            double get_load_amount() const;
    };
} // namespace godot
