#include "E3DModel.hpp"

namespace godot {
    E3DModel::~E3DModel() {
        clear();
    }

    void E3DModel::clear() {
        for (int i = 0; i < submodels.size(); i++) {
            Ref<E3DSubModel> sm = submodels.get(i);
            if (sm.is_valid()) {
                sm->clear();
            }
        }
        submodels.clear();
    }

    void E3DModel::_bind_methods() {
        BIND_PROPERTY(Variant::STRING, "name", "name", &E3DModel::set_name, &E3DModel::get_name, "p_name");
        BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "submodels", "submodels", &E3DModel::set_submodels, &E3DModel::get_submodels, "p_submodels", PROPERTY_HINT_ARRAY_TYPE, "E3DSubModel");
    }

    void E3DModel::add_child(const Ref<E3DSubModel> &p_sub_model) {
        submodels.append(p_sub_model);
    }
} // namespace godot
