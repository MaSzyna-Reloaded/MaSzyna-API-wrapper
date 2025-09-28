#include "E3DModel.hpp"

namespace godot {

    void E3DModel::_bind_methods() {}

    void E3DModel::add_child(const E3DSubModel &sub_model) {
        submodels.append(&sub_model);
    }
} // namespace godot