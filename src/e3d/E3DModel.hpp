#pragma once
#include "E3DSubModel.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <macros.hpp>

namespace godot {
    class E3DModel : public Resource {
            GDCLASS(E3DModel, Resource)
        public:
            static constexpr int FORMAT_VERSION = 20260504; // must be incremented when public API of E3DModel or
                                                            // E3DSubModel is changing
            ~E3DModel() override;

        protected:
            static void _bind_methods();
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<E3DSubModel>, submodels);

            void add_child(const Ref<E3DSubModel> &p_sub_model);
            void clear();
    };
} // namespace godot
