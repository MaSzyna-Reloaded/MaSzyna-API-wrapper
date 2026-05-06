#include "OggVorbisFormatLoader.hpp"

#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void OggVorbisFormatLoader::_bind_methods() {}

    PackedStringArray OggVorbisFormatLoader::_get_recognized_extensions() const {
        PackedStringArray extensions;
        extensions.append("ogg");
        return extensions;
    }

    bool OggVorbisFormatLoader::_handles_type(const StringName &p_type) const {
        return p_type == StringName("Resource");
    }

    String OggVorbisFormatLoader::_get_resource_type(const String &p_path) const {
        return "Resource";
    }

    Variant OggVorbisFormatLoader::_load(
            const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
        Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
        if (file.is_null()) {
            UtilityFunctions::push_error("Failed to open OGG file: " + p_path);
            return Variant();
        }

        return AudioStreamOggVorbis::load_from_file(p_path);
    }

} // namespace godot
