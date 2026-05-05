#include "E3DResourceFormatLoader.hpp"

#include "parsers/e3d_parser.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void E3DResourceFormatLoader::_bind_methods() {}

    PackedStringArray E3DResourceFormatLoader::_get_recognized_extensions() const {
        PackedStringArray extensions;
        extensions.append("e3d");
        return extensions;
    }

    bool E3DResourceFormatLoader::_handles_type(const StringName &p_type) const {
        return p_type == StringName("E3DModel") || p_type == StringName("Resource");
    }

    String E3DResourceFormatLoader::_get_resource_type(const String &p_path) const {
        return p_path.get_extension().to_lower() == "e3d" ? "E3DModel" : "";
    }

    Variant E3DResourceFormatLoader::_load(
            const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
        Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
        if (file.is_null()) {
            UtilityFunctions::push_error("Failed to open E3D file: " + p_path);
            return Variant();
        }

        E3DParser *parser = E3DParser::get_instance();
        if (parser == nullptr) {
            UtilityFunctions::push_error("E3DParser singleton is not available");
            return Variant();
        }

        return parser->parse(file);
    }

} // namespace godot
