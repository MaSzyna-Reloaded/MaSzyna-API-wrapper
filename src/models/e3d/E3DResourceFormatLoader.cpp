#include "E3DResourceFormatLoader.hpp"
#include <godot_cpp/classes/file_access.hpp>

namespace godot {

    void E3DResourceFormatLoader::_bind_methods() {}

    PackedStringArray E3DResourceFormatLoader::_get_recognized_extensions() const {
        PackedStringArray arr;
        arr.push_back("e3d");
        return arr;
    }

    bool E3DResourceFormatLoader::_handles_type(const StringName &p_type) const {
        return p_type == StringName("E3DModel");
    }

    String E3DResourceFormatLoader::_get_resource_type(const String &p_path) const {
        return "E3DModel";
    }

    Variant E3DResourceFormatLoader::_load(
            const String &p_path, const String &p_original_path, const bool p_use_sub_threads, const int32_t p_cache_mode) const {
        const Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
        if (!file.is_valid()) {
            UtilityFunctions::push_error("Failed to open E3D file: " + p_path);
            return Variant();
        }

        E3DParser *parser = memnew(E3DParser);
        Ref<E3DModel> model = parser->parse(file);
        memdelete(parser);
        return model;
        // return ResourceFormatLoader::_load(p_path, p_original_path, p_use_sub_threads, p_cache_mode);
    }
} // namespace godot
