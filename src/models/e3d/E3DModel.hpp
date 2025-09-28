#pragma once
#include "E3DSubModel.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <macros.hpp>

namespace godot {
    class E3DModel: public Resource {
        GDCLASS(E3DModel, Resource)
        protected:
            static void _bind_methods();
        MAKE_MEMBER_GS(String, name, "");
        MAKE_MEMBER_GS_NR(TypedArray<E3DSubModel>, submodels, TypedArray<E3DSubModel>());

        void add_child(const E3DSubModel& sub_model);
    };
} // namespace godot