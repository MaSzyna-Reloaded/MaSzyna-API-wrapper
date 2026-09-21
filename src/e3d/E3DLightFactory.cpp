#include "E3DLightFactory.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    namespace {
        constexpr const char *STREET_LAMP_PREFIX = "latarnia";
        /// The halo billboards around the lamp head, and the lit patch on the ground
        constexpr const char *HALO_MATERIAL = "poswiata";
        constexpr const char *POOL_MATERIAL_1 = "light1";
        constexpr const char *POOL_MATERIAL_2 = "light2";

        struct StreetLampAnchors {
                Transform3D head;
                float pool_extent = 0.0;
                Color color = Color(1.0, 1.0, 1.0);
                bool has_head = false;
                bool has_pool = false;
        };

        bool has_material(const E3DSubModel *p_submodel, const char *p_name) {
            return p_submodel->get_material_name().to_lower().ends_with(p_name);
        }

        /// Depth first walk accumulating the transform, collecting both anchors of a street lamp
        void collect_street_lamp_anchors(
                const TypedArray<E3DSubModel> &p_submodels, const Transform3D &p_parent_transform,
                StreetLampAnchors &p_anchors) {
            for (int i = 0; i < p_submodels.size(); i++) {
                const Ref<E3DSubModel> submodel = p_submodels[i];
                if (submodel.is_null()) {
                    continue;
                }
                const Transform3D transform = p_parent_transform * submodel->get_transform();
                if (!p_anchors.has_head && has_material(submodel.ptr(), HALO_MATERIAL)) {
                    p_anchors.head = transform;
                    p_anchors.color = submodel->get_diffuse_color();
                    p_anchors.has_head = true;
                }
                if (!p_anchors.has_pool &&
                    (has_material(submodel.ptr(), POOL_MATERIAL_1) || has_material(submodel.ptr(), POOL_MATERIAL_2)) &&
                    submodel->get_mesh().is_valid()) {
                    const Vector3 size = submodel->get_mesh()->get_aabb().size;
                    p_anchors.pool_extent = MAX(size.x, size.z) * 0.5f;
                    p_anchors.has_pool = p_anchors.pool_extent > 0.0f;
                }
                collect_street_lamp_anchors(submodel->get_submodels(), transform, p_anchors);
            }
        }
    } // namespace

    const E3DModelLight *E3DModelLights::find(const String &p_name) const {
        for (const E3DModelLight &light: lights) {
            if (light.name == p_name) {
                return &light;
            }
        }
        return nullptr;
    }

    namespace {
        /// Collects the real lights of the model, carrying down the light of the nearest "on"
        /// ancestor and the transform relative to the model root
        void collect_placements(
                const TypedArray<E3DSubModel> &p_submodels, const Transform3D &p_parent_transform,
                const HashMap<E3DSubModel *, String> &p_light_owners, const String &p_parent_light_name,
                Vector<E3DModelLightPlacement> &p_placements, HashMap<E3DSubModel *, Transform3D> &p_on_transforms) {
            for (int i = 0; i < p_submodels.size(); i++) {
                const Ref<E3DSubModel> submodel = p_submodels[i];
                if (submodel.is_null()) {
                    continue;
                }
                const Transform3D transform = p_parent_transform * submodel->get_transform();
                String light_name = p_parent_light_name;
                if (const HashMap<E3DSubModel *, String>::ConstIterator owner = p_light_owners.find(submodel.ptr());
                    owner != p_light_owners.end()) {
                    light_name = owner->value;
                    p_on_transforms[submodel.ptr()] = transform;
                }
                if (submodel->get_submodel_type() == E3DSubModel::SUBMODEL_FREE_SPOTLIGHT) {
                    E3DModelLightPlacement placement;
                    placement.light_name = light_name;
                    placement.params = E3DLightFactory::from_submodel(submodel.ptr(), light_name);
                    placement.params.transform = transform;
                    p_placements.push_back(placement);
                }
                collect_placements(
                        submodel->get_submodels(), transform, p_light_owners, light_name, p_placements,
                        p_on_transforms);
            }
        }
    } // namespace

    E3DModelLights E3DLightFactory::discover(const Ref<E3DModel> &p_model, const String &p_model_filename) {
        E3DModelLights model_lights;
        if (p_model.is_null()) {
            return model_lights;
        }

        HashMap<E3DSubModel *, String> light_owners;
        const TypedDictionary<String, E3DModelLightDefinition> lights = p_model->get_lights();
        const Array light_names = lights.keys();
        for (int i = 0; i < light_names.size(); i++) {
            const String light_name = light_names[i];
            const Ref<E3DModelLightDefinition> light_info = lights[light_name];
            if (light_info.is_null()) {
                continue;
            }
            E3DModelLight light;
            light.name = light_name;
            light.on = p_model->get_node_or_null(light_info->get_on_submodel_path()).ptr();
            light.off = p_model->get_node_or_null(light_info->get_off_submodel_path()).ptr();
            model_lights.lights.push_back(light);
            // a real light belongs to the light of its nearest "on" ancestor
            if (light.on != nullptr) {
                light_owners[light.on] = light_name;
            }
        }

        HashMap<E3DSubModel *, Transform3D> on_transforms;
        collect_placements(
                p_model->get_submodels(), Transform3D(), light_owners, String(), model_lights.placements,
                on_transforms);
        if (!model_lights.placements.is_empty() || !is_street_lamp(p_model_filename)) {
            return model_lights;
        }

        for (const E3DModelLight &light: model_lights.lights) {
            if (light.on == nullptr) {
                continue;
            }
            E3DModelLightPlacement placement;
            placement.light_name = light.name;
            placement.synthesized = true;
            const HashMap<E3DSubModel *, Transform3D>::ConstIterator on_transform = on_transforms.find(light.on);
            if (make_street_lamp(
                        light.on, on_transform == on_transforms.end() ? Transform3D() : on_transform->value,
                        placement.params)) {
                model_lights.placements.push_back(placement);
            }
        }
        return model_lights;
    }

    E3DLightParams E3DLightFactory::from_submodel(E3DSubModel *p_submodel, const String &p_light_name) {
        E3DLightParams params;
        const bool is_end_light = p_light_name.begins_with("endsignal") || p_light_name.begins_with("endtab");

        params.energy = DEFAULT_LIGHT_ENERGY;
        if (p_light_name.begins_with("headlamp")) {
            params.energy = DEFAULT_HEAD_LIGHT_ENERGY;
        } else if (p_light_name.begins_with("highbeam")) {
            params.energy = DEFAULT_HIGHBEAM_LIGHT_ENERGY;
        } else if (is_end_light) {
            params.energy = DEFAULT_END_LIGHT_ENERGY;
        } else if (p_submodel->get_light_energy() > 0.0) {
            params.energy = p_submodel->get_light_energy();
        }

        params.range = DEFAULT_LIGHT_SPOT_RANGE;
        if (is_end_light) {
            params.range = FORCED_END_LIGHT_SPOT_RANGE;
        } else if (p_submodel->get_light_range() > 0.0) {
            params.range = p_submodel->get_light_range();
        }

        // iFarAttenDecay: 0 no decay, 1 and 2 are powers of 1/R (Model3d.h:120)
        params.attenuation = 1.0;
        switch (p_submodel->get_far_attenuation_decay()) {
            case 0:
                params.attenuation = 0.0;
                break;
            case 2:
                params.attenuation = 2.0;
                break;
            default:
                break;
        }

        const float inner_angle = Math::rad_to_deg(Math::acos(CLAMP(p_submodel->get_cos_hotspot_angle(), -1.0f, 1.0f)));
        const float outer_angle = MAX(p_submodel->get_light_angle(), 0.001f);
        const float penumbra_ratio = CLAMP((outer_angle - inner_angle) / outer_angle, 0.0f, 1.0f);

        params.omni = outer_angle > MAX_SPOT_ANGLE;
        params.spot_angle = MIN(outer_angle, MAX_SPOT_ANGLE);
        params.spot_attenuation = Math::lerp(2.0f, 0.0f, penumbra_ratio);
        params.transform = p_submodel->get_transform();
        params.color = p_submodel->get_diffuse_color();
        params.color.a = 1.0;
        return params;
    }

    bool E3DLightFactory::is_street_lamp(const String &p_model_filename) {
        return p_model_filename.get_file().to_lower().begins_with(STREET_LAMP_PREFIX);
    }

    bool E3DLightFactory::make_street_lamp(
            E3DSubModel *p_on_submodel, const Transform3D &p_on_transform, E3DLightParams &p_params) {
        if (p_on_submodel == nullptr) {
            return false;
        }

        StreetLampAnchors anchors;
        collect_street_lamp_anchors(p_on_submodel->get_submodels(), p_on_transform, anchors);
        if (!anchors.has_head || !anchors.has_pool) {
            return false;
        }

        const float height = anchors.head.origin.y;
        if (height <= 0.0f) {
            return false; // a head at or below the ground gives no cone to compute
        }

        p_params.omni = false;
        // pointing straight down: Godot's spot shines along -Z
        p_params.transform = Transform3D(Basis(Vector3(1, 0, 0), Math_PI * -0.5), anchors.head.origin);
        p_params.color = anchors.color;
        p_params.color.a = 1.0;
        p_params.energy = DEFAULT_LIGHT_ENERGY;
        p_params.range = Math::sqrt((height * height) + (anchors.pool_extent * anchors.pool_extent));
        p_params.spot_angle =
                MIN(Math::rad_to_deg(Math::atan2(anchors.pool_extent, height)), MAX_SPOT_ANGLE);
        p_params.spot_attenuation = 1.0;
        p_params.attenuation = 1.0;
        return true;
    }
} // namespace godot
