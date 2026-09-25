#include "MaszynaTranslationServer.hpp"
#include "core/MaszynaRuntime.hpp"
#include "core/UserSettings.hpp"
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    MaszynaTranslationServer::MaszynaTranslationServer() {
        UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL(user_settings);
        MaszynaRuntime *runtime = MaszynaRuntime::get_instance();
        ERR_FAIL_NULL(runtime);
        user_settings->connect("game_dir_changed", callable_mp(this, &MaszynaTranslationServer::_on_game_dir_changed));
        runtime->connect(
                MaszynaRuntime::language_changed_signal,
                callable_mp(this, &MaszynaTranslationServer::_on_language_changed));
        _on_game_dir_changed();
    }

    MaszynaTranslationServer::~MaszynaTranslationServer() {
        if (translation.is_valid()) {
            TranslationServer::get_singleton()->remove_translation(translation);
        }
    }

    void MaszynaTranslationServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("load_translation", "po_path"), &MaszynaTranslationServer::load_translation);
        ClassDB::bind_method(D_METHOD("get_languages"), &MaszynaTranslationServer::get_languages);
    }

    void MaszynaTranslationServer::_on_game_dir_changed() {
        languages.clear();
        languages.push_back(MaszynaRuntime::DEFAULT_LANGUAGE);
        const String lang_dir = UserSettings::get_instance()->get_maszyna_game_dir().path_join("lang");
        // a game directory without translations is English alone
        const PackedStringArray files =
                DirAccess::dir_exists_absolute(lang_dir) ? DirAccess::get_files_at(lang_dir) : PackedStringArray();
        for (const String &file: files) {
            // template.po is the catalogue translators start from, with every msgstr empty
            if (file.get_extension().to_lower() == "po" && file.get_basename() != "template") {
                languages.push_back(file.get_basename());
            }
        }
        _on_language_changed();
    }

    void MaszynaTranslationServer::_on_language_changed() {
        const String language = MaszynaRuntime::get_instance()->get_language();
        // locale::init(), translation.cpp:18 - "lang/" + Global.asLang + ".po"
        load_translation(
                UserSettings::get_instance()->get_maszyna_game_dir().path_join("lang").path_join(
                        language + String(".po")));
    }

    PackedStringArray MaszynaTranslationServer::get_languages() const {
        return languages;
    }

    void MaszynaTranslationServer::load_translation(const String &p_po_path) {
        Ref<Translation> game_translation;
        if (FileAccess::file_exists(p_po_path)) {
            game_translation = ResourceLoader::get_singleton()->load(p_po_path);
        } else {
            UtilityFunctions::print_verbose("MaszynaTranslationServer: cannot open lang file: ", p_po_path);
        }

        TranslationServer *translation_server = TranslationServer::get_singleton();
        const String language = MaszynaRuntime::get_instance()->get_language();
        Ref<Translation> merged;
        merged.instantiate();
        merged->set_locale(language);

        // the game's catalogues carry neither msgctxt nor plurals, nor do the wrapper's - only the
        // plain singular messages are merged
        const PackedStringArray wrapper_paths =
                ProjectSettings::get_singleton()->get_setting(TRANSLATIONS_SETTING, PackedStringArray());
        for (const String &path: wrapper_paths) {
            const Ref<Translation> wrapper_translation = ResourceLoader::get_singleton()->load(path);
            ERR_CONTINUE_MSG(wrapper_translation.is_null(), "MaszynaTranslationServer: cannot load " + path);
            if (translation_server->compare_locales(language, wrapper_translation->get_locale()) <= 0) {
                continue;
            }
            for (const String &message: wrapper_translation->get_message_list()) {
                merged->add_message(message, wrapper_translation->get_message(message));
            }
        }
        if (game_translation.is_valid()) {
            for (const String &message: game_translation->get_message_list()) {
                merged->add_message(message, game_translation->get_message(message));
            }
        }

        if (translation.is_valid()) {
            translation_server->remove_translation(translation);
        }
        translation = merged;
        translation_server->add_translation(translation);

        // set_locale() tells the tree to re-translate only when the locale changes; a new catalogue
        // in the same language has to say so itself
        if (translation_server->get_locale() == translation_server->standardize_locale(language)) {
            if (MainLoop *main_loop = Engine::get_singleton()->get_main_loop()) {
                main_loop->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
            }
            return;
        }
        translation_server->set_locale(language);
    }
} // namespace godot
