#include "E3DSubModel.hpp"

namespace godot {
    E3DSubModel::~E3DSubModel() {
        clear();
    }

    void E3DSubModel::clear() {
        for (int i = 0; i < submodels.size(); i++) {
            Ref<E3DSubModel> sm = submodels.get(i);
            if (sm.is_valid()) {
                sm->clear();
            }
        }
        submodels.clear();
        mesh.unref();
        parent = nullptr;
    }

    void E3DSubModel::_bind_methods() {
        // Enums
        BIND_ENUM_CONSTANT(SUBMODEL_GL_POINTS)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_LINES)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_LINE_STRIP)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_LINE_LOOP)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_TRIANGLES)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_TRIANGLE_STRIP)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_TRIANGLE_FAN)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_QUADS)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_QUAD_STRIP)
        BIND_ENUM_CONSTANT(SUBMODEL_GL_POLYGON)
        BIND_ENUM_CONSTANT(SUBMODEL_TRANSFORM)
        BIND_ENUM_CONSTANT(SUBMODEL_FREE_SPOTLIGHT)
        BIND_ENUM_CONSTANT(SUBMODEL_STARS)

        BIND_ENUM_CONSTANT(ANIMATION_NONE)
        BIND_ENUM_CONSTANT(ANIMATION_ROTATE_VEC)
        BIND_ENUM_CONSTANT(ANIMATION_ROTATE_XYZ)
        BIND_ENUM_CONSTANT(ANIMATION_MOVE)
        BIND_ENUM_CONSTANT(ANIMATION_JUMP_SECONDS)
        BIND_ENUM_CONSTANT(ANIMATION_JUMP_MINUTES)
        BIND_ENUM_CONSTANT(ANIMATION_JUMP_HOURS)
        BIND_ENUM_CONSTANT(ANIMATION_JUMP_HOURS24)
        BIND_ENUM_CONSTANT(ANIMATION_SECONDS)
        BIND_ENUM_CONSTANT(ANIMATION_MINUTES)
        BIND_ENUM_CONSTANT(ANIMATION_HOURS)
        BIND_ENUM_CONSTANT(ANIMATION_HOURS24)
        BIND_ENUM_CONSTANT(ANIMATION_BILLBOARD)
        BIND_ENUM_CONSTANT(ANIMATION_WIND)
        BIND_ENUM_CONSTANT(ANIMATION_SKY)
        BIND_ENUM_CONSTANT(ANIMATION_DIGITAL)
        BIND_ENUM_CONSTANT(ANIMATION_DIGICLK)
        BIND_ENUM_CONSTANT(ANIMATION_UNDEFINED)
        BIND_ENUM_CONSTANT(ANIMATION_IK)
        BIND_ENUM_CONSTANT(ANIMATION_IK1)
        BIND_ENUM_CONSTANT(ANIMATION_IK2)
        BIND_ENUM_CONSTANT(ANIMATION_UNKNOWN)

        // Properties

        BIND_PROPERTY_W_HINT(
                E3DSubModel, Variant::INT, submodel_type, PROPERTY_HINT_ENUM,
                "GL_POINTS,GL_LINES,GL_LINE_STRIP,GL_LINE_LOOP,GL_TRIANGLES,GL_TRIANGLE_STRIP,GL_TRIANGLE_FAN,GL_QUADS,"
                "GL_QUAD_STRIP,GL_POLYGON,TRANSFORM:256,FREE_SPOTLIGHT,STARS");

        BIND_PROPERTY_W_HINT(
                E3DSubModel, Variant::INT, animation, PROPERTY_HINT_ENUM,
                "NONE,ROTATE_VEC,ROTATE_XYZ,MOVE,JUMP_SECONDS,JUMP_MINUTES,JUMP_HOURS,JUMP_HOURS24,SECONDS,MINUTES,"
                "HOURS,HOURS24,BILLBOARD,WIND,SKY,DIGITAL,DIGICLK,UNDEFINED,IK:256,IK1,IK2,UNKNOWN");

        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, lights_on_threshold);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, visibility_light);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, visibility_range_begin);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, visibility_range_end);

        BIND_PROPERTY(E3DSubModel, Variant::COLOR, diffuse_color);
        BIND_PROPERTY(E3DSubModel, Variant::COLOR, self_illumination);

        BIND_PROPERTY(E3DSubModel, Variant::BOOL, material_colored);
        BIND_PROPERTY(E3DSubModel, Variant::BOOL, dynamic_material);
        BIND_PROPERTY(E3DSubModel, Variant::INT, dynamic_material_index);
        BIND_PROPERTY(E3DSubModel, Variant::BOOL, material_transparent);
        BIND_PROPERTY(E3DSubModel, Variant::STRING, material_name);

        BIND_PROPERTY(E3DSubModel, Variant::TRANSFORM3D, transform);
        BIND_PROPERTY(E3DSubModel, Variant::TRANSFORM3D, material_transform);

        BIND_PROPERTY_W_HINT(E3DSubModel, Variant::OBJECT, mesh, PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh");

        BIND_PROPERTY_W_HINT_RES_ARRAY(E3DSubModel, Variant::ARRAY, submodels, PROPERTY_HINT_ARRAY_TYPE, "E3DSubModel");

        BIND_PROPERTY(E3DSubModel, Variant::BOOL, visible);
        BIND_PROPERTY(E3DSubModel, Variant::BOOL, skip_rendering);

        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, light_range);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, light_attenuation);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, light_angle);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, near_attenuation_start);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, near_attenuation_end);
        BIND_PROPERTY(E3DSubModel, Variant::BOOL, use_near_attenuation);
        BIND_PROPERTY(E3DSubModel, Variant::INT, far_attenuation_decay);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, cos_hotspot_angle);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, cos_view_angle);
        BIND_PROPERTY(E3DSubModel, Variant::FLOAT, light_energy);
    }

    void E3DSubModel::add_child(const Ref<E3DSubModel> &p_sub_model) {
        submodels.append(p_sub_model);
    }

    void E3DSubModel::set_parent(E3DSubModel *p_sub_model) {
        parent = p_sub_model;
        if (p_sub_model != nullptr) {
            p_sub_model->add_child(this);
        }
    }
} // namespace godot
