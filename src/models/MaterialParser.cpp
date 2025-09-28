#include "MaterialParser.hpp"
#include "MaterialManager.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include "parsers/maszyna_parser.hpp"

namespace godot {
    void MaterialParser::_bind_methods() {}

    Ref<MaszynaMaterial> MaterialParser::parse(const String &model_path, const String &material_path) {
        manager = memnew(MaterialManager);
        MaszynaMaterial material;
        String final_path = manager->get_material_path(model_path, material_path);
        if (const Ref<FileAccess> file = FileAccess::open(material_path, FileAccess::READ); file.is_valid()) {
            parser = memnew(MaszynaParser);
            parser->initialize(file->get_buffer(file->get_length()));
            const Dictionary data = {};
            TypedArray<Dictionary> current = {data};
            String key = "";
            String value = "";

            while (!parser->eof_reached()) {
                if (String token = parser->next_token(STOP_KEY); token.ends_with(":")) {
                    key = token.split(":")[0];
                } else {
                    if (token == "}") {
                        current.pop_front();
                        key = "";
                    } else if (token == "{") {
                        Dictionary sub = {};
                        current[0].get(key) = sub;
                        key = "";
                        current.push_front(sub);
                    } else {
                        if (!token.is_empty()) {
                            if (Dictionary current_dict = current[0]; current_dict.has(key)) {
                                if (current_dict[key].get_type() != Variant::ARRAY) {
                                    Array _new;
                                    _new.push_back(current_dict[key]);
                                    current_dict[key] = _new;
                                }

                                Array target_array = current_dict[key];
                                target_array.push_back(token);
                            } else {
                                current_dict[key] = token;
                            }
                        }
                    }
                }
            }

            memdelete(parser);
        }

        memdelete(manager);
        return &material;
    }
} //namespace godot
