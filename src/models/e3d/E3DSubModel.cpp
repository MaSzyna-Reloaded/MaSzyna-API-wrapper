#include "E3DSubModel.hpp"

namespace godot {

    E3DSubModel::E3DSubModel() {
        parent = nullptr;
    }

    void E3DSubModel::clear() {
        for (int i = 0; i < submodels.size(); i++) {
            Ref<E3DSubModel> sm = submodels.get(i);
            if (sm.is_valid()) {
                sm->clear();
            }
        }
        submodels.clear();
    }

    void E3DSubModel::_bind_methods() {
        // Enums
        BIND_ENUM_CONSTANT(GL_POINTS)
        BIND_ENUM_CONSTANT(GL_LINES)
        BIND_ENUM_CONSTANT(GL_LINE_STRIP)
        BIND_ENUM_CONSTANT(GL_LINE_LOOP)
        BIND_ENUM_CONSTANT(GL_TRIANGLES)
        BIND_ENUM_CONSTANT(GL_TRIANGLE_STRIP)
        BIND_ENUM_CONSTANT(GL_TRIANGLE_FAN)
        BIND_ENUM_CONSTANT(GL_QUADS)
        BIND_ENUM_CONSTANT(GL_QUAD_STRIP)
        BIND_ENUM_CONSTANT(GL_POLYGON)
        BIND_ENUM_CONSTANT(TRANSFORM)
        BIND_ENUM_CONSTANT(FREE_SPOTLIGHT)
        BIND_ENUM_CONSTANT(STARS)

        BIND_ENUM_CONSTANT(NONE)
        BIND_ENUM_CONSTANT(ROTATE_VEC)
        BIND_ENUM_CONSTANT(ROTATE_XYZ)
        BIND_ENUM_CONSTANT(MOVE)
        BIND_ENUM_CONSTANT(JUMP_SECONDS)
        BIND_ENUM_CONSTANT(JUMP_MINUTES)
        BIND_ENUM_CONSTANT(JUMP_HOURS)
        BIND_ENUM_CONSTANT(JUMP_HOURS24)
        BIND_ENUM_CONSTANT(SECONDS)
        BIND_ENUM_CONSTANT(MINUTES)
        BIND_ENUM_CONSTANT(HOURS)
        BIND_ENUM_CONSTANT(HOURS24)
        BIND_ENUM_CONSTANT(BILLBOARD)
        BIND_ENUM_CONSTANT(WIND)
        BIND_ENUM_CONSTANT(SKY)
        BIND_ENUM_CONSTANT(DIGITAL)
        BIND_ENUM_CONSTANT(DIGICLK)
        BIND_ENUM_CONSTANT(UNDEFINED)
        BIND_ENUM_CONSTANT(IK)
        BIND_ENUM_CONSTANT(IK1)
        BIND_ENUM_CONSTANT(IK2)
        BIND_ENUM_CONSTANT(UNKNOWN)

        BIND_PROPERTY_W_HINT(
                Variant::INT, "submodel_type", "submodel_type", &E3DSubModel::set_submodel_type,
                &E3DSubModel::get_submodel_type, "p_submodel_type", PROPERTY_HINT_ENUM,
                "GL_POINTS,GL_LINES,GL_LINE_STRIP,GL_LINE_LOOP,GL_TRIANGLES,GL_TRIANGLE_STRIP,GL_TRIANGLE_FAN,GL_QUADS,"
                "GL_QUAD_STRIP,GL_POLYGON,TRANSFORM,FREE_SPOTLIGHT,STARS");

        BIND_PROPERTY_W_HINT(
                Variant::INT, "animation", "animation", &E3DSubModel::set_animation, &E3DSubModel::get_animation,
                "p_animation", PROPERTY_HINT_ENUM,
                "NONE,ROTATE_VEC,ROTATE_XYZ,MOVE,JUMP_SECONDS,JUMP_MINUTES,JUMP_HOURS,JUMP_HOURS24,SECONDS,MINUTES,"
                "HOURS,HOURS24,BILLBOARD,WIND,SKY,DIGITAL,DIGICLK,UNDEFINED,IK,IK1,IK2,UNKNOWN");

        BIND_PROPERTY(
                Variant::FLOAT, "lights_on_threshold", "lights_on_threshold", &E3DSubModel::set_lights_on_threshold,
                &E3DSubModel::get_lights_on_threshold, "p_lights_on_threshold");
        BIND_PROPERTY(
                Variant::FLOAT, "visibility_light", "visibility_light", &E3DSubModel::set_visibility_light,
                &E3DSubModel::get_visibility_light, "p_visibility_light");
        BIND_PROPERTY(
                Variant::FLOAT, "visibility_range_begin", "visibility_range_begin",
                &E3DSubModel::set_visibility_range_begin, &E3DSubModel::get_visibility_range_begin,
                "p_visibility_range_begin");
        BIND_PROPERTY(
                Variant::FLOAT, "visibility_range_end", "visibility_range_end", &E3DSubModel::set_visibility_range_end,
                &E3DSubModel::get_visibility_range_end, "p_visibility_range_end");

        BIND_PROPERTY(
                Variant::COLOR, "diffuse_color", "diffuse_color", &E3DSubModel::set_diffuse_color,
                &E3DSubModel::get_diffuse_color, "p_diffuse_color");
        BIND_PROPERTY(
                Variant::COLOR, "self_illumination", "self_illumination", &E3DSubModel::set_self_illumination,
                &E3DSubModel::get_self_illumination, "p_self_illumination");

        BIND_PROPERTY(
                Variant::BOOL, "material_colored", "material_colored", &E3DSubModel::set_material_colored,
                &E3DSubModel::get_material_colored, "p_material_colored");
        BIND_PROPERTY(
                Variant::BOOL, "dynamic_material", "dynamic_material", &E3DSubModel::set_dynamic_material,
                &E3DSubModel::get_dynamic_material, "p_dynamic_material");
        BIND_PROPERTY(
                Variant::INT, "dynamic_material_index", "dynamic_material_index",
                &E3DSubModel::set_dynamic_material_index, &E3DSubModel::get_dynamic_material_index,
                "p_dynamic_material_index");
        BIND_PROPERTY(
                Variant::BOOL, "material_transparent", "material_transparent", &E3DSubModel::set_material_transparent,
                &E3DSubModel::get_material_transparent, "p_material_transparent");
        BIND_PROPERTY(
                Variant::STRING, "material_name", "material_name", &E3DSubModel::set_material_name,
                &E3DSubModel::get_material_name, "p_material_name");

        BIND_PROPERTY(
                Variant::TRANSFORM3D, "transform", "transform", &E3DSubModel::set_transform,
                &E3DSubModel::get_transform, "p_transform");
        BIND_PROPERTY(
                Variant::TRANSFORM3D, "material_transform", "material_transform", &E3DSubModel::set_material_transform,
                &E3DSubModel::get_material_transform, "p_material_transform");

        BIND_PROPERTY_W_HINT(
                Variant::OBJECT, "mesh", "mesh", &E3DSubModel::set_mesh, &E3DSubModel::get_mesh, "p_mesh",
                PROPERTY_HINT_RESOURCE_TYPE, "Mesh");

        BIND_PROPERTY_W_HINT_RES_ARRAY(
                Variant::ARRAY, "submodels", "submodels", &E3DSubModel::set_submodels, &E3DSubModel::get_submodels,
                "p_submodels", PROPERTY_HINT_ARRAY_TYPE, "E3DSubModel");

        BIND_PROPERTY(
                Variant::BOOL, "visible", "visible", &E3DSubModel::set_visible, &E3DSubModel::get_visible, "p_visible");
        BIND_PROPERTY(
                Variant::BOOL, "skip_rendering", "skip_rendering", &E3DSubModel::set_skip_rendering,
                &E3DSubModel::get_skip_rendering, "p_skip_rendering");
    }

    void E3DSubModel::add_child(const Ref<E3DSubModel> &p_sub_model) {
        submodels.append(p_sub_model);
    }

    void E3DSubModel::set_parent(E3DSubModel *p_parent) {
        parent = p_parent;
    }

    void E3DSubModel::set_mesh(const Ref<Mesh> &p_mesh) {
        mesh.unref();

        if (p_mesh == nullptr || !p_mesh.is_valid()) {
            return;
        }

        mesh = p_mesh;
    }


    Ref<Mesh> E3DSubModel::get_mesh() const {
        return mesh;
    }

    void E3DSubModel::set_submodels(const TypedArray<E3DSubModel> &p_submodels) {
        submodels = p_submodels;
    }

    TypedArray<E3DSubModel> E3DSubModel::get_submodels() const {
        return submodels;
    }
} // namespace godot
