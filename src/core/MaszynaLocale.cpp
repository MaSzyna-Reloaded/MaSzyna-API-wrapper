#include "MaszynaLocale.hpp"
#include "core/MaszynaRuntime.hpp"
#include "core/UserSettings.hpp"
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    MaszynaLocale::MaszynaLocale() {
        UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL(user_settings);
        MaszynaRuntime *runtime = MaszynaRuntime::get_instance();
        ERR_FAIL_NULL(runtime);
        user_settings->connect("game_dir_changed", callable_mp(this, &MaszynaLocale::_on_game_dir_changed));
        runtime->connect(
                MaszynaRuntime::language_changed_signal, callable_mp(this, &MaszynaLocale::_on_language_changed));
        _on_game_dir_changed();
    }

    void MaszynaLocale::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_translation", "po_path"), &MaszynaLocale::load_translation);
        ClassDB::bind_method(D_METHOD("gettext", "msgid"), &MaszynaLocale::gettext);
        ClassDB::bind_method(D_METHOD("get_languages"), &MaszynaLocale::get_languages);
    }

    void MaszynaLocale::_on_game_dir_changed() {
        languages.clear();
        languages.push_back(MaszynaRuntime::DEFAULT_LANGUAGE);
        const String lang_dir = UserSettings::get_instance()->get_maszyna_game_dir().path_join("lang");
        // a game directory without translations is English alone
        const PackedStringArray files =
                DirAccess::dir_exists_absolute(lang_dir) ? DirAccess::get_files_at(lang_dir) : PackedStringArray();
        for (const String &file : files) {
            // template.po is the catalogue translators start from, with every msgstr empty
            if (file.get_extension().to_lower() == "po" && file.get_basename() != "template") {
                languages.push_back(file.get_basename());
            }
        }
        _on_language_changed();
    }

    void MaszynaLocale::_on_language_changed() {
        const String language = MaszynaRuntime::get_instance()->get_language();
        // locale::init(), translation.cpp:18 - "lang/" + Global.asLang + ".po"
        load_translation(
                UserSettings::get_instance()->get_maszyna_game_dir().path_join("lang").path_join(language + String(".po")));
        // the wrapper's own strings, which the game's catalogue does not have, follow the same
        // language through Godot's own translations (addons/libmaszyna/translations)
        TranslationServer::get_singleton()->set_locale(language);
    }

    PackedStringArray MaszynaLocale::get_languages() const {
        return languages;
    }

    void MaszynaLocale::load_translation(const String &p_po_path) {
        translation.unref();
        if (!FileAccess::file_exists(p_po_path)) {
            UtilityFunctions::print_verbose("MaszynaLocale: cannot open lang file: ", p_po_path);
            return;
        }
        translation = ResourceLoader::get_singleton()->load(p_po_path);
    }

    String MaszynaLocale::gettext(const String &p_msgid) const {
        const String message = translation.is_valid() ? String(translation->get_message(p_msgid)) : String();
        if (!message.is_empty()) {
            return message;
        }
        // the wrapper's own strings, which the game's catalogue does not have
        // (addons/libmaszyna/translations), else the msgid itself
        return TranslationServer::get_singleton()->translate(p_msgid);
    }
} // namespace godot
