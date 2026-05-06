#include "E3DInstancer.hpp"

#include "E3DModelInstance.hpp"
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void E3DInstancer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("instantiate", "target_node", "model", "editable"), &E3DInstancer::instantiate);
        ClassDB::bind_method(D_METHOD("clear", "target_node"), &E3DInstancer::clear);
        ClassDB::bind_method(D_METHOD("sync", "target_node"), &E3DInstancer::sync);
    }

    Ref<Material> E3DInstancer::_get_colored_material() {
        if (colored_material.is_valid()) {
            return colored_material;
        }

        ResourceLoader *resource_loader = ResourceLoader::get_singleton();
        if (resource_loader != nullptr) {
            colored_material = resource_loader->load("res://addons/libmaszyna/e3d/colored.material");
        }
        return colored_material;
    }

    void E3DInstancer::instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable) {
        UtilityFunctions::push_error(get_class() + ".instantiate() must be implemented");
    }

    void E3DInstancer::clear(E3DModelInstance *p_target_node) {
        UtilityFunctions::push_error(get_class() + ".clear() must be implemented");
    }

    void E3DInstancer::sync(E3DModelInstance *p_target_node) {
        UtilityFunctions::push_error(get_class() + ".sync() must be implemented");
    }

    bool E3DInstancer::_is_submodel_valid(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel) const {
        if (p_submodel.is_null() || p_submodel->get_skip_rendering()) {
            return false;
        }

        if (p_submodel->get_submodel_type() == E3DSubModel::SUBMODEL_TRANSFORM) {
            return true;
        }

        if (p_submodel->get_submodel_type() != E3DSubModel::SUBMODEL_GL_TRIANGLES) {
            return false;
        }

        Array excluded_names = p_target_node->get_exclude_node_names();
        for (int i = 0; i < excluded_names.size(); i++) {
            if (p_submodel->get_name() == String(excluded_names[i])) {
                return false;
            }
        }

        return true;
    }

    Ref<Material>
    E3DInstancer::_get_material_override(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel) {
        MaterialManager *material_manager = MaterialManager::get_instance();
        if (material_manager == nullptr) {
            return nullptr;
        }

        PackedStringArray path_parts = p_target_node->get_data_path().split("/");
        String unprefixed_model_path;
        for (int i = 1; i < path_parts.size(); i++) {
            if (!unprefixed_model_path.is_empty()) {
                unprefixed_model_path += "/";
            }
            unprefixed_model_path += path_parts[i];
        }

        if (p_submodel->get_dynamic_material()) {
            Array skins = p_target_node->get_skins();
            if (skins.size() < p_submodel->get_dynamic_material_index() + 1) {
                UtilityFunctions::push_warning(
                        "Model " + String(p_target_node->get_name()) +
                        " has no skins set, but submodel requires material #" +
                        String::num_int64(p_submodel->get_dynamic_material_index()));
                return nullptr;
            }

            const MaterialManager::Transparency transparency =
                    p_submodel->get_material_transparent() ? MaterialManager::ALPHA_SCISSOR : MaterialManager::DISABLED;
            return material_manager->get_material(
                    unprefixed_model_path, String(skins[p_submodel->get_dynamic_material_index()]), transparency);
        }

        if (p_submodel->get_material_colored()) {
            return _get_colored_material();
        }

        if (!p_submodel->get_material_name().is_empty()) {
            const MaterialManager::Transparency transparency =
                    p_submodel->get_material_transparent() ? MaterialManager::ALPHA_SCISSOR : MaterialManager::DISABLED;
            return material_manager->get_material(
                    unprefixed_model_path, p_submodel->get_material_name(), transparency, false,
                    p_submodel->get_diffuse_color());
        }

        return nullptr;
    }

    bool E3DInstancer::_requires_alpha_depth_prepass_sorting(const Ref<Material> &p_material) const {
        const BaseMaterial3D *base_material = Object::cast_to<BaseMaterial3D>(p_material.ptr());
        if (base_material == nullptr) {
            return false;
        }

        return base_material->get_transparency() == BaseMaterial3D::TRANSPARENCY_ALPHA_DEPTH_PRE_PASS;
    }

} // namespace godot
