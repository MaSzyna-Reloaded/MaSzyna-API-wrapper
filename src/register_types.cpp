#include "register_types.h"
#include "brakes/TrainBrake.hpp"
#include "brakes/TrainElectroPneumaticDynamicBrake.hpp"
#include "brakes/TrainSpringBrake.hpp"
#include "core/GameLog.hpp"
#include "core/ActionQueue.hpp"
#include "core/GenericTrainPart.hpp"
#include "core/TrainController.hpp"
#include "core/TrainPart.hpp"
#include "core/TrainSystem.hpp"
#include "doors/TrainDoors.hpp"
#include "engines/TrainDieselElectricEngine.hpp"
#include "engines/TrainDieselEngine.hpp"
#include "engines/TrainElectricEngine.hpp"
#include "engines/TrainElectricSeriesEngine.hpp"
#include "engines/TrainEngine.hpp"
#include "lighting/TrainLighting.hpp"
#include "models/MaterialManager.hpp"
#include "models/MaterialParser.hpp"
#include "models/e3d/E3DModel.hpp"
#include "models/e3d/E3DModelManager.hpp"
#include "models/e3d/E3DResourceFormatLoader.hpp"
#include "models/e3d/instance/E3DModelInstance.hpp"
#include "models/e3d/instance/E3DModelInstanceManager.hpp"
#include "models/e3d/instance/E3DModelNodesInstancer.hpp"
#include "load/TrainLoad.hpp"
#include "parsers/maszyna_parser.hpp"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "resources/material/MaszynaMaterial.hpp"
#include "settings/UserSettings.hpp"
#include "systems/TrainSecuritySystem.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/defs.hpp>

using namespace godot;

TrainSystem *train_system_singleton = nullptr;
UserSettings *user_settings_singleton = nullptr;
ActionQueue *action_queue_singleton = nullptr;
E3DModelInstanceManager *model_instance_manager_singleton = nullptr;

static bool is_doctool_mode() {
    const PackedStringArray args = OS::get_singleton()->get_cmdline_args();
    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == "--doctool") {
            return true;
        }
    }
    return false;
}
GameLog *game_log_singleton = nullptr;

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");;

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
//         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        // GDREGISTER_CLASS(ActionQueue);
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
        GDREGISTER_CLASS(TrainController);
        GDREGISTER_CLASS(TrainSecuritySystem);
        GDREGISTER_CLASS(TrainSystem);
        GDREGISTER_CLASS(TrainLighting)
        GDREGISTER_CLASS(GameLog);
        GDREGISTER_CLASS(WWListItem);
        GDREGISTER_CLASS(MotorParameter);
        GDREGISTER_CLASS(LightListItem)
        GDREGISTER_CLASS(TrainElectroPneumaticDynamicBrake)
        GDREGISTER_CLASS(TrainLoad)
        GDREGISTER_CLASS(LoadListItem)
        GDREGISTER_CLASS(LightListItem);
        GDREGISTER_CLASS(TrainElectroPneumaticDynamicBrake);

        // GDREGISTER_RUNTIME_CLASS(E3DModelInstance);
        // GDREGISTER_CLASS(E3DModelInstanceManager);
        // GDREGISTER_CLASS(E3DModelNodesInstancer);
        // GDREGISTER_CLASS(E3DModel);
        // GDREGISTER_CLASS(E3DModelManager);
        // GDREGISTER_CLASS(E3DResourceFormatLoader);
        // GDREGISTER_CLASS(E3DSubModel);
        train_system_singleton = memnew(TrainSystem);
        game_log_singleton = memnew(GameLog);
        Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);
        // GDREGISTER_RUNTIME_CLASS(E3DModelInstance);
        // GDREGISTER_CLASS(E3DModelInstanceManager);
        // GDREGISTER_CLASS(E3DModelNodesInstancer);
        // GDREGISTER_CLASS(E3DModel);
        // GDREGISTER_CLASS(E3DModelManager);
        // GDREGISTER_CLASS(E3DResourceFormatLoader);
        // GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(MaterialParser);
        GDREGISTER_CLASS(MaterialManager);
        GDREGISTER_CLASS(MaszynaMaterial);
        // GDREGISTER_CLASS(UserSettings);

        if (!is_doctool_mode()) {
            train_system_singleton = memnew(TrainSystem);
            // log_system_singleton = memnew(LogSystem);
            // user_settings_singleton = memnew(UserSettings);
            // action_queue_singleton = memnew(ActionQueue);
            // model_instance_manager_singleton = memnew(E3DModelInstanceManager);
            Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);
            // Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);
            Engine::get_singleton()->register_singleton("LogSystem", log_system_singleton);
            // Engine::get_singleton()->register_singleton("ActionQueue", action_queue_singleton);
            // Engine::get_singleton()->register_singleton("E3DModelInstanceManager", model_instance_manager_singleton);

            // Engine::get_singleton()->register_singleton("MaterialManager", memnew(MaterialManager));
            // MaterialManager::init();
        }
    }
}

void uninitialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("De-initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (train_system_singleton != nullptr) {
        Engine::get_singleton()->unregister_singleton("TrainSystem");
        Engine::get_singleton()->unregister_singleton("GameLog");
        // memdelete(train_system_singleton);
        // train_system_singleton = nullptr;
    }
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        // Unregister and free singletons we created at initialization.
        if (const Engine* _singleton = Engine::get_singleton(); _singleton != nullptr) {
            if (_singleton->has_singleton("TrainSystem")) {
                Engine::get_singleton()->unregister_singleton("TrainSystem");
            }
            // if (_singleton->has_singleton("UserSettings")) {
            //     Engine::get_singleton()->unregister_singleton("UserSettings");
            // }
            if (_singleton->has_singleton("LogSystem")) {
                Engine::get_singleton()->unregister_singleton("LogSystem");
            }
            // if (_singleton->has_singleton("ActionQueue")) {
            //     Engine::get_singleton()->unregister_singleton("ActionQueue");
            // }
            // if (_singleton->has_singleton("E3DModelInstanceManager")) {
            //     Engine::get_singleton()->unregister_singleton("E3DModelInstanceManager");
            // }
        }

        // Explicitly delete instances to avoid leaks during doctool/editor shutdown.
        if (train_system_singleton) {
            memdelete(train_system_singleton);
            train_system_singleton = nullptr;
        }
        // if (const Engine* _singleton = Engine::get_singleton(); _singleton != nullptr) {
        //     if (_singleton->has_singleton("MaterialManager")) {
        //         MaterialManager::cleanup();
        //         memdelete(_singleton->get_singleton("MaterialManager"));
        //         Engine::get_singleton()->unregister_singleton("MaterialManager");
        //     }
        // }
        // if (user_settings_singleton) {
        //     memdelete(user_settings_singleton);
        //     user_settings_singleton = nullptr;
        // }
        if (log_system_singleton) {
            memdelete(log_system_singleton);
            log_system_singleton = nullptr;
        }
        // if (action_queue_singleton) {
        //     memdelete(action_queue_singleton);
        //     action_queue_singleton = nullptr;
        // }
        // if (model_instance_manager_singleton) {
        //     memdelete(model_instance_manager_singleton);
        //     model_instance_manager_singleton = nullptr;
        // }
        // if (user_settings_singleton) {
        //     memdelete(user_settings_singleton);
        //     user_settings_singleton = nullptr;
        // }
        // if (log_system_singleton) {
        //     memdelete(log_system_singleton);
        //     log_system_singleton = nullptr;
        // }
        // if (action_queue_singleton) {
        //     memdelete(action_queue_singleton);
        //     action_queue_singleton = nullptr;
        // }
        // if (model_instance_manager_singleton) {
        //     memdelete(model_instance_manager_singleton);
        //     model_instance_manager_singleton = nullptr;
        // }
    }

}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT libmaszyna_library_init(
            const GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library,
            GDExtensionInitialization *r_initialization) {
        const GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_libmaszyna_module);
        init_obj.register_terminator(uninitialize_libmaszyna_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
