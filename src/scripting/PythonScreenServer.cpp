#include "PythonScreenServer.hpp"
#include "core/UserSettings.hpp"
#include <cstdint>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace godot {
    namespace {
        /// The part of the CPython 2.7 C API the screens need, resolved from the library at run
        /// time. PyObject stays opaque: references are counted through Py_IncRef/Py_DecRef, which
        /// 2.7 exports as functions, so no Python header is needed to build the extension.
        struct PyObject;
        using Py_ssize_t = intptr_t;

        struct PythonApi {
                int *Py_IgnoreEnvironmentFlag = nullptr;
                int *Py_DontWriteBytecodeFlag = nullptr;
                int *Py_NoUserSiteDirectory = nullptr;
                void (*Py_SetPythonHome)(char *) = nullptr;
                void (*Py_InitializeEx)(int) = nullptr;
                void (*Py_Finalize)() = nullptr;
                int (*PyRun_SimpleStringFlags)(const char *, void *) = nullptr;
                PyObject *(*PyImport_AddModule)(const char *) = nullptr;
                PyObject *(*PyObject_GetAttrString)(PyObject *, const char *) = nullptr;
                int (*PyObject_SetAttrString)(PyObject *, const char *, PyObject *) = nullptr;
                PyObject *(*PyObject_CallFunction)(PyObject *, char *, ...) = nullptr;
                PyObject *(*PyObject_CallMethod)(PyObject *, char *, char *, ...) = nullptr;
                PyObject *(*PyDict_New)() = nullptr;
                PyObject *(*PyList_New)(Py_ssize_t) = nullptr;
                int (*PyList_Append)(PyObject *, PyObject *) = nullptr;
                PyObject *(*Py_BuildValue)(const char *, ...) = nullptr;
                int (*PyDict_SetItemString)(PyObject *, const char *, PyObject *) = nullptr;
                PyObject *(*PyBool_FromLong)(long) = nullptr;
                PyObject *(*PyInt_FromLong)(long) = nullptr;
                long (*PyInt_AsLong)(PyObject *) = nullptr;
                PyObject *(*PyFloat_FromDouble)(double) = nullptr;
                PyObject *(*PyString_FromString)(const char *) = nullptr;
                char *(*PyString_AsString)(PyObject *) = nullptr;
                int (*PyString_AsStringAndSize)(PyObject *, char **, Py_ssize_t *) = nullptr;
                Py_ssize_t (*PySequence_Size)(PyObject *) = nullptr;
                PyObject *(*PySequence_GetItem)(PyObject *, Py_ssize_t) = nullptr;
                PyObject *(*PyErr_Occurred)() = nullptr;
                void (*PyErr_Print)() = nullptr;
                void (*Py_IncRef)(PyObject *) = nullptr;
                void (*Py_DecRef)(PyObject *) = nullptr;

                /// Loads the library and resolves every symbol; false with an error printed when
                /// either fails. The library stays loaded for the life of the process - the
                /// extension modules the scripts import (PIL) keep pointers into it.
                bool load(const String &p_library_path) {
#ifdef _WIN32
                    HMODULE library = LoadLibraryW(reinterpret_cast<LPCWSTR>(p_library_path.utf16().get_data()));
                    auto resolve = [library](const char *p_name) -> void * {
                        return reinterpret_cast<void *>(GetProcAddress(library, p_name));
                    };
#else
                    // RTLD_GLOBAL: the extension modules a script imports resolve the interpreter's
                    // symbols from the global namespace
                    void *library = dlopen(p_library_path.utf8().get_data(), RTLD_NOW | RTLD_GLOBAL);
                    auto resolve = [library](const char *p_name) -> void * { return dlsym(library, p_name); };
#endif
                    if (library == nullptr) {
                        UtilityFunctions::push_error(
                                "PythonScreenServer: cannot load ", p_library_path, " - Python screens stay blank");
                        return false;
                    }
                    bool resolved = true;
                    auto bind = [&resolve, &resolved](auto &p_function, const char *p_name) {
                        p_function = reinterpret_cast<std::remove_reference_t<decltype(p_function)>>(resolve(p_name));
                        if (p_function == nullptr) {
                            UtilityFunctions::push_error("PythonScreenServer: ", p_name, " missing from the library");
                            resolved = false;
                        }
                    };
                    bind(Py_IgnoreEnvironmentFlag, "Py_IgnoreEnvironmentFlag");
                    bind(Py_DontWriteBytecodeFlag, "Py_DontWriteBytecodeFlag");
                    bind(Py_NoUserSiteDirectory, "Py_NoUserSiteDirectory");
                    bind(Py_SetPythonHome, "Py_SetPythonHome");
                    bind(Py_InitializeEx, "Py_InitializeEx");
                    bind(Py_Finalize, "Py_Finalize");
                    bind(PyRun_SimpleStringFlags, "PyRun_SimpleStringFlags");
                    bind(PyImport_AddModule, "PyImport_AddModule");
                    bind(PyObject_GetAttrString, "PyObject_GetAttrString");
                    bind(PyObject_SetAttrString, "PyObject_SetAttrString");
                    bind(PyObject_CallFunction, "PyObject_CallFunction");
                    bind(PyObject_CallMethod, "PyObject_CallMethod");
                    bind(PyDict_New, "PyDict_New");
                    bind(PyList_New, "PyList_New");
                    bind(PyList_Append, "PyList_Append");
                    bind(Py_BuildValue, "Py_BuildValue");
                    bind(PyDict_SetItemString, "PyDict_SetItemString");
                    bind(PyBool_FromLong, "PyBool_FromLong");
                    bind(PyInt_FromLong, "PyInt_FromLong");
                    bind(PyInt_AsLong, "PyInt_AsLong");
                    bind(PyFloat_FromDouble, "PyFloat_FromDouble");
                    bind(PyString_FromString, "PyString_FromString");
                    bind(PyString_AsString, "PyString_AsString");
                    bind(PyString_AsStringAndSize, "PyString_AsStringAndSize");
                    bind(PySequence_Size, "PySequence_Size");
                    bind(PySequence_GetItem, "PySequence_GetItem");
                    bind(PyErr_Occurred, "PyErr_Occurred");
                    bind(PyErr_Print, "PyErr_Print");
                    bind(Py_IncRef, "Py_IncRef");
                    bind(Py_DecRef, "Py_DecRef");
                    return resolved;
                }

                /// Prints the pending exception, if there is one; true when there was
                bool print_error() const {
                    if (PyErr_Occurred() == nullptr) {
                        return false;
                    }
                    PyErr_Print();
                    return true;
                }

                /// Runs a script file in __main__, the way the original runs every script
                /// (python_taskqueue::run_file()) - the class it defines lands there
                bool run_file(PyObject *p_main, const String &p_path) const {
                    PyObject *path = PyString_FromString(p_path.utf8().get_data());
                    PyObject_SetAttrString(p_main, "_maszyna_script_path", path);
                    Py_DecRef(path);
                    return PyRun_SimpleStringFlags("execfile(_maszyna_script_path)", nullptr) == 0;
                }

                /// A value of the state as a Python object, with the types the original passes
                /// (dictionary_source: floats, integers, bools, strings and lists of 2D points);
                /// nullptr for any other type
                PyObject *to_object(const Variant &p_value) const {
                    switch (p_value.get_type()) {
                        case Variant::BOOL:
                            return PyBool_FromLong(static_cast<bool>(p_value) ? 1 : 0);
                        case Variant::INT:
                            return PyInt_FromLong(static_cast<long>(static_cast<int64_t>(p_value)));
                        case Variant::FLOAT:
                            return PyFloat_FromDouble(p_value);
                        case Variant::STRING:
                        case Variant::STRING_NAME:
                            return PyString_FromString(String(p_value).utf8().get_data());
                        case Variant::VECTOR2: {
                            const Vector2 point = p_value;
                            return Py_BuildValue("(dd)", static_cast<double>(point.x), static_cast<double>(point.y));
                        }
                        case Variant::ARRAY: {
                            const Array values = p_value;
                            PyObject *list = PyList_New(0);
                            for (int64_t i = 0; i < values.size(); i++) {
                                PyObject *item = to_object(values[i]);
                                if (item != nullptr) {
                                    PyList_Append(list, item);
                                    Py_DecRef(item);
                                }
                            }
                            return list;
                        }
                        default:
                            return nullptr;
                    }
                }
        };

        PythonApi python;
    } // namespace

    void PythonScreenServer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("screen_create", "script_path", "commands_received"), &PythonScreenServer::screen_create);
        ClassDB::bind_method(D_METHOD("screen_get_texture", "screen"), &PythonScreenServer::screen_get_texture);
        ClassDB::bind_method(
                D_METHOD("screen_request_render", "screen", "state"), &PythonScreenServer::screen_request_render);
        ClassDB::bind_method(D_METHOD("screen_free", "screen"), &PythonScreenServer::screen_free);
    }

    PythonScreenServer::PythonScreenServer() {
        mutex.instantiate();
        semaphore.instantiate();
    }

    PythonScreenServer::~PythonScreenServer() {
        if (worker.is_null()) {
            return;
        }
        {
            MutexLock lock(**mutex);
            exiting = true;
        }
        semaphore->post();
        worker->wait_to_finish();
    }

    RID PythonScreenServer::screen_create(const String &p_script_path, const Callable &p_commands_received) {
        if (worker.is_null()) {
            const UserSettings *user_settings = UserSettings::get_instance();
            ERR_FAIL_NULL_V(user_settings, RID());
            const String game_dir = user_settings->get_maszyna_game_dir();
            String home = ProjectSettings::get_singleton()->get_setting("maszyna/python/home", "");
#ifdef _WIN32
            // PyInt.cpp:233 - the 64-bit Windows runtime ships in the game directory
            home = home.is_empty() ? game_dir.path_join("python64") : home;
            const String library = game_dir.path_join("python27.dll");
#else
            // the original's linuxpython64 (PyInt.cpp:238) is only a virtualenv over the system's
            // libpython, without PIL - the wrapper's own runtime has a directory of its own
            home = home.is_empty() ? game_dir.path_join("python2.7") : home;
            const String library = home.path_join("lib/libpython2.7.so.1.0");
#endif
            worker.instantiate();
            worker->start(callable_mp(this, &PythonScreenServer::_worker_loop).bind(library, home, game_dir));
        }
        Ref<Image> blank = Image::create_empty(1, 1, false, Image::FORMAT_RGBA8);
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        screens.insert(rid, Screen{p_script_path, ImageTexture::create_from_image(blank), p_commands_received});
        return rid;
    }

    Ref<Texture2D> PythonScreenServer::screen_get_texture(const RID &p_screen) const {
        const Screen *screen = screens.getptr(p_screen);
        ERR_FAIL_NULL_V(screen, Ref<Texture2D>());
        return screen->texture;
    }

    void PythonScreenServer::screen_request_render(const RID &p_screen, const Dictionary &p_state) {
        const Screen *screen = screens.getptr(p_screen);
        ERR_FAIL_NULL(screen);
        {
            MutexLock lock(**mutex);
            bool replaced = false;
            for (Request &request : requests) {
                if (request.screen == p_screen) {
                    request.state = p_state.duplicate();
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                requests.push_back(Request{p_screen, screen->script_path, p_state.duplicate()});
            }
        }
        semaphore->post();
    }

    void PythonScreenServer::screen_free(const RID &p_screen) {
        screens.erase(p_screen);
        MutexLock lock(**mutex);
        for (List<Request>::Element *element = requests.front(); element != nullptr; element = element->next()) {
            if (element->get().screen == p_screen) {
                element->erase();
                break;
            }
        }
    }

    void PythonScreenServer::_publish(
            const RID &p_screen, const int p_width, const int p_height, const PackedByteArray &p_pixels,
            const PackedStringArray &p_commands) {
        Screen *screen = screens.getptr(p_screen);
        if (screen == nullptr) {
            return; // freed while it was being drawn
        }
        if (!p_pixels.is_empty()) {
            const Ref<Image> image = Image::create_from_data(p_width, p_height, false, Image::FORMAT_RGBA8, p_pixels);
            if (screen->texture->get_width() == p_width && screen->texture->get_height() == p_height) {
                screen->texture->update(image);
            } else {
                screen->texture->set_image(image);
            }
        }
        if (!p_commands.is_empty()) {
            screen->commands_received.call(p_commands);
        }
    }

    void PythonScreenServer::_worker_loop(const String &p_library, const String &p_home, const String &p_game_dir) {
        const bool loaded = python.load(p_library);
        // Py_SetPythonHome keeps the pointer, so the buffer lives as long as the interpreter
        CharString home = p_home.utf8();
        PyObject *main = nullptr;
        HashMap<String, PyObject *> renderers;
        if (loaded) {
            // the player's own Python installation must not leak in, and nothing is written
            // into the game directory (the scripts' .pyc would land next to them)
            *python.Py_IgnoreEnvironmentFlag = 1;
            *python.Py_DontWriteBytecodeFlag = 1;
            *python.Py_NoUserSiteDirectory = 1;
            python.Py_SetPythonHome(home.ptrw());
            python.Py_InitializeEx(0);
            main = python.PyImport_AddModule("__main__");
            PyObject *game_dir = python.PyString_FromString(p_game_dir.utf8().get_data());
            python.PyObject_SetAttrString(main, "_maszyna_game_dir", game_dir);
            python.Py_DecRef(game_dir);
            // the scripts open "./fonts/..." and "./textures/..." and import "from scripts", all
            // relative to the game directory, which the original is always started in
            python.PyRun_SimpleStringFlags(
                    "import os, sys\nos.chdir(_maszyna_game_dir)\nsys.path.insert(0, _maszyna_game_dir)\n", nullptr);
            // the base class of nearly every screen; a script that does not derive from it
            // still runs when it is missing
            python.run_file(main, p_game_dir.path_join("python/local/abstractscreenrenderer.py"));
        }

        while (true) {
            semaphore->wait();
            while (true) {
                Request request;
                {
                    MutexLock lock(**mutex);
                    if (exiting) {
                        if (loaded) {
                            for (const KeyValue<String, PyObject *> &renderer : renderers) {
                                if (renderer.value != nullptr) {
                                    python.Py_DecRef(renderer.value);
                                }
                            }
                            python.Py_Finalize();
                        }
                        return;
                    }
                    if (requests.is_empty()) {
                        break;
                    }
                    request = requests.front()->get();
                    requests.pop_front();
                }
                if (main == nullptr) {
                    continue; // no interpreter; the error has been printed once already
                }

                // python_taskqueue::fetch_renderer() - one instance per script, failures cached
                // as well so a broken script is not re-run on every update
                PyObject **cached = renderers.getptr(request.script_path);
                if (cached == nullptr) {
                    PyObject *renderer = nullptr;
                    if (python.run_file(main, request.script_path + ".py")) {
                        const String class_name = request.script_path.get_file();
                        PyObject *renderer_class = python.PyObject_GetAttrString(main, class_name.utf8().get_data());
                        if (renderer_class != nullptr) {
                            renderer = python.PyObject_CallFunction(
                                    renderer_class, const_cast<char *>("(s)"),
                                    (request.script_path.get_base_dir() + "/").utf8().get_data());
                            python.Py_DecRef(renderer_class);
                        }
                        if (renderer != nullptr) {
                            PyObject *result = python.PyObject_CallMethod(
                                    renderer, const_cast<char *>("manul_set_format"), const_cast<char *>("(s)"),
                                    "RGBA");
                            if (result != nullptr) {
                                python.Py_DecRef(result);
                            }
                        }
                        python.print_error();
                    }
                    cached = &renderers.insert(request.script_path, renderer)->value;
                }
                PyObject *renderer = *cached;
                if (renderer == nullptr) {
                    continue;
                }

                PyObject *state = python.PyDict_New();
                const Array keys = request.state.keys();
                for (int64_t i = 0; i < keys.size(); i++) {
                    PyObject *item = python.to_object(request.state[keys[i]]);
                    if (item != nullptr) {
                        python.PyDict_SetItemString(state, String(keys[i]).utf8().get_data(), item);
                        python.Py_DecRef(item);
                    }
                }
                PyObject *output = python.PyObject_CallMethod(
                        renderer, const_cast<char *>("render"), const_cast<char *>("(O)"), state);
                python.Py_DecRef(state);
                int width = 0;
                int height = 0;
                PackedByteArray pixels;
                if (output != nullptr) {
                    PyObject *width_object =
                            python.PyObject_CallMethod(renderer, const_cast<char *>("get_width"), nullptr);
                    PyObject *height_object =
                            python.PyObject_CallMethod(renderer, const_cast<char *>("get_height"), nullptr);
                    char *buffer = nullptr;
                    Py_ssize_t size = 0;
                    if (width_object != nullptr && height_object != nullptr &&
                        python.PyString_AsStringAndSize(output, &buffer, &size) == 0) {
                        width = static_cast<int>(python.PyInt_AsLong(width_object));
                        height = static_cast<int>(python.PyInt_AsLong(height_object));
                        const int64_t expected = static_cast<int64_t>(width) * height * BYTES_PER_PIXEL;
                        if (width > 0 && height > 0 && size >= expected) {
                            pixels.resize(expected);
                            memcpy(pixels.ptrw(), buffer, expected);
                        } else {
                            UtilityFunctions::push_error(
                                    "PythonScreenServer: ", request.script_path, " returned ", static_cast<int64_t>(size),
                                    " bytes for ", width, "x", height);
                        }
                    }
                    if (width_object != nullptr) {
                        python.Py_DecRef(width_object);
                    }
                    if (height_object != nullptr) {
                        python.Py_DecRef(height_object);
                    }
                    python.Py_DecRef(output);
                }
                python.print_error();

                PackedStringArray commands;
                PyObject *command_list = python.PyObject_CallMethod(renderer, const_cast<char *>("getCommands"), nullptr);
                if (command_list != nullptr) {
                    const Py_ssize_t count = python.PySequence_Size(command_list);
                    for (Py_ssize_t i = 0; i < count; i++) {
                        PyObject *command = python.PySequence_GetItem(command_list, i);
                        const char *text = command != nullptr ? python.PyString_AsString(command) : nullptr;
                        if (text != nullptr) {
                            commands.push_back(String::utf8(text));
                        }
                        if (command != nullptr) {
                            python.Py_DecRef(command);
                        }
                    }
                    python.Py_DecRef(command_list);
                }
                python.print_error();

                if (!pixels.is_empty() || !commands.is_empty()) {
                    callable_mp(this, &PythonScreenServer::_publish)
                            .call_deferred(request.screen, width, height, pixels, commands);
                }
            }
        }
    }
} // namespace godot
