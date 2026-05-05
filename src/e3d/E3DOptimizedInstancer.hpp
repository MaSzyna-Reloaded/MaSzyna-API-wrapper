#pragma once

#include "E3DInstancer.hpp"
#include <unordered_map>
#include <vector>

namespace godot {

    class E3DOptimizedInstancer : public E3DInstancer {
            GDCLASS(E3DOptimizedInstancer, E3DInstancer)

        private:
            struct InstanceData {
                    RID rid;
                    Transform3D local_transform;
            };

            struct InstancerEntry {
                    std::vector<Ref<Material>> materials;
                    std::vector<InstanceData> instances;
            };

            std::unordered_map<uint64_t, InstancerEntry> instances_by_target;

            void _add_submodels(
                    InstancerEntry &p_entry, E3DModelInstance *p_target_node, const Array &p_submodels,
                    const Transform3D &p_parent_transform, const RID &p_scenario);
            void _add_submodel(
                    InstancerEntry &p_entry, E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel,
                    const Transform3D &p_local_transform, const RID &p_scenario);

        protected:
            static void _bind_methods();

        public:
            static E3DOptimizedInstancer *get_instance() {
                return dynamic_cast<E3DOptimizedInstancer *>(
                        Engine::get_singleton()->get_singleton("E3DOptimizedInstancer"));
            }

            void instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable) override;
            void clear(E3DModelInstance *p_target_node) override;
            void sync(E3DModelInstance *p_target_node) override;
    };

} // namespace godot
