#pragma once
#include "vehicles/rail/RailVehicleAppearance.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

#include <array>

namespace godot {
    /* What a rail vehicle looks like, drawn: its models, the submodels that move and how they
     * move, its lights, smoke, couplers, head display, low-poly interior, the detail it is drawn in
     * and where the player finds it. Keyed by the vehicle's RailVehicleServer handle.
     *
     * The vehicle is drawn at a node (vehicle_attach()) - the one its cab, sounds and anything
     * else of the vehicle ride on; this server moves that node wherever RailVehicleServer places
     * the vehicle, and draws the models there. What moves is animated from the vehicle's state
     * natively, once a frame for the vehicles near the camera and a few times a second for the
     * rest - no script runs per vehicle. */
    class RailVehicleRenderingServer : public Object {
            GDCLASS(RailVehicleRenderingServer, Object)

        public:
            /* The vehicle's exterior model was built - as nodes near the camera, as
             * RenderingServer instances far from it; what dresses its nodes does it again */
            static const char *vehicle_model_built_signal;

            /* maszyna/vehicles/detail_distance [m]: nearer than this a vehicle is drawn as nodes and
             * animated, further it is drawn as RenderingServer instances and holds still */
            static constexpr const char *DETAIL_DISTANCE_SETTING = "maszyna/vehicles/detail_distance";
            static constexpr float DEFAULT_DETAIL_DISTANCE = 350.0;
            /* A vehicle drawn in detail keeps it until this much nearer than the detail distance,
             * so one on the boundary is not rebuilt over and over - proportional, as a fixed margin
             * is nothing at a long detail distance and more than the distance at a short one */
            static constexpr float DETAIL_HYSTERESIS = 0.25;
            static constexpr float DETAIL_HYSTERESIS_MIN = 25.0;
            /* How often every vehicle's detail, lights and smoke are looked at [s] - a plume and a
             * lamp change slowly - and how many vehicles a frame may look at doing it */
            static constexpr double SLOW_UPDATE_PERIOD = 0.25;
            static constexpr int MAX_SLOW_UPDATES_PER_FRAME = 16;
            /* How many vehicles drawn in detail a frame animates (pantographs, wipers, mirrors) */
            static constexpr int MAX_DETAILED_UPDATES_PER_FRAME = 32;

        private:
            static RailVehicleRenderingServer *singleton;

            /* The low-poly interior's cabs, cab0 for a vehicle's single one (DynObj.cpp:2383-2391) */
            static constexpr std::array<const char *, 3> LOW_POLY_CABS = {"cab0", "cab1", "cab2"};

            /* A submodel of the exterior model that moves, and where it rests in the vehicle's own
             * frame - what its pose turns it from */
            struct Part {
                    String submodel;
                    Transform3D rest;
            };

            struct Visual {
                    ObjectID node;
                    Ref<RailVehicleAppearance> appearance;
                    /* The exterior and the low-poly interior - this server's own when it built them
                     * from the appearance, else whoever handed them over owns them */
                    RID model;
                    RID low_poly;
                    RID passengers;
                    Vector<RID> attachments;
                    RID load;
                    /* The cargo's model file, read again with the game's data */
                    String load_data_path;
                    String load_model_filename;
                    bool own_models = false;
                    Transform3D model_transform;
                    /* How far the cargo sinks into an empty vehicle [m] (DynObj.cpp:3070-3080) */
                    double load_height = 0.0;
                    Part bogies[2];
                    /* front rolling, powered, rear rolling */
                    Vector<Part> wheels[3];
                    /* RailVehicleElectricEngine::PantographSelector: lower arm, its pair, upper arm,
                     * its pair, slider */
                    Vector<Part> pantograph_arms[2];
                    Vector<Part> wiper_arms;
                    Vector<Part> mirrors;
                    /* The coupler and hose submodels the model has (AirCoupler::Init(),
                     * DynObj.cpp:2170-2181) */
                    HashSet<String> coupler_submodels;
                    int64_t coupler_state = -1;
                    /* Every pose of the exterior, sent to it in one call */
                    Dictionary poses;
                    TypedDictionary<String, bool> lights;
                    bool headlights_dimmed = false;
                    Ref<Material> head_display_material;
                    /* Born optimized: the models are made before the vehicle stands on its track, at
                     * the origin, and a scenery's hundreds of vehicles all built as node hierarchies
                     * (cars, interiors) filled the load; _update_detail() details the near ones */
                    bool detailed = false;
                    RID pickable;
                    RID detection_area;
                    RID detection_shape;
                    /* The cab the player sees from, 0 for none, and whether it is a modelled one */
                    int cab = 0;
                    bool has_cab_model = false;
                    /* The level of each low-poly cab's light (vehicle_set_cab_light_level()), and the
                     * self-illumination the cab is drawn with, following it */
                    double cab_light_levels[LOW_POLY_CABS.size()] = {};
                    double cab_light_energies[LOW_POLY_CABS.size()] = {};
                    PackedFloat64Array wiper_positions;
                    double mirror_left = -1.0;
                    double mirror_right = -1.0;
                    int mirror_cab = 0;
            };

