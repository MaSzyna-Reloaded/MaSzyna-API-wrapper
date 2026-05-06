#pragma once

#include "E3DInstancer.hpp"

namespace godot {

    class E3DNodesInstancer : public E3DInstancer {
            GDCLASS(E3DNodesInstancer, E3DInstancer)

        private:
            void _do_add_submodels(
                    E3DModelInstance *p_target_node, Node *p_parent, const Array &p_submodels, bool p_editable);
            Node *_create_submodel_instance(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel);
            void _update_submodel_material(
                    E3DModelInstance *p_target_node, Node *p_subnode, const Ref<E3DSubModel> &p_submodel);

        protected:
            static void _bind_methods();

        public:
            static E3DNodesInstancer *get_instance() {
                return dynamic_cast<E3DNodesInstancer *>(Engine::get_singleton()->get_singleton("E3DNodesInstancer"));
            }

            void instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable) override;
            void clear(E3DModelInstance *p_target_node) override;
            void sync(E3DModelInstance *p_target_node) override;
    };

} // namespace godot
