#include "E3DSubModel.hpp"

namespace godot {
    void E3DSubModel::_bind_methods() {
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
    }

    void E3DSubModel::add_child(const Ref<E3DSubModel> &p_sub_model) {
        submodels.append(p_sub_model);
    }

    void E3DSubModel::set_parent(E3DSubModel *p_sub_model) {
        parent = p_sub_model;
        if (p_sub_model != nullptr) {
            p_sub_model->add_child(Ref<E3DSubModel>(this));
        }
    }
} //namespace godot
