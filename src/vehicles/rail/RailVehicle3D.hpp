#pragma once

#include "vehicles/base/VehicleController.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>
#include <vector>

namespace godot {
    class Cabin3D;
    class RailVehicleBuffCoupl;
    class RailVehicleController;
    class VehiclePhysicsNode;
    class Area3D;
    class RailVehicleElectricEngine;
    class RailVehicleDieselEngine;
    class RailVehicleLighting;
    class RailVehicleWipers;
    class RailVehicleDoors;
    class VisibleOnScreenNotifier3D;

    class RailVehicle3D : public Node3D {
            GDCLASS(RailVehicle3D, Node3D)

        public:
            static const char *controller_changed_signal;

        private:
            NodePath model_instance_path;
            TypedDictionary<String, bool> lights;
            /// What the model's headlamps were last told about the headlights' dimming
            bool headlights_dimmed = false;
            NodePath controller_path;
            NodePath front_bogie_path;
            NodePath rear_bogie_path;
            TypedArray<NodePath> front_rolling_wheel_paths;
            TypedArray<NodePath> powered_wheel_paths;
            TypedArray<NodePath> rear_rolling_wheel_paths;
            /* The slider's width as the model gives it, handed to RailVehicleServer with the arms'
             * geometry for a vehicle whose FIZ declares none (CSW) */
            double pantograph_collector_width = 0.5;
            TypedArray<NodePath> pantograph_front_arm_paths;
            TypedArray<NodePath> pantograph_rear_arm_paths;
            // three per wiper (arm 1, arm 2, blade), an empty path for a missing one
            TypedArray<NodePath> wiper_arm_paths;
            // the mirrors in the original's order, <animmirrorprefix><number> from 1 - odd on the
            // left, even on the right (DynObj.cpp:5887-5910), an empty path for a missing one
            TypedArray<NodePath> mirror_paths;
            // coupler/hose submodels by lowercased name, e.g. "cpneumatic1r_on" (DynObj.cpp:2170-2181)
            Dictionary coupler_submodel_paths;
            String start_track_name;
            double start_track_offset = 0.0;
            int start_direction = 0;
            Ref<PackedScene> cabin_scene;
            bool cabin_rotate_180deg = false;
            bool joint_cabs = false;
            NodePath low_poly_cabin_path;
            /* The cargo the scenery gave this vehicle, drawn as its own model. It sits lower the
             * emptier the vehicle is, so where it sits is not known until the vehicle's
             * configuration has landed - see _apply_load_offset(). */
            NodePath load_model_path;
            ObjectID load_model_id;
            double low_poly_cabin_emission_energy = 0.2;
            double low_poly_cabin_emission_fade_time = 0.2;
            NodePath head_display_e3d_path;
            Ref<Material> head_display_material;
            NodePath head_display_node_path;

