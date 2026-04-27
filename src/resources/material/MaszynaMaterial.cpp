#include "MaszynaMaterial.hpp"
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    MaszynaMaterial::MaszynaMaterial() = default;

    void MaszynaMaterial::apply_to_material(const Ref<StandardMaterial3D> &p_material) const {
        p_material->set_transparency(transparency);
        switch (transparency) {
            case BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR: {
                p_material->set_alpha_scissor_threshold(0.1);
                break;
            }
            default:;
        }

        if (shader == "shadowlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
            p_material->set_flag(BaseMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
        } else if (shader == "sunlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
        }
    }

    void MaszynaMaterial::_bind_methods() {
        BIND_PROPERTY(
                Variant::STRING, "albedo_texture_path", "albedo_texture_path",
                &MaszynaMaterial::set_albedo_texture_path, &MaszynaMaterial::get_albedo_texture_path,
                "albedo_texture_path");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "transparency", "transparency", &MaszynaMaterial::set_transparency,
                &MaszynaMaterial::get_transparency, "transparency", PROPERTY_HINT_ENUM, "Disabled,Alpha,AlphaScissor");
        BIND_PROPERTY(
                Variant::STRING, "normal_texture_path", "normal_texture_path",
                &MaszynaMaterial::set_normal_texture_path, &MaszynaMaterial::get_normal_texture_path,
                "normal_texture_path");
        BIND_PROPERTY(
                Variant::STRING, "winter_albedo_texture_path", "winter_albedo_texture_path",
                &MaszynaMaterial::set_winter_albedo_texture_path, &MaszynaMaterial::get_winter_albedo_texture_path,
                "winter_albedo_texture_path");
        BIND_PROPERTY(
                Variant::STRING, "winter_normal_texture_path", "winter_normal_texture_path",
                &MaszynaMaterial::set_winter_normal_texture_path, &MaszynaMaterial::get_winter_normal_texture_path,
                "winter_normal_texture_path");
        BIND_PROPERTY(
                Variant::STRING, "autumn_albedo_texture_path", "autumn_albedo_texture_path",
                &MaszynaMaterial::set_autumn_albedo_texture_path, &MaszynaMaterial::get_autumn_albedo_texture_path,
                "autumn_albedo_texture_path");
        BIND_PROPERTY(
                Variant::STRING, "shader", "shader", &MaszynaMaterial::set_shader, &MaszynaMaterial::get_shader,
                "shader");
        BIND_PROPERTY(
                Variant::INT, "shadow_rank", "shadow_rank", &MaszynaMaterial::set_shadow_rank,
                &MaszynaMaterial::get_shadow_rank, "shadow_rank");
    }

    String MaszynaMaterial::_parse_texture_path(const String &p_texture_path) {
        const PackedStringArray parts = p_texture_path.split(":");
        String tex = parts.get(0);
        if (parts.has("t")) {
            transparency = StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR;
        }

        return tex;
    }

    String MaszynaMaterial::get_albedo_texture_path() const {
        return albedo_texture_path;
    }

    void MaszynaMaterial::set_albedo_texture_path(const String &p_value) {
        albedo_texture_path = p_value;
        _parse_texture_path(p_value);
    }

    String MaszynaMaterial::get_winter_albedo_texture_path() const {
        return winter_albedo_texture_path;
    }

    void MaszynaMaterial::set_winter_albedo_texture_path(const String &p_value) {
        winter_albedo_texture_path = p_value;
        _parse_texture_path(p_value);
    }

    String MaszynaMaterial::get_autumn_albedo_texture_path() const {
        return autumn_albedo_texture_path;
    }

    void MaszynaMaterial::set_autumn_albedo_texture_path(const String &p_value) {
        autumn_albedo_texture_path = p_value;
        _parse_texture_path(p_value);
    }
} // namespace godot
