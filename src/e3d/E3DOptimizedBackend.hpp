#pragma once
#include "E3DInstanceBackend.hpp"

namespace godot {
    /// Renders submodels as RenderingServer instances, without creating any nodes.
    class E3DOptimizedBackend : public E3DInstanceBackend {
        private:
            void _add_submodels(
                    E3DInstanceData &p_instance, const TypedArray<E3DSubModel> &p_submodels,
                    const Transform3D &p_parent_transform, const Vector<E3DSubModel *> &p_parent_chain,
                    const Vector<E3DSubModel *> &p_force_alpha_submodels, bool p_force_alpha,
                    E3DMaterialResolver &p_material_resolver);
            void _add_submodel(
                    E3DInstanceData &p_instance, E3DSubModel *p_submodel, const Transform3D &p_local_transform,
                    const Vector<E3DSubModel *> &p_chain, bool p_force_alpha, E3DMaterialResolver &p_material_resolver);

        public:
            void build(E3DInstanceData &p_instance, E3DMaterialResolver &p_material_resolver) override;
            void clear(E3DInstanceData &p_instance) override;
            void update(const E3DInstanceData &p_instance) override;
            void apply_transform(const E3DInstanceData &p_instance) override;
            void apply_poses(E3DInstanceData &p_instance) override;
    };
} // namespace godot
