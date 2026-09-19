#pragma once

#include "TrainController.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

namespace godot {
    class Area3D;
    class VisibleOnScreenNotifier3D;

    class RailVehicle3D : public Node3D {
            GDCLASS(RailVehicle3D, Node3D)

        private:

            NodePath model_instance_path;
            TypedDictionary<String, bool> lights;
            NodePath controller_path;
            NodePath front_bogie_path;
            NodePath rear_bogie_path;
            TypedArray<NodePath> front_rolling_wheel_paths;
            TypedArray<NodePath> powered_wheel_paths;
            TypedArray<NodePath> rear_rolling_wheel_paths;
            Vector3 pantograph_front_offset;
            Vector3 pantograph_rear_offset;
            double pantograph_collector_width = 0.5;
            TypedArray<NodePath> pantograph_front_arm_paths;
            TypedArray<NodePath> pantograph_rear_arm_paths;
            // coupler/hose submodels by lowercased name, e.g. "cpneumatic1r_on" (DynObj.cpp:2170-2181)
            Dictionary coupler_submodel_paths;
            String start_track_name;
            double start_track_offset = 0.0;
            int start_direction = 0;
            Ref<PackedScene> cabin_scene;
            bool cabin_rotate_180deg = false;
            bool joint_cabs = false;
            NodePath low_poly_cabin_path;
            double low_poly_cabin_emission_energy = 0.2;
            double low_poly_cabin_emission_fade_time = 0.2;
            NodePath head_display_e3d_path;
            Ref<Material> head_display_material;
            NodePath head_display_node_path;

            bool dirty = true;
            bool needs_head_display_update = false;
            Node *head_display_e3d = nullptr;
            Node3D *cabin = nullptr;
            Node3D *camera = nullptr;
            Node *cabin_player = nullptr;
            int cabin_show_frames = 0;
            TrainController *controller = nullptr;
            Node *electric_engine = nullptr;
            Node *fiz_controller = nullptr;
            Node3D *model_node = nullptr;
            Area3D *detection_area = nullptr;
            VisibleOnScreenNotifier3D *visibility_notifier = nullptr;
            bool is_visible = true;
            bool force_detail_refresh = true;
            Transform3D last_center_transform;
            Node3D *low_poly_cabin = nullptr;
            TypedArray<ShaderMaterial> low_poly_emissive_materials;
            Ref<Tween> low_poly_emission_tween;
            RID rid;
            bool pending_start_track_retry = false;
            double update_time = 0.0;
            bool animation_bindings_dirty = true;
            Node3D *front_bogie_node = nullptr;
            Node3D *rear_bogie_node = nullptr;
            TypedArray<Node3D> front_rolling_wheel_nodes;
            TypedArray<Node3D> powered_wheel_nodes;
            TypedArray<Node3D> rear_rolling_wheel_nodes;
            Dictionary node_rest_bases;
            Dictionary bogie_rest_global_bases;
            bool bogie_configuration_warned = false;
            TypedArray<Node3D> pantograph_front_arm_nodes;
            TypedArray<Node3D> pantograph_rear_arm_nodes;
            Dictionary pantograph_front_geometry;
            Dictionary pantograph_rear_geometry;
            bool pantograph_front_converged = true;
            bool pantograph_rear_converged = true;
            TypedArray<Dictionary> pantograph_wire_cache;

            Object *_singleton(const StringName &p_name) const;
            TrainController *_resolve_controller(const NodePath &p_node_path) const;
            void _jump_into_cabin(Node3D *p_cabin, Node *p_player);
            void _show_cabin_after_frames();
            void _apply_cabin_camera_configuration();
            void _on_controller_changed(TrainController *p_controller);
            void _update_head_display();
            void _schedule_head_display_update();
            void _process_impl(double p_delta);
            void _process_dirty();
            void _sync_model_lights();
            void _sync_lights_from_controller();
            void _on_low_poly_cabin_e3d_loaded();
            void _update_low_poly_cabs_visibility();
            void _on_roof_light_changed(bool p_enabled);
            void _set_low_poly_emission_energy(double p_value);
            void _update_detection_area();
            void _on_screen_entered();
            void _on_screen_exited();
            void _on_track_manager_tracks_changed();
            void _apply_start_track();
            TypedArray<Node3D> _resolve_animation_nodes(const TypedArray<NodePath> &p_paths) const;
            void _capture_rest_basis(Node3D *p_node);
            TypedArray<Node3D> _resolve_pantograph_arm_nodes(const TypedArray<NodePath> &p_paths) const;
            Dictionary _cache_pantograph_geometry(const TypedArray<Node3D> &p_nodes) const;
            void _cache_animation_bindings();
            Dictionary coupler_submodel_nodes;
            int64_t coupler_visibility_state = -1;
            int _air_coupler_status(const String &p_name) const;
            int _pneumatic_variant(int p_end, bool p_brake_hose) const;
            void _show_air_coupler(const String &p_name, bool p_on, bool p_xon);
            void _update_couplers();
            void _apply_wheel_rotation(const TypedArray<Node3D> &p_nodes, double p_angle_degrees);
            void _update_wheel_animation_state();
            void _update_track_transform();
            Dictionary _pantograph_frame_axes() const;
            void _update_pantograph_power();
            double _pantograph_wire_voltage(
                    int p_index, const Vector3 &p_offset, const Vector3 &p_forward, const Vector3 &p_up,
                    const Vector3 &p_left, double p_assumed_voltage, double p_current);
            void _update_pantograph_raise_state(double p_delta);
            bool _update_pantograph_arm(
                    int p_index, Dictionary p_geometry, const TypedArray<Node3D> &p_arm_nodes, bool p_is_active,
                    double p_delta);
            Dictionary _find_pantograph_wire(
                    int p_index, const Vector3 &p_contact_point, const Vector3 &p_up, const Vector3 &p_forward,
                    const Vector3 &p_left);
            void _apply_pantograph_animation(const TypedArray<Node3D> &p_nodes, const Dictionary &p_geometry);

        protected:
            static void _bind_methods();

        public:
            RailVehicle3D();

            void _enter_tree() override;
            void _ready() override;
            void _exit_tree() override;
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)

            void enter_cabin(Node *p_player);
            void leave_cabin(Node *p_player);
            void process_manually(const Variant &p_delta);
            TrainController *get_controller() const;
            void move_on_track(double p_distance);
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
            void set_pantograph_front_offset(const Vector3 &p_value);
            Vector3 get_pantograph_front_offset() const;
            void set_pantograph_rear_offset(const Vector3 &p_value);
            Vector3 get_pantograph_rear_offset() const;
            void set_pantograph_collector_width(double p_value);
            double get_pantograph_collector_width() const;
            void set_pantograph_front_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_pantograph_front_arm_paths() const;
            void set_pantograph_rear_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_pantograph_rear_arm_paths() const;
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
