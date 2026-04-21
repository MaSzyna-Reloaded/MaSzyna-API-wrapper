#pragma once

#include "models/e3d/E3DModel.hpp"
#include "models/e3d/instance/E3DModelInstance.hpp"
#include <vector>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {
    class E3DRenderingServerInstancer {
        public:
            struct InstanceRecord {
                RID instance_rid;
                Transform3D local_transform;
                bool visible;
                String name;
            };

        private:
            static void _append_submodels(
                    const E3DModelInstance &p_target_node,
                    const TypedArray<E3DSubModel> &submodels,
                    const Transform3D &parent_transform,
                    bool parent_visible,
                    std::vector<InstanceRecord> &instances);

        public:
            static std::vector<InstanceRecord> instantiate(
                    const Ref<E3DModel> &p_model,
                    const E3DModelInstance &p_target_node,
                    const RID &scenario);
    };
} // namespace godot
