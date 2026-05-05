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
#include "e3d/E3DInstancer.hpp"
#include "e3d/E3DModel.hpp"
#include "e3d/E3DModelInstance.hpp"
#include "e3d/E3DModelManager.hpp"
#include "e3d/E3DModelTool.hpp"
#include "e3d/E3DNodesInstancer.hpp"
#include "e3d/E3DOptimizedInstancer.hpp"
#include "e3d/E3DResourceFormatLoader.hpp"
#include "e3d/E3DSubModel.hpp"
#include "engines/TrainDieselElectricEngine.hpp"
#include "engines/TrainDieselEngine.hpp"
#include "engines/TrainElectricEngine.hpp"
#include "engines/TrainElectricSeriesEngine.hpp"
#include "engines/TrainEngine.hpp"
#include "lighting/TrainLighting.hpp"
#include "load/TrainLoad.hpp"
#include "materials/MaszynaMaterial.hpp"
#include "materials/MaterialManager.hpp"
#include "materials/MaterialParser.hpp"
#include "parsers/e3d_parser.hpp"
#include "parsers/maszyna_parser.hpp"
#include "register_types.h"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "sound/OggVorbisFormatLoader.hpp"
#include "systems/TrainSecuritySystem.hpp"
#include "wheels/TrainWheels.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

TrainSystem *train_system_singleton = nullptr;
GameLog *game_log_singleton = nullptr;
E3DParser *e3d_parser_singleton = nullptr;
UserSettings *user_settings_singleton = nullptr;
MaterialManager *material_manager_singleton = nullptr;              // 5
E3DModelManager *e3d_model_manager_singleton = nullptr;             // 6
E3DModelTool *e3d_model_tool_singleton = nullptr;                   // 7
E3DNodesInstancer *e3d_nodes_instancer_singleton = nullptr;         // 8
E3DOptimizedInstancer *e3d_optimized_instancer_singleton = nullptr; // 9
Ref<E3DResourceFormatLoader> e3d_resource_format_loader;
Ref<OggVorbisFormatLoader> ogg_vorbis_format_loader;

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        //         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(UserSettings);
        GDREGISTER_CLASS(ResourceCache);
        GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(E3DModel);
        GDREGISTER_CLASS(E3DModelInstance);
        GDREGISTER_ABSTRACT_CLASS(E3DInstancer);
        GDREGISTER_CLASS(E3DNodesInstancer);
        GDREGISTER_CLASS(E3DOptimizedInstancer);
        GDREGISTER_CLASS(E3DModelManager);
        GDREGISTER_CLASS(E3DModelTool);
        GDREGISTER_CLASS(E3DResourceFormatLoader);
        GDREGISTER_CLASS(E3DParser);
        GDREGISTER_CLASS(MaszynaParser);
        GDREGISTER_CLASS(MaszynaMaterial);
        GDREGISTER_CLASS(MaterialParser);
        GDREGISTER_CLASS(MaterialManager);
        GDREGISTER_CLASS(OggVorbisFormatLoader);
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
        GDREGISTER_CLASS(TrainWheels);
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
        GDREGISTER_CLASS(TrainBuffCoupl)

        user_settings_singleton = memnew(UserSettings);
        train_system_singleton = memnew(TrainSystem);
        game_log_singleton = memnew(GameLog);
        e3d_parser_singleton = memnew(E3DParser);
        material_manager_singleton = memnew(MaterialManager);
        e3d_model_manager_singleton = memnew(E3DModelManager);
        e3d_model_tool_singleton = memnew(E3DModelTool);
        e3d_nodes_instancer_singleton = memnew(E3DNodesInstancer);
        e3d_optimized_instancer_singleton = memnew(E3DOptimizedInstancer);

        Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);                    // 1
        Engine::get_singleton()->register_singleton("E3DParser", e3d_parser_singleton);                          // 2
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);                              // 3
        Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);                      // 4
        Engine::get_singleton()->register_singleton("MaterialManager", material_manager_singleton);              // 5
        Engine::get_singleton()->register_singleton("E3DModelManager", e3d_model_manager_singleton);             // 6
        Engine::get_singleton()->register_singleton("E3DModelTool", e3d_model_tool_singleton);                   // 7
        Engine::get_singleton()->register_singleton("E3DNodesInstancer", e3d_nodes_instancer_singleton);         // 8
        Engine::get_singleton()->register_singleton("E3DOptimizedInstancer", e3d_optimized_instancer_singleton); // 9

        e3d_resource_format_loader.instantiate();
        ogg_vorbis_format_loader.instantiate();

        ResourceLoader::get_singleton()->add_resource_format_loader(e3d_resource_format_loader);
        ResourceLoader::get_singleton()->add_resource_format_loader(ogg_vorbis_format_loader);
    }
}

void uninitialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("De-initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (Engine::get_singleton()->has_singleton("E3DOptimizedInstancer")) {
        Engine::get_singleton()->unregister_singleton("E3DOptimizedInstancer"); // 9
    }

    if (Engine::get_singleton()->has_singleton("E3DNodesInstancer")) {
        Engine::get_singleton()->unregister_singleton("E3DNodesInstancer"); // 8
    }

    if (Engine::get_singleton()->has_singleton("E3DModelTool")) {
        Engine::get_singleton()->unregister_singleton("E3DModelTool"); // 7
    }

    if (Engine::get_singleton()->has_singleton("E3DModelManager")) {
        Engine::get_singleton()->unregister_singleton("E3DModelManager"); // 6
    }

    if (Engine::get_singleton()->has_singleton("MaterialManager")) {
        Engine::get_singleton()->unregister_singleton("MaterialManager"); // 5
    }

    if (Engine::get_singleton()->has_singleton("TrainSystem")) {
        Engine::get_singleton()->unregister_singleton("TrainSystem"); // 4
    }

    if (Engine::get_singleton()->has_singleton("GameLog")) {
        Engine::get_singleton()->unregister_singleton("GameLog"); // 3
    }

    if (Engine::get_singleton()->has_singleton("E3DParser")) {
        Engine::get_singleton()->unregister_singleton("E3DParser"); // 2
    }

    if (Engine::get_singleton()->has_singleton("UserSettings")) {
        Engine::get_singleton()->unregister_singleton("UserSettings"); // 1
    }

    if (ogg_vorbis_format_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(ogg_vorbis_format_loader);
    }

    if (e3d_resource_format_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(e3d_resource_format_loader);
    }

    if (e3d_optimized_instancer_singleton != nullptr) { // 9
        memdelete(e3d_optimized_instancer_singleton);
        e3d_optimized_instancer_singleton = nullptr;
    }

    if (e3d_nodes_instancer_singleton != nullptr) { // 8
        memdelete(e3d_nodes_instancer_singleton);
        e3d_nodes_instancer_singleton = nullptr;
    }

    if (e3d_model_tool_singleton != nullptr) { // 7
        memdelete(e3d_model_tool_singleton);
        e3d_model_tool_singleton = nullptr;
    }

    if (e3d_model_manager_singleton != nullptr) { // 6
        memdelete(e3d_model_manager_singleton);
        e3d_model_manager_singleton = nullptr;
    }

    if (material_manager_singleton != nullptr) { // 5
        memdelete(material_manager_singleton);
        material_manager_singleton = nullptr;
    }

    if (train_system_singleton != nullptr) { // 4
        memdelete(train_system_singleton);
        train_system_singleton = nullptr;
    }

    if (game_log_singleton != nullptr) { // 3
        memdelete(game_log_singleton);
        game_log_singleton = nullptr;
    }

    if (e3d_parser_singleton != nullptr) { // 2
        memdelete(e3d_parser_singleton);
        e3d_parser_singleton = nullptr;
    }

    if (user_settings_singleton != nullptr) { // 1
        memdelete(user_settings_singleton);
        user_settings_singleton = nullptr;
    }

    e3d_resource_format_loader.unref();
    ogg_vorbis_format_loader.unref();
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
