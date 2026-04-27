#pragma once
#include "E3DSubModel.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <macros.hpp>

namespace godot {
    class E3DModel : public Resource {
            GDCLASS(E3DModel, Resource)
        protected:
            static void _bind_methods();
            TypedArray<E3DSubModel> submodels;

        public:
            void add_child(const Ref<E3DSubModel> &p_sub_model);
            void clear();
            void set_submodels(const TypedArray<E3DSubModel> &p_submodels);
            TypedArray<E3DSubModel> get_submodels() const;
            AABB get_aabb() const;
    };
} // namespace godot