            bool dirty = true;
            bool needs_head_display_update = false;
            /* A node living in the scene tree keeps no pointer to another object. The vehicle is
             * its `rid`, and what this node draws is read from RailVehicleServer by it where it is
             * used (_component()): the components are the server's business, not this node's.
             * Every node - the model, the nodes of its tree, the cab - is held by its ObjectID and
             * resolved where it is used (_object()), since it may be freed under this one at any
             * time (a model rebuilt, "Edit FIZ" re-adding the vehicle - FINDINGS.md 2026-09-30). */
            ObjectID head_display_e3d_id;
            ObjectID cabin_id;
            /* The vehicle whose signals this node listens to (config_changed, roof_light_changed) -
             * held only to connect and disconnect them */
            ObjectID controller_id;
            ObjectID fiz_controller_id;
            ObjectID model_node_id;
            ObjectID detection_area_id;
            ObjectID visibility_notifier_id;
            bool is_visible = true;
            /// Fallback for maszyna/vehicles/detail_distance
            static constexpr float DEFAULT_VEHICLE_DETAIL_DISTANCE_M = 350.0;
            /// maszyna/vehicles/detail_distance is the distance the node hierarchy is
            /// dropped at; it is taken back this much closer, so a vehicle sitting on the boundary
            /// is not rebuilt over and over. Proportional, because a fixed margin is either nothing
            /// at a long detail distance or larger than the distance itself at a short one.
            static constexpr float VEHICLE_DETAIL_HYSTERESIS = 0.25;
            static constexpr float VEHICLE_DETAIL_HYSTERESIS_MIN_M = 25.0;
            /// The model currently uses the node hierarchy (bogies, wheels, pantograph arms)
            bool model_detailed = true;
            /// The model registered with SceneryHUDMouseServer - only while it is detailed, so
            /// only the vehicles near the camera are tested under the cursor
            RID pickable;
            bool force_detail_refresh = true;
            Transform3D last_body_transform;
            ObjectID low_poly_cabin_id;
            TypedArray<ShaderMaterial> low_poly_emissive_materials;
            Ref<Tween> low_poly_emission_tween;
            /* The vehicle this node draws - the VehiclePhysicsNode's handle, taken when it built it */
            RID rid;
            bool pending_start_track_retry = false;
            double update_time = 0.0;
            bool animation_bindings_dirty = true;
            ObjectID front_bogie_node_id;
            ObjectID rear_bogie_node_id;
            Vector<ObjectID> front_rolling_wheel_nodes;
            Vector<ObjectID> powered_wheel_nodes;
            Vector<ObjectID> rear_rolling_wheel_nodes;
            HashMap<ObjectID, Basis> node_rest_bases;
            HashMap<ObjectID, Basis> bogie_rest_global_bases;
            bool bogie_configuration_warned = false;
            Vector<ObjectID> pantograph_front_arm_nodes;
            Vector<ObjectID> pantograph_rear_arm_nodes;
            Vector<ObjectID> wiper_arm_nodes;
            PackedFloat64Array wiper_applied_positions;
            struct MirrorNode {
                    ObjectID node;
                    bool right = false;
                    // at the front end of the model: the original's offset().z > 0 (DynObj.cpp:5903)
                    bool front = false;
            };
            std::vector<MirrorNode> mirror_nodes;
            double mirror_applied_left = -1.0;
            double mirror_applied_right = -1.0;
            int mirror_applied_cab = 0;

            /// The object behind `p_id`, or null once it has been freed or is of another class
            template<typename T>
            static T *_object(const ObjectID &p_id) {
                return Object::cast_to<T>(ObjectDB::get_instance(static_cast<uint64_t>(p_id)));
            }
            /* The vehicle's component of a kind, as RailVehicleServer hands it out by `rid` */
            template<typename T>
            Ref<T> _component(VehicleComponentType::Type p_type) const;
            static ObjectID _id_of(const Object *p_object) {
                return p_object != nullptr ? ObjectID(p_object->get_instance_id()) : ObjectID();
            }
            RailVehicleController *_resolve_controller(const NodePath &p_node_path) const;
            void _on_controller_changed(RailVehicleController *p_controller);
            void _on_vehicle_changed();
            void _bind_vehicle_node();
            Ref<RailVehicleBuffCoupl> _coupler() const;
            void _on_vehicle_config_changed();
            void _update_head_display();
            void _schedule_head_display_update();
            void _process_impl(double p_delta);
            void _process_dirty();
            void _sync_model_lights();
            void _sync_lights_from_controller();
            void _on_low_poly_cabin_e3d_loaded();
            void _update_low_poly_cabs_visibility();
            void _on_roof_light_changed(bool p_enabled);
            void _apply_load_offset();

            void _set_low_poly_emission_energy(double p_value);
            void _update_detection_area();
            void _on_screen_entered();
            void _on_screen_exited();
            void _on_track_server_tracks_changed();
            void _apply_start_track();
            Vector<ObjectID> _resolve_animation_nodes(const TypedArray<NodePath> &p_paths) const;
            void _capture_rest_basis(Node3D *p_node);
            Vector<ObjectID> _resolve_pantograph_arm_nodes(const TypedArray<NodePath> &p_paths) const;
            /* The pantograph as the model builds it, measured off its arm nodes and handed to
             * RailVehicleServer, which raises it - TAnimPant's lengths and angles (DynObj.cpp:5508-5549) */
            void _publish_pantograph_geometry(
                    RailVehicleElectricEngine::PantographSelector p_pantograph, const Vector<ObjectID> &p_nodes) const;
            void _cache_animation_bindings();
            HashMap<String, ObjectID> coupler_submodel_nodes;
            int64_t coupler_visibility_state = -1;
            int _air_coupler_status(const String &p_name) const;
            int _pneumatic_variant(int p_end, bool p_brake_hose) const;
            void _show_air_coupler(const String &p_name, bool p_on, bool p_xon);
            void _update_couplers();
            void _update_wipers();
            void _update_mirrors();
            void _apply_wheel_rotation(const Vector<ObjectID> &p_nodes, double p_angle_degrees);
            void _apply_node_rotation(const ObjectID &p_node, double p_angle_degrees);
            void _update_wheel_animation_state();
            void _update_model_detail();
            void _register_pickable(const RID &p_instance);
            void _update_smoke();
            /* The arms drawn as far as RailVehicleServer has raised them */
            void _update_pantograph_animation();
            void _apply_pantograph_animation(const Vector<ObjectID> &p_nodes, const Vector2 &p_raise);

        protected:
            static void _bind_methods();

        public:
            RailVehicle3D();

            void _enter_tree() override;
            void _ready() override;
            void _exit_tree() override;
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)

            void show_cabin();
            void hide_cabin();
            /// The cab interior while it is shown, else null
            Cabin3D *get_cabin() const;
            Ref<RailVehicleController> get_controller() const;
            /// This vehicle's handle in RailVehicleServer - the key anything
            /// keeping per-vehicle state of its own is meant to use.
            RID get_rid() const;
            /* Applies the placement vehicle's step just produced - the bogie
             * pivots, the body basis derived from them and the wheel animation. The server
             * calls it at the end of its tick, so nothing here renders a frame behind its
             * own physics. */
            void apply_track_placement();
            void move_on_track(double p_distance);
            void _on_model_node_e3d_loading();
            void _on_model_node_e3d_loaded();

            void set_model_instance_path(const NodePath &p_value);
            NodePath get_model_instance_path() const;
            void set_lights(const TypedDictionary<String, bool> &p_value);
            TypedDictionary<String, bool> get_lights() const;
            void set_controller_path(const NodePath &p_value);
            NodePath get_controller_path() const;
            void set_front_bogie_path(const NodePath &p_value);
            NodePath get_front_bogie_path() const;
            void set_rear_bogie_path(const NodePath &p_value);
            NodePath get_rear_bogie_path() const;
            void set_front_rolling_wheel_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_front_rolling_wheel_paths() const;
            void set_powered_wheel_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_powered_wheel_paths() const;
            void set_rear_rolling_wheel_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_rear_rolling_wheel_paths() const;
            void set_pantograph_collector_width(double p_value);
            double get_pantograph_collector_width() const;
            void set_pantograph_front_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_pantograph_front_arm_paths() const;
            void set_pantograph_rear_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_pantograph_rear_arm_paths() const;
            void set_wiper_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_wiper_arm_paths() const;
            void set_mirror_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_mirror_paths() const;
            void set_coupler_submodel_paths(const Dictionary &p_value);
            Dictionary get_coupler_submodel_paths() const;
            int get_pneumatic_layout(int p_end, bool p_brake_hose) const;
            void set_start_track_name(const String &p_value);
            String get_start_track_name() const;
            void set_start_track_offset(double p_value);
            double get_start_track_offset() const;
            void set_start_direction(int p_value);
            int get_start_direction() const;
            void set_cabin_scene(const Ref<PackedScene> &p_value);
            Ref<PackedScene> get_cabin_scene() const;
            void set_cabin_rotate_180deg(bool p_value);
            bool get_cabin_rotate_180deg() const;
            void set_joint_cabs(bool p_value);
            bool get_joint_cabs() const;
            void set_low_poly_cabin_path(const NodePath &p_value);
            NodePath get_low_poly_cabin_path() const;
            void set_load_model_path(const NodePath &p_value);
            NodePath get_load_model_path() const;
            void set_low_poly_cabin_emission_energy(double p_value);
            double get_low_poly_cabin_emission_energy() const;
            void set_low_poly_cabin_emission_fade_time(double p_value);
            double get_low_poly_cabin_emission_fade_time() const;
            void set_head_display_e3d_path(const NodePath &p_value);
            NodePath get_head_display_e3d_path() const;
            void set_head_display_material(const Ref<Material> &p_value);
            Ref<Material> get_head_display_material() const;
            void set_head_display_node_path(const NodePath &p_value);
            NodePath get_head_display_node_path() const;
    };
} // namespace godot
