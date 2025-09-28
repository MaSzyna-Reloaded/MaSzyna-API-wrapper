#pragma once
#include <godot_cpp/classes/array_mesh.hpp>
#include "macros.hpp"
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>


namespace godot {
    class E3DSubModel: public Resource {
        GDCLASS(E3DSubModel, Resource)
        protected:
            static void _bind_methods();
        E3DSubModel* parent = nullptr;
        public:
            enum SubModelType {
                GL_POINTS,
                GL_LINES,
                GL_LINE_STRIP,
                GL_LINE_LOOP,
                GL_TRIANGLES,
                GL_TRIANGLE_STRIP,
                GL_TRIANGLE_FAN,
                GL_QUADS,
                GL_QUAD_STRIP,
                GL_POLYGON,
                TRANSFORM = 256,
                FREE_SPOTLIGHT,
                STARS
            };

            enum AnimationType {
                NONE = 0,
                ROTATE_VEC,
                ROTATE_XYZ,
                MOVE,
                JUMP_SECONDS,
                JUMP_MINUTES,
                JUMP_HOURS,
                JUMP_HOURS24,
                SECONDS,
                MINUTES,
                HOURS,
                HOURS24,
                BILLBOARD,
                WIND,
                SKY,
                DIGITAL,
                DIGICLK,
                UNDEFINED,
                IK=256,
                IK1,
                IK2,
                UNKNOWN=-1
            };

            MAKE_MEMBER_GS(String, name, "")
            MAKE_MEMBER_GS(SubModelType, submodel_type, GL_TRIANGLES)
            MAKE_MEMBER_GS(AnimationType, animation, NONE)
            MAKE_MEMBER_GS(float, lights_on_threshold, 0.0)
            MAKE_MEMBER_GS(float, visibility_light, 0.0)
            MAKE_MEMBER_GS(float, visibility_range_begin, 0.0)
            MAKE_MEMBER_GS(float, visibility_range_end, 0.0)
            MAKE_MEMBER_GS(Color, diffuse_color, Color(255, 255, 255, 255))
            MAKE_MEMBER_GS(Color, self_illumination, Color(255, 255, 255, 255))
            MAKE_MEMBER_GS(bool, material_colored, false)
            MAKE_MEMBER_GS(bool, dynamic_material, false)
            MAKE_MEMBER_GS(int, dynamic_material_index, 0)
            MAKE_MEMBER_GS(bool, material_transparent, false)
            MAKE_MEMBER_GS(String, material_name, "")
            MAKE_MEMBER_GS_NO_DEF(Transform3D, transform)
            MAKE_MEMBER_GS_NO_DEF(Transform3D, material_transform)
            MAKE_MEMBER_GS_NO_DEF(Ref<ArrayMesh>, mesh)
            MAKE_MEMBER_GS_NO_DEF(TypedArray<E3DSubModel>, submodels)
            MAKE_MEMBER_GS(bool, visible, true)
            MAKE_MEMBER_GS(bool, skip_rendering, false);

            void add_child(const E3DSubModel& p_sub_model);
            void set_parent(E3DSubModel* p_sub_model);
    };
} //namespace godot

VARIANT_ENUM_CAST(E3DSubModel::AnimationType)
VARIANT_ENUM_CAST(E3DSubModel::SubModelType)