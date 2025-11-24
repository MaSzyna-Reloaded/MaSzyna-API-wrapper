#include "register_types.h"
#include "brakes/TrainBrake.hpp"
#include "brakes/TrainElectroPneumaticDynamicBrake.hpp"
#include "brakes/TrainSpringBrake.hpp"
#include "core/GameLog.hpp"
#include "core/GenericTrainPart.hpp"
#include "core/TrainController.hpp"
#include "core/TrainNode.hpp"
#include "core/TrainPart.hpp"
#include "core/TrainPartNode.hpp"
#include "core/TrainSystem.hpp"
#include "doors/TrainDoors.hpp"
#include "engines/TrainDieselElectricEngine.hpp"
#include "engines/TrainDieselEngine.hpp"
#include "engines/TrainElectricEngine.hpp"
#include "engines/TrainElectricSeriesEngine.hpp"
#include "engines/TrainEngine.hpp"
#include "godot-ecs/CapabilityRegistry.hpp"
#include "godot-ecs/Component.hpp"
#include "godot-ecs/Entity.hpp"
#include "godot-ecs/EntityRegistry.hpp"
#include "godot-ecs/EntityNode.hpp"
#include "lighting/TrainLighting.hpp"
#include "load/TrainLoad.hpp"
#include "parsers/maszyna_parser.hpp"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "systems/TrainSecuritySystem.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

TrainSystem *train_system_singleton = nullptr;
GameLog *game_log_singleton = nullptr;

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
        GDREGISTER_CLASS(CapabilityRegistry);
        GDREGISTER_CLASS(Component);
        GDREGISTER_CLASS(Entity);
        GDREGISTER_CLASS(EntityNode);
        GDREGISTER_CLASS(EntityRegistry);
        GDREGISTER_CLASS(TrainNode);
        GDREGISTER_CLASS(TrainPartNode);
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

        train_system_singleton = memnew(TrainSystem);
        game_log_singleton = memnew(GameLog);
        Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);
    }
}

void uninitialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("De-initializing libmaszyna module on level " + String::num(p_level) + "...");;

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (train_system_singleton != nullptr) {
        Engine::get_singleton()->unregister_singleton("TrainSystem");
        Engine::get_singleton()->unregister_singleton("GameLog");
        // memdelete(train_system_singleton);
        // train_system_singleton = nullptr;
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
