#pragma once
#include "macros.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    class E3DSubModel: public Resource {
        GDCLASS(E3DSubModel, Resource)
        public:
            ~E3DSubModel() override;
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

            MAKE_MEMBER_GS_NR(String, name, "")
            MAKE_MEMBER_GS_NR(SubModelType, submodel_type, GL_TRIANGLES)
            MAKE_MEMBER_GS_NR(AnimationType, animation, NONE)
            MAKE_MEMBER_GS_NR(float, lights_on_threshold, 0.0)
            MAKE_MEMBER_GS_NR(float, visibility_light, 0.0)
            MAKE_MEMBER_GS_NR(float, visibility_range_begin, 0.0)
            MAKE_MEMBER_GS_NR(float, visibility_range_end, 0.0)
            MAKE_MEMBER_GS_NR(Color, diffuse_color, Color(1.0, 1.0, 1.0, 1.0))
            MAKE_MEMBER_GS_NR(Color, self_illumination, Color(1.0, 1.0, 1.0, 1.0))
            MAKE_MEMBER_GS_NR(bool, material_colored, false)
            MAKE_MEMBER_GS_NR(bool, dynamic_material, false)
            MAKE_MEMBER_GS_NR(int, dynamic_material_index, 0)
            MAKE_MEMBER_GS_NR(bool, material_transparent, false)
            MAKE_MEMBER_GS_NR(String, material_name, "")
            MAKE_MEMBER_GS_NR(Transform3D, transform, Transform3D())
            MAKE_MEMBER_GS_NR(Transform3D, material_transform, Transform3D())
            MAKE_MEMBER_GS_NR(Ref<ArrayMesh>, mesh, Ref<ArrayMesh>())
            MAKE_MEMBER_GS_NR(TypedArray<E3DSubModel>, submodels, TypedArray<E3DSubModel>())
            MAKE_MEMBER_GS_NR(bool, visible, true)
            MAKE_MEMBER_GS_NR(bool, skip_rendering, false)

            void add_child(const Ref<E3DSubModel>& p_sub_model);
            void set_parent(E3DSubModel* p_sub_model);
            void clear();
    };
} //namespace godot

VARIANT_ENUM_CAST(E3DSubModel::AnimationType)
VARIANT_ENUM_CAST(E3DSubModel::SubModelType)