            enum PneumaticLine {
                PNEUMATIC_LINE_BRAKE,
                PNEUMATIC_LINE_MAIN,
            };

            enum PneumaticLayout {
                PNEUMATIC_LAYOUT_NONE,
                PNEUMATIC_LAYOUT_LEFT,
                PNEUMATIC_LAYOUT_RIGHT,
                PNEUMATIC_LAYOUT_BOTH,
            };

            /* A mechanical coupler uses OFF, ON and XON. An air hose additionally selects the right
             * submodel, slanted or straight (TDynamicObject::SetPneumatic(), DynObj.cpp:497). */
            enum CouplerVariant {
                COUPLER_VARIANT_OFF,
                COUPLER_VARIANT_ON,
                COUPLER_VARIANT_XON,
                COUPLER_VARIANT_RIGHT_XON,
                COUPLER_VARIANT_RIGHT_ON,
                COUPLER_VARIANT_COUNT,
            };

            HashMap<RID, Visual> vehicles;
            /* The vehicle each exterior model draws */
            HashMap<RID, RID> model_vehicles;
            /* The vehicles in the order the frame visits them, and where each visit left off */
            Vector<RID> visit_order;
            int slow_cursor = 0;
            int detailed_cursor = 0;
            double slow_elapsed = 0.0;
            /* The vehicles whose low-poly cabs are still following their cab lights */
            Vector<RID> fading;
            bool processing = false;

            static Node3D *_node(const Visual &p_visual);
            void _set_processing(bool p_processing);
            void _process_frame();
            void _on_data_reload_requested();
            void _free_models(Visual &p_visual);
            void _create_models(const RID &p_vehicle, Visual &p_visual);
            void _bind_parts(const RID &p_vehicle, Visual &p_visual);
            Part _part(const Visual &p_visual, const String &p_submodel) const;
            Vector<Part> _parts(const Visual &p_visual, const PackedStringArray &p_submodels) const;
            void _publish_pantograph_geometry(
                    const RID &p_vehicle, const Visual &p_visual,
                    RailVehicleElectricEngine::PantographSelector p_pantograph) const;
            void _place(const RID &p_vehicle, Visual &p_visual);
            void _pose(Visual &p_visual, const Part &p_part, const Basis &p_pose);
            void _pose_running_gear(const RID &p_vehicle, Visual &p_visual);
            void _pose_pantographs(const RID &p_vehicle, Visual &p_visual);
            void _pose_wipers(const RID &p_vehicle, Visual &p_visual);
            void _pose_mirrors(const RID &p_vehicle, Visual &p_visual);
            void _send_poses(Visual &p_visual);
            void _update_couplers(const RID &p_vehicle, Visual &p_visual);
            PneumaticLayout _pneumatic_layout(
                    const Visual &p_visual, RailVehicleController::CouplerEnd p_end, PneumaticLine p_line) const;
            CouplerVariant _pneumatic_variant(
                    const RID &p_vehicle, const Visual &p_visual, RailVehicleController::CouplerEnd p_end,
                    PneumaticLine p_line) const;
            void _show_air_coupler(const Visual &p_visual, const String &p_name, bool p_on, bool p_xon);
            void _update_lights(const RID &p_vehicle, Visual &p_visual);
            void _update_smoke(const RID &p_vehicle, const Visual &p_visual) const;
            void _update_detail(const RID &p_vehicle, Visual &p_visual);
            void _update_low_poly_cabs(const Visual &p_visual) const;
            void _update_load(const RID &p_vehicle, Visual &p_visual);
            void _update_detection_area(Visual &p_visual);
            void _register_pickable(const RID &p_vehicle, Visual &p_visual);
            void _on_vehicle_placed(const RID &p_vehicle);
            void _on_vehicle_trainset_changed(const RID &p_vehicle);
            void _on_vehicle_coupler_changed(const RID &p_vehicle, int64_t p_flag);
            void _on_vehicle_config_changed(const RID &p_vehicle);
            void _on_vehicle_freed(const RID &p_vehicle);
            void _on_instance_built(const RID &p_instance);

