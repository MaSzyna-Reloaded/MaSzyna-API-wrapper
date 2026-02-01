#include "brakes/TrainBrake.hpp"
#include "brakes/TrainElectroPneumaticDynamicBrake.hpp"
#include "brakes/TrainSpringBrake.hpp"
#include "core/GameLog.hpp"
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
#include "load/TrainLoad.hpp"
#include "models/MaterialManager.hpp"
#include "models/MaterialParser.hpp"
#include "parsers/maszyna_parser.hpp"
#include "register_types.h"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "resources/material/MaszynaMaterial.hpp"
#include "systems/TrainSecuritySystem.hpp"
#include <core/ActionQueue.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <models/e3d/E3DResourceFormatLoader.hpp>
#include <models/e3d/instance/E3DModelInstanceManager.hpp>
#include <models/e3d/instance/E3DNodesInstancer.hpp>

using namespace godot;

TrainSystem *train_system_singleton = nullptr;
GameLog *game_log_singleton = nullptr;
MaterialManager* material_manager_singleton = nullptr;
ActionQueue *action_queue_singleton = nullptr;
E3DResourceFormatLoader *e3d_loader_singleton = nullptr;
E3DModelInstanceManager *e3d_model_instance_manager_singleton = nullptr;

static bool is_doctool_mode() {
    const PackedStringArray args = OS::get_singleton()->get_cmdline_args();
    for (const auto & arg : args) {
        if (arg == "--doctool") {
            return true;
        }
    }
    return false;
}

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");;

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
//         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
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
        GDREGISTER_CLASS(TrainElectroPneumaticDynamicBrake);
        GDREGISTER_CLASS(TrainLoad);
        GDREGISTER_CLASS(LoadListItem);
        GDREGISTER_CLASS(MaterialParser);
        GDREGISTER_CLASS(MaterialManager);
        GDREGISTER_CLASS(MaszynaMaterial);

        // E3D
        GDREGISTER_CLASS(E3DModel);
        GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(E3DModelManager);
        GDREGISTER_CLASS(E3DModelInstanceManager);
        GDREGISTER_CLASS(E3DModelInstance);
        GDREGISTER_CLASS(E3DNodesInstancer);
        GDREGISTER_CLASS(E3DParser);
        GDREGISTER_CLASS(E3DResourceFormatLoader);

        // Core
        GDREGISTER_CLASS(ActionQueue)

        if (!is_doctool_mode()) {
            train_system_singleton = memnew(TrainSystem);
            game_log_singleton = memnew(GameLog);
            material_manager_singleton = memnew(MaterialManager);
            action_queue_singleton = memnew(ActionQueue);
            e3d_model_instance_manager_singleton = memnew(E3DModelInstanceManager);
            e3d_loader_singleton = memnew(E3DResourceFormatLoader);

            Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);
            Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);
            Engine::get_singleton()->register_singleton("MaterialManager", material_manager_singleton);
            Engine::get_singleton()->register_singleton("ActionQueue", action_queue_singleton);
            Engine::get_singleton()->register_singleton("E3DModelInstanceManager", e3d_model_instance_manager_singleton);
            ResourceLoader::get_singleton()->add_resource_format_loader(e3d_loader_singleton);
        }
    }
}

void uninitialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("De-initializing libmaszyna module on level " + String::num(p_level) + "...");;

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    if (Engine *_singleton = Engine::get_singleton(); !is_doctool_mode() && _singleton != nullptr) {
        if (train_system_singleton != nullptr) {
            _singleton->unregister_singleton("TrainSystem");
        }

        if (game_log_singleton != nullptr) {
            _singleton->unregister_singleton("GameLog");
        }

        if (material_manager_singleton != nullptr) {
            _singleton->unregister_singleton("MaterialManager");
        }

        if (_singleton->has_singleton("ActionQueue")) {
            _singleton->unregister_singleton("ActionQueue");
        }

        if (_singleton->has_singleton("E3DModelInstanceManager")) {
            _singleton->unregister_singleton("E3DModelInstanceManager");
        }
    }

    if (train_system_singleton != nullptr) {
        memdelete(train_system_singleton);
        train_system_singleton = nullptr;
    }

    if (game_log_singleton != nullptr) {
        memdelete(game_log_singleton);
        game_log_singleton = nullptr;
    }

    if (material_manager_singleton != nullptr) {
        memdelete(material_manager_singleton);
        material_manager_singleton = nullptr;
    }

    if (action_queue_singleton != nullptr) {
        memdelete(action_queue_singleton);
        action_queue_singleton = nullptr;
    }

    if (e3d_model_instance_manager_singleton != nullptr) {
        memdelete(e3d_model_instance_manager_singleton);
        e3d_model_instance_manager_singleton = nullptr;
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
