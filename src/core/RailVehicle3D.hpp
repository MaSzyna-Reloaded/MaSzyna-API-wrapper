#pragma once

#include "VehicleController.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

namespace godot {
    class Cabin3D;
    class VehicleBuffCoupl;
    class VehiclePhysicsNode;
    class Area3D;
    class VehicleElectricEngine;
    class VisibleOnScreenNotifier3D;

    class RailVehicle3D : public Node3D {
            GDCLASS(RailVehicle3D, Node3D)

        public:
            static const char *controller_changed_signal;

        private:
            NodePath model_instance_path;
            TypedDictionary<String, bool> lights;
            NodePath controller_path;
            NodePath front_bogie_path;
            NodePath rear_bogie_path;
            TypedArray<NodePath> front_rolling_wheel_paths;
            TypedArray<NodePath> powered_wheel_paths;
            TypedArray<NodePath> rear_rolling_wheel_paths;
            /* FIXME(#184): where a collector sits is the vehicle's geometry, not the drawing
             * node's - the original keeps it in TAnimPant::vPos. It is exported here only because
             * the instancer reads it off the model, and the pantograph power path cannot move to
             * RailVehicleServer until it does not have to come back here for these. */
            Vector3 pantograph_front_offset;
            Vector3 pantograph_rear_offset;
            double pantograph_collector_width = 0.5;
            /* Half of the slider's width, taken from the vehicle's own CSW when its configuration
             * lands - see _on_vehicle_config_changed(). */
            double pantograph_slider_half_width = 0.5;
            /* How far outside the slider the guide horn still catches a wire (DynObj.cpp:93,
             * fWidthExtra). Without it a pantograph drops the wire wherever it swings sideways -
             * at a span junction, over a switch, or on the zigzag - and the vehicle reads a real
             * loss of voltage where the original keeps contact. */
            static constexpr double PANTOGRAPH_HORN_WIDTH = 0.381;
            TypedArray<NodePath> pantograph_front_arm_paths;
            TypedArray<NodePath> pantograph_rear_arm_paths;
            // three per wiper (arm 1, arm 2, blade), an empty path for a missing one
            TypedArray<NodePath> wiper_arm_paths;
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
            Node3D *load_model = nullptr;
            double low_poly_cabin_emission_energy = 0.2;
            double low_poly_cabin_emission_fade_time = 0.2;
            NodePath head_display_e3d_path;
            Ref<Material> head_display_material;
            NodePath head_display_node_path;

            bool dirty = true;
            bool needs_head_display_update = false;
            Node *head_display_e3d = nullptr;
            Cabin3D *cabin = nullptr;
            Node3D *camera = nullptr;
            Node *cabin_player = nullptr;
            int cabin_show_frames = 0;
            VehicleController *controller = nullptr;
            VehicleElectricEngine *electric_engine = nullptr;
            VehiclePhysicsNode *fiz_controller = nullptr;
            Node3D *model_node = nullptr;
            Area3D *detection_area = nullptr;
            VisibleOnScreenNotifier3D *visibility_notifier = nullptr;
            bool is_visible = true;
            /// Fallback for maszyna/vehicles/detail_distance
            static constexpr float DEFAULT_VEHICLE_DETAIL_DISTANCE_M = 1000.0;
            /// maszyna/vehicles/detail_distance is the distance the node hierarchy is
            /// dropped at; it is taken back this much closer, so a vehicle sitting on the boundary
            /// is not rebuilt over and over. Proportional, because a fixed margin is either nothing
            /// at a long detail distance or larger than the distance itself at a short one.
            static constexpr float VEHICLE_DETAIL_HYSTERESIS = 0.25;
            static constexpr float VEHICLE_DETAIL_HYSTERESIS_MIN_M = 25.0;
            /// The model currently uses the node hierarchy (bogies, wheels, pantograph arms)
            bool model_detailed = true;
            bool force_detail_refresh = true;
            Transform3D last_body_transform;
            Node3D *low_poly_cabin = nullptr;
            TypedArray<ShaderMaterial> low_poly_emissive_materials;
            Ref<Tween> low_poly_emission_tween;
            RID rid;
            /* Whether `rid` is this node's own handle or the vehicle's, adopted from the controller. */
            bool rid_owned = false;
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
            TypedArray<Node3D> wiper_arm_nodes;
            PackedFloat64Array wiper_applied_positions;
            Dictionary pantograph_front_geometry;
            Dictionary pantograph_rear_geometry;
            bool pantograph_front_converged = true;
            bool pantograph_rear_converged = true;
            TypedArray<Dictionary> pantograph_wire_cache;

            VehicleController *_resolve_controller(const NodePath &p_node_path) const;
            void _jump_into_cabin(Node3D *p_cabin, Node *p_player);
            void _show_cabin_after_frames();
            void _apply_cabin_camera_configuration();
            void _on_controller_changed(VehicleController *p_controller);
            void _on_vehicle_changed();
            void _bind_vehicle_node();
            const VehicleBuffCoupl *_coupler() const;
            void _on_vehicle_config_changed();
            void _adopt_vehicle_parts();
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
            String _track_position_text() const;
            void _report_contact_gap(int p_index, bool p_is_active, bool p_converged);

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
            void _update_wipers();
            void _apply_wheel_rotation(const TypedArray<Node3D> &p_nodes, double p_angle_degrees);
            void _update_wheel_animation_state();
            void _update_model_detail();
            void _update_smoke();
            /// Vehicle frame the pantograph geometry is expressed in. Built once per frame: it used
            /// to be a Dictionary of three Vector3s, allocated and boxed again for every call, with
            /// get_global_transform() asked three times over.
            struct PantographFrame {
                    Transform3D transform;
                    Vector3 forward;
                    Vector3 up;
                    Vector3 left;
            };

            PantographFrame _pantograph_frame() const;
            /// The controller state is fetched once per frame and handed down - every one of these
            /// used to ask for it again, and a scenery runs hundreds of powered vehicles
            void _update_pantograph_power(const Dictionary &p_state);
            double _pantograph_wire_voltage(
                    int p_index, const Vector3 &p_offset, const PantographFrame &p_frame, double p_assumed_voltage,
                    double p_current);
            void _update_pantograph_raise_state(double p_delta, const Dictionary &p_state);
            bool _update_pantograph_arm(
                    int p_index, Dictionary p_geometry, const TypedArray<Node3D> &p_arm_nodes, bool p_is_active,
                    double p_delta, const Dictionary &p_state);
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
            VehicleController *get_controller() const;
            /// This vehicle's handle in RailVehicleServer - the key anything
            /// keeping per-vehicle state of its own is meant to use.
            RID get_rid() const;
            /* Applies the placement RailVehicleServer's step just produced - the bogie
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
            void set_wiper_arm_paths(const TypedArray<NodePath> &p_value);
            TypedArray<NodePath> get_wiper_arm_paths() const;
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