        protected:
            static void _bind_methods();

        public:
            static RailVehicleRenderingServer *get_instance();

            RailVehicleRenderingServer();
            ~RailVehicleRenderingServer() override;

            /* The vehicle is drawn at the node p_node_id - moved there wherever the vehicle is
             * placed. Freed with the vehicle (VehicleServer.vehicle_freed). */
            void vehicle_attach(const RID &p_vehicle, uint64_t p_node_id);
            void vehicle_detach(const RID &p_vehicle);
            bool vehicle_is_attached(const RID &p_vehicle) const;
            /* The node the vehicle is drawn at, 0 for none */
            uint64_t vehicle_get_node(const RID &p_vehicle) const;
            /* The world the node is in, an empty RID out of it: the models this server built are
             * drawn there - the node's NOTIFICATION_ENTER_WORLD/EXIT_WORLD, as VisualInstance3D's */
            void vehicle_set_scenario(const RID &p_vehicle, const RID &p_scenario);
            /* What the vehicle looks like. With model files, this server builds the models itself;
             * without, it draws the ones vehicle_set_models() hands it. */
            void vehicle_set_appearance(const RID &p_vehicle, const Ref<RailVehicleAppearance> &p_appearance);
            Ref<RailVehicleAppearance> vehicle_get_appearance(const RID &p_vehicle) const;
            /* The exterior and the low-poly interior, as E3DRenderingServer instances somebody else
             * owns - a vehicle assembled by hand */
            void vehicle_set_models(const RID &p_vehicle, const RID &p_model, const RID &p_low_poly);
            /* The exterior model as E3DRenderingServer draws it */
            RID vehicle_get_model(const RID &p_vehicle) const;
            /* The cargo, drawn at the floor of the vehicle (DynObj.cpp:866) - none for an empty
             * file name */
            void
            vehicle_set_load_model(const RID &p_vehicle, const String &p_data_path, const String &p_model_filename);
            void vehicle_set_head_display_material(const RID &p_vehicle, const Ref<Material> &p_material);
            /* The cab the player looks from (0 for none) and whether it is modelled: the low-poly
             * interior hides that cab, or all of them with jointcabs: (DynObj.cpp:1335-1340) */
            void vehicle_set_cab(const RID &p_vehicle, int p_cab, bool p_has_cab_model);
            /* The level (0..1) of the light of a cab - 1, 0 or -1, as the cab layer counts them -
             * that its low-poly cab is lit at (TDynamicObject::set_cab_lights(), DynObj.cpp:841-853);
             * with jointcabs: every cab at the brightest */
            void vehicle_set_cab_light_level(const RID &p_vehicle, int p_cab, double p_level);
            /* Whether the vehicle is drawn in detail - as nodes, animated */
            bool vehicle_is_detailed(const RID &p_vehicle) const;
    };
} // namespace godot
