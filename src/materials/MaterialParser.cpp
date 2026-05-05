#include "MaterialParser.hpp"

#include "core/UserSettings.hpp"
#include "parsers/maszyna_parser.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    static Array _get_stop_tokens() {
        return Array::make(" ", ";", String::chr(9), String::chr(10), String::chr(13));
    }

    void MaterialParser::_bind_methods() {
        ClassDB::bind_method(D_METHOD("parse", "model_path", "material_file"), &MaterialParser::parse);
    }

    String MaterialParser::_find_material_path(const String &p_model_path, const String &p_material_file) const {
        UserSettings *user_settings = UserSettings::get_instance();
        if (user_settings == nullptr) {
            return "";
        }

        const String project_data_dir = user_settings->get_maszyna_game_dir();
        const String material_filename = p_material_file + String(".mat");
        PackedStringArray possible_paths;
        possible_paths.append(project_data_dir.path_join(p_model_path).path_join(material_filename));
        possible_paths.append(
                project_data_dir.path_join("textures").path_join(p_model_path).path_join(material_filename));
        possible_paths.append(project_data_dir.path_join(material_filename));
        possible_paths.append(project_data_dir.path_join("textures").path_join(material_filename));

        for (int i = 0; i < possible_paths.size(); i++) {
            if (FileAccess::file_exists(possible_paths[i])) {
                return possible_paths[i];
            }
        }

        return "";
    }

    Ref<MaszynaMaterial> MaterialParser::parse(const String &p_model_path, const String &p_material_file) const {
        Ref<MaszynaMaterial> material;
        material.instantiate();

        const String final_path = _find_material_path(p_model_path, p_material_file);
        if (final_path.is_empty()) {
            material->set_albedo_texture_path(p_material_file);
            return material;
        }

        Ref<FileAccess> file = FileAccess::open(final_path, FileAccess::ModeFlags::READ);
        if (file.is_null()) {
            material->set_albedo_texture_path(p_material_file);
            return material;
        }

        MaszynaParser *parser = memnew(MaszynaParser);
        parser->initialize(file->get_buffer(file->get_length()));

        Dictionary data;
        String key;
        int scope_depth = 0;

        while (!parser->eof_reached()) {
            String token = parser->next_token(_get_stop_tokens());
            if (token.ends_with(":")) {
                key = token.split(":")[0];
                continue;
            }

            if (token == "}") {
                scope_depth = MAX(scope_depth - 1, 0);
                key = "";
                continue;
            }

            if (token == "{") {
                scope_depth++;
                key = "";
                continue;
            }

            if (token.is_empty() || scope_depth > 0 || key.is_empty()) {
                continue;
            }

            if (data.has(key)) {
                Variant existing = data[key];
                if (existing.get_type() != Variant::ARRAY) {
                    Array values;
                    values.push_back(existing);
                    existing = values;
                }
                Array values = existing;
                values.push_back(token);
                data[key] = values;
            } else {
                data[key] = token;
            }
        }
        memdelete(parser);

        material->set_shader(data.get("shader", ""));

        Variant albedo_value = data.get("texture_diffuse", data.get("texture1", ""));
        Variant normal_value = data.get("texture_normalmap", data.get("texture2", ""));

        if (albedo_value.get_type() == Variant::ARRAY) {
            UtilityFunctions::push_warning(
                    "Material: " + final_path + ": more than 1 texture parameters aren't supported!");
            Array albedo_array = albedo_value;
            albedo_value = albedo_array.is_empty() ? Variant("") : albedo_array[0];
        }

        if (normal_value.get_type() == Variant::ARRAY) {
            UtilityFunctions::push_warning(
                    "Material: " + final_path + ": more than 1 texture parameters aren't supported!");
            Array normal_array = normal_value;
            normal_value = normal_array.is_empty() ? Variant("") : normal_array[0];
        }

        material->set_albedo_texture_path(albedo_value);
        material->set_normal_texute_path(normal_value);
        return material;
    }

} // namespace godot
