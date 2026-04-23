#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include "parsers/maszyna_parser.hpp"

#include <godot_cpp/classes/file_access.hpp>

namespace godot {
    void MaterialParser::_bind_methods() {
        ClassDB::bind_method(D_METHOD("parse", "material_path", "material_name"), &MaterialParser::parse);
    }

    Ref<MaszynaMaterial> MaterialParser::parse(const String &p_material_path, const String &p_material_name) const {
        UtilityFunctions::print_verbose("[MaterialParser] Parsing material: " + p_material_path);
        Ref<MaszynaMaterial> material = memnew(MaszynaMaterial);
        if (const Ref<FileAccess> file = FileAccess::open(p_material_path, FileAccess::READ); file.is_valid()) {
            UtilityFunctions::print_verbose("[MaterialParser] Opening material file: " + p_material_path);
            MaszynaParser *parser = memnew(MaszynaParser);
            parser->initialize(file->get_buffer(static_cast<int64_t>(file->get_length())));
            const Dictionary data = {};
            TypedArray<Dictionary> current = {data};
            String key = "";
            String value = "";

            while (!parser->eof_reached()) {
                if (String token = parser->next_token(stop_key); token.ends_with(":")) {
                    key = token.split(":").get(0);
                } else {
                    if (token == "}") {
                        if (current.size() > 1) {
                            current.pop_front();
                        }
                        key = "";
                    } else if (token == "{") {
                        Dictionary sub = {};
                        if (current.size() > 0) {
                            Dictionary current_dict = current.get(0);
                            current_dict.set(key, sub);
                            current.push_front(sub);
                        }
                        key = "";
                    } else {
                        if (!token.is_empty() && current.size() > 0) {
                            Dictionary current_dict = current.get(0);
                            if (current_dict.has(key)) {
                                if (current_dict.get(key, "").get_type() != Variant::ARRAY) {
                                    Array new_dict;
                                    new_dict.push_back(current_dict.get(key, ""));
                                    current_dict.set(key, new_dict);
                                }

                                Array target_array = current_dict.get(key, "");
                                target_array.push_back(token);
                            } else {
                                current_dict.set(key, token);
                            }
                        }
                    }
                }
            }

            material->set_shader(data.get("shader", ""));
            Variant t1 = data.get("texture_diffuse", data.get("texture1", ""));
            Variant t2 = data.get("texture_normalmap", data.get("texture2", ""));

            if (t1.get_type() == Variant::ARRAY) {
                UtilityFunctions::push_warning("[MaterialParser]: More than 1 texture parameters (texture_diffuse, " + p_material_path + ") are not supported!");
                t1 = t1.get(0);
            }

            if (t2.get_type() == Variant::ARRAY) {
                UtilityFunctions::push_warning("[MaterialParser]: More than 1 texture parameters (texture_normalmap, " + p_material_path + ") are not supported!");
                t2 = t2.get(0);
            }

            material->set_albedo_texture_path(t1);
            material->set_normal_texture_path(t2);
            memdelete(parser);
        } else {
            material->set_albedo_texture_path(p_material_name);
        }
        return material;
    }
} //namespace godot