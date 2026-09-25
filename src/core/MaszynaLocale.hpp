#pragma once
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/translation.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
    /// The original engine's `locale` (translation.cpp:18-160): the game's own translation,
    /// `<game_dir>/lang/<language>.po`, looked up by its English msgid.
    ///
    /// Kept apart from TranslationServer on purpose - the game's catalogue translates the
    /// original's strings, not this project's UI. The language is MaszynaRuntime's, the file is
    /// read by Godot's own PO loader.
    class MaszynaLocale : public Object {
            GDCLASS(MaszynaLocale, Object)

        public:
            static MaszynaLocale *get_instance() {
                return Object::cast_to<MaszynaLocale>(Engine::get_singleton()->get_singleton("MaszynaLocale"));
            }

        private:
            Ref<Translation> translation;
            PackedStringArray languages;

            void _on_game_dir_changed();
            void _on_language_changed();

        protected:
            static void _bind_methods();

        public:
            MaszynaLocale();

            /// Replaces the catalogue with the one in `p_po_path`; with no such file every msgid
            /// stays untranslated, as in the original (translation.cpp:21)
            void load_translation(const String &p_po_path);
            /// The translation of `p_msgid` from the game's catalogue, else from the wrapper's own
            /// (Godot's translations, addons/libmaszyna/translations), else `p_msgid` itself
            /// (locale::lookup_s, translation.cpp:33)
            String gettext(const String &p_msgid) const;
            /// The languages there is a catalogue for in the game's `lang/`, English (the msgids
            /// themselves) always first
            PackedStringArray get_languages() const;
    };
} // namespace godot
