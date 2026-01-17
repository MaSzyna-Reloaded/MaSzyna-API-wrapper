#include "MaszynaMaterial.hpp"
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    MaszynaMaterial::MaszynaMaterial() :
        albedo_texture_path(String(), [this](const String &newVal) {
                    // Do not re-assign to albedo_texture_path here to avoid infinite recursion.
                    // The value is already set when the callback is called.
                    // If we need to modify the value, we should do it BEFORE it's set or in a way that doesn't trigger the callback.
                    // For now, _parse_texture_path handles transparency side-effects.
                    _parse_texture_path(newVal);
                }),
    winter_albedo_texture_path(String(), [this](const String &newVal) {
                    _parse_texture_path(newVal);
                }),
    autumn_albedo_texture_path(String(), [this](const String &newVal) {
                    _parse_texture_path(newVal);
                }) {}

    void MaszynaMaterial::apply_to_material(StandardMaterial3D *p_material) const {
        p_material->set_transparency(transparency);
        switch (transparency) {
            case BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR: {
                p_material->set_alpha_scissor_threshold(0.1);
                break;
            }
            default: ;
        }

        if (shader == "shadowlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
            p_material->set_flag(BaseMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
        } else if (shader == "sunlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
        }
    }

    void MaszynaMaterial::_bind_methods() {
        BIND_PROPERTY(Variant::STRING, "albedo_texture_path", "albedo_texture_path", &MaszynaMaterial::set_albedo_texture_path, &MaszynaMaterial::get_albedo_texture_path, "albedo_texture_path");
        // BIND_PROPERTY_W_HINT(Variant::INT, "transparency", "transparency", &MaszynaMaterial::set_transparency, &MaszynaMaterial::get_transparency, "transparency", PROPERTY_HINT_ENUM, "Disabled,Alpha,AlphaScissor");
        BIND_PROPERTY(Variant::STRING, "normal_texture_path", "normal_texture_path", &MaszynaMaterial::set_normal_texture_path, &MaszynaMaterial::get_normal_texture_path, "normal_texture_path");
        BIND_PROPERTY(Variant::STRING, "winter_albedo_texture_path", "winter_albedo_texture_path", &MaszynaMaterial::set_winter_albedo_texture_path, &MaszynaMaterial::get_winter_albedo_texture_path, "winter_albedo_texture_path");
        BIND_PROPERTY(Variant::STRING, "winter_normal_texture_path", "winter_normal_texture_path", &MaszynaMaterial::set_winter_normal_texture_path, &MaszynaMaterial::get_winter_normal_texture_path, "winter_normal_texture_path");
        BIND_PROPERTY(Variant::STRING, "autumn_albedo_texture_path", "autumn_albedo_texture_path", &MaszynaMaterial::set_autumn_albedo_texture_path, &MaszynaMaterial::get_autumn_albedo_texture_path, "autumn_albedo_texture_path");
        BIND_PROPERTY(Variant::STRING, "shader", "shader", &MaszynaMaterial::set_shader, &MaszynaMaterial::get_shader, "shader");
        BIND_PROPERTY(Variant::INT, "shadow_rank", "shadow_rank", &MaszynaMaterial::set_shadow_rank, &MaszynaMaterial::get_shadow_rank, "shadow_rank");
    }

    String MaszynaMaterial::_parse_texture_path(const String &texturePath) {
        PackedStringArray _parts = texturePath.split(":");
        String _tex = _parts[0];
        if (_parts.has("t")) {
            transparency = StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR;
        }

        return _tex;
    }
}
