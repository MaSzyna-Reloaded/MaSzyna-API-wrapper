#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include "parsers/maszyna_parser.hpp"

#include <godot_cpp/classes/file_access.hpp>

namespace godot {
    void MaterialParser::_bind_methods() {
        ClassDB::bind_method(D_METHOD("parse", "model_path", "material_path"), &MaterialParser::parse);
    }

    Ref<MaszynaMaterial> MaterialParser::parse(const String &model_path, const String &material_path) {
        Ref<MaszynaMaterial> material = memnew(MaszynaMaterial);
        material_manager = memnew(MaterialManager);
        const String final_path = material_manager->get_material_path(model_path, material_path);
        if (const Ref<FileAccess> file = FileAccess::open(final_path, FileAccess::READ); file.is_valid()) {
            parser = memnew(MaszynaParser);
            parser->initialize(file->get_buffer(static_cast<int64_t>(file->get_length())));
            const Dictionary data = {};
            TypedArray<Dictionary> current = {data};
            String key = "";
            String value = "";

            while (!parser->eof_reached()) {
                if (String token = parser->next_token(STOP_KEY); token.ends_with(":")) {
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
                                    Array _new;
                                    _new.push_back(current_dict.get(key, ""));
                                    current_dict.set(key, _new);
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
            auto _t1 = data.get("texture_diffuse", data.get("texture1", ""));
            auto _t2 = data.get("texture_normalmap", data.get("texture2", ""));

            if (_t1.get_type() == Variant::ARRAY) {
                UtilityFunctions::push_warning("[MaterialParser]: More than 1 texture parameters (texture_diffuse, " + final_path + ") are not supported!");
                _t1 = _t1.get(0);
            }

            if (_t2.get_type() == Variant::ARRAY) {
                UtilityFunctions::push_warning("[MaterialParser]: More than 1 texture parameters (texture_normalmap, " + final_path + ") are not supported!");
                _t2 = _t2.get(0);
            }

            material->set_albedo_texture_path(_t1);
            material->set_normal_texture_path(_t2);
            memdelete(parser);
            parser = nullptr;
        } else {
            material->set_albedo_texture_path(material_path);
        }
        return material;
    }
} //namespace godot
