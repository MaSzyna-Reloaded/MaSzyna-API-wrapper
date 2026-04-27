#include "brakes/TrainBrake.hpp"
#include "brakes/TrainElectroPneumaticDynamicBrake.hpp"
#include "brakes/TrainSpringBrake.hpp"
#include "buffers/TrainBuffCoupl.hpp"
#include "core/GameLog.hpp"
#include "core/GenericTrainPart.hpp"
#include "core/ResourceCache.hpp"
#include "core/TrainController.hpp"
#include "core/TrainPart.hpp"
#include "core/TrainSystem.hpp"
#include "core/UserSettings.hpp"
#include "doors/TrainDoors.hpp"
#include "engines/TrainDieselElectricEngine.hpp"
#include "engines/TrainDieselEngine.hpp"
#include "engines/TrainElectricEngine.hpp"
#include "engines/TrainElectricSeriesEngine.hpp"
#include "engines/TrainEngine.hpp"
#include "lighting/TrainLighting.hpp"
#include "load/TrainLoad.hpp"
#include "models/MaterialManager.hpp"
#include "models/MaterialParser.hpp"
#include "models/TrackRenderingServer.hpp"
#include "parsers/maszyna_parser.hpp"
#include "register_types.h"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "resources/material/MaszynaMaterial.hpp"
#include "systems/TrainSecuritySystem.hpp"
#include "wheels/TrainWheels.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <models/e3d/E3DResourceFormatLoader.hpp>
#include <models/e3d/instance/E3DModelInstanceManager.hpp>
#include <models/e3d/instance/E3DNodesInstancer.hpp>

using namespace godot;

UserSettings *user_settings_singleton = nullptr;
TrainSystem *train_system_singleton = nullptr;
GameLog *game_log_singleton = nullptr;
MaterialManager *material_manager_singleton = nullptr;
TrackRenderingServer *track_rendering_server_singleton = nullptr;
Ref<E3DResourceFormatLoader> e3d_loader_singleton;
E3DModelManager *e3d_model_manager_singleton = nullptr;
E3DNodesInstancer *e3d_nodes_instancer_singleton = nullptr;
E3DModelInstanceManager *e3d_model_instance_manager_singleton = nullptr;

static bool is_doctool_mode() {
    const PackedStringArray args = OS::get_singleton()->get_cmdline_args();
    for (const auto &arg: args) {
        if (arg == "--doctool") {
            return true;
        }
    }
    return false;
}

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        //         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(UserSettings);
        GDREGISTER_CLASS(MaszynaParser);
        GDREGISTER_ABSTRACT_CLASS(TrainPart);
        GDREGISTER_CLASS(GenericTrainPart);
        GDREGISTER_CLASS(TrainBrake);
        GDREGISTER_CLASS(TrainSpringBrake);
        GDREGISTER_CLASS(TrainDoors);
        GDREGISTER_ABSTRACT_CLASS(TrainEngine);
        GDREGISTER_CLASS(TrainDieselEngine);
        GDREGISTER_CLASS(TrainDieselElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(TrainElectricEngine);
        GDREGISTER_CLASS(TrainElectricSeriesEngine);
        GDREGISTER_CLASS(TrackRenderingServer);
        GDREGISTER_CLASS(TrainController);
        GDREGISTER_CLASS(TrainWheels);
        GDREGISTER_CLASS(TrainSecuritySystem);
        GDREGISTER_CLASS(TrainSystem);
        GDREGISTER_CLASS(TrainLighting)
        GDREGISTER_CLASS(GameLog);
        GDREGISTER_CLASS(WWListItem);
        GDREGISTER_CLASS(MotorParameter);
        GDREGISTER_CLASS(LightListItem)
        GDREGISTER_CLASS(MaterialParser);
        GDREGISTER_CLASS(MaterialManager);
        GDREGISTER_CLASS(MaszynaMaterial);
        GDREGISTER_CLASS(TrainElectroPneumaticDynamicBrake)
        GDREGISTER_CLASS(TrainLoad)
        GDREGISTER_CLASS(LoadListItem)
        GDREGISTER_CLASS(TrainBuffCoupl)

        // E3D
        GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(E3DModel);
        GDREGISTER_CLASS(E3DParser);

        GDREGISTER_CLASS(E3DModelInstance);
        GDREGISTER_CLASS(E3DModelManager);
        GDREGISTER_CLASS(E3DNodesInstancer);
        GDREGISTER_CLASS(E3DModelInstanceManager);
        GDREGISTER_CLASS(E3DResourceFormatLoader);

        // Core
        GDREGISTER_CLASS(ResourceCache)

        if (!is_doctool_mode()) {
            user_settings_singleton = memnew(UserSettings);
            train_system_singleton = memnew(TrainSystem);
            game_log_singleton = memnew(GameLog);
            material_manager_singleton = memnew(MaterialManager);
            track_rendering_server_singleton = memnew(TrackRenderingServer);
            e3d_model_manager_singleton = memnew(E3DModelManager);
            e3d_model_instance_manager_singleton = memnew(E3DModelInstanceManager);
            e3d_nodes_instancer_singleton = memnew(E3DNodesInstancer);
            e3d_loader_singleton.instantiate();

            Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);
            Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);
            Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);
            Engine::get_singleton()->register_singleton("MaterialManager", material_manager_singleton);
            Engine::get_singleton()->register_singleton("TrackRenderingServer", track_rendering_server_singleton);
            Engine::get_singleton()->register_singleton("E3DModelManager", e3d_model_manager_singleton);
            Engine::get_singleton()->register_singleton("E3DNodesInstancer", e3d_nodes_instancer_singleton);
            Engine::get_singleton()->register_singleton(
                    "E3DModelInstanceManager", e3d_model_instance_manager_singleton);
            ResourceLoader::get_singleton()->add_resource_format_loader(e3d_loader_singleton);
        }
    }
}

void uninitialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("De-initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (Engine *singleton = Engine::get_singleton(); !is_doctool_mode() && singleton != nullptr) {
        if (singleton->has_singleton("E3DModelInstanceManager")) {
            singleton->unregister_singleton("E3DModelInstanceManager");
        }
        if (singleton->has_singleton("E3DNodesInstancer")) {
            singleton->unregister_singleton("E3DNodesInstancer");
        }
        if (singleton->has_singleton("E3DModelManager")) {
            singleton->unregister_singleton("E3DModelManager");
        }
        if (singleton->has_singleton("TrackRenderingServer")) {
            singleton->unregister_singleton("TrackRenderingServer");
        }
        if (singleton->has_singleton("MaterialManager")) {
            singleton->unregister_singleton("MaterialManager");
        }
        if (singleton->has_singleton("GameLog")) {
            singleton->unregister_singleton("GameLog");
        }
        if (singleton->has_singleton("TrainSystem")) {
            singleton->unregister_singleton("TrainSystem");
        }
        if (singleton->has_singleton("UserSettings")) {
            singleton->unregister_singleton("UserSettings");
        }
    }

    if (e3d_loader_singleton.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(e3d_loader_singleton);
        e3d_loader_singleton.unref();
    }

    if (e3d_nodes_instancer_singleton != nullptr) {
        memdelete(e3d_nodes_instancer_singleton);
        e3d_nodes_instancer_singleton = nullptr;
    }

    if (e3d_model_instance_manager_singleton != nullptr) {
        memdelete(e3d_model_instance_manager_singleton);
        e3d_model_instance_manager_singleton = nullptr;
    }

    if (e3d_model_manager_singleton != nullptr) {
        memdelete(e3d_model_manager_singleton);
        e3d_model_manager_singleton = nullptr;
    }

    if (track_rendering_server_singleton != nullptr) {
        memdelete(track_rendering_server_singleton);
        track_rendering_server_singleton = nullptr;
    }

    if (material_manager_singleton != nullptr) {
        memdelete(material_manager_singleton);
        material_manager_singleton = nullptr;
    }

    if (game_log_singleton != nullptr) {
        memdelete(game_log_singleton);
        game_log_singleton = nullptr;
    }

    if (train_system_singleton != nullptr) {
        memdelete(train_system_singleton);
        train_system_singleton = nullptr;
    }

    if (user_settings_singleton != nullptr) {
        memdelete(user_settings_singleton);
        user_settings_singleton = nullptr;
    }
}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT libmaszyna_library_init(
            const GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library,
            GDExtensionInitialization *p_initialization) {
        const GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, p_initialization);

        init_obj.register_initializer(initialize_libmaszyna_module);
        init_obj.register_terminator(uninitialize_libmaszyna_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
