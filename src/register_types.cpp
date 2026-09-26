#include "brakes/MoverVehicleBrake.hpp"
#include "brakes/MoverVehicleElectroPneumaticDynamicBrake.hpp"
#include "brakes/MoverVehicleSpringBrake.hpp"
#include "brakes/VehicleBrake.hpp"
#include "brakes/VehicleElectroPneumaticDynamicBrake.hpp"
#include "brakes/VehicleSpringBrake.hpp"
#include "buffers/MoverVehicleBuffCoupl.hpp"
#include "buffers/VehicleBuffCoupl.hpp"
#include "cabin/Cabin3D.hpp"
#include "cabin/CabinHUDMouseSystem.hpp"
#include "controllers/MoverVehicleUniversalController.hpp"
#include "controllers/VehicleUniversalController.hpp"
#include "core/GameLog.hpp"
#include "core/GenericVehicleComponent.hpp"
#include "core/GenericVehicleComponentNode.hpp"
#include "core/MaszynaRuntime.hpp"
#include "core/MaszynaTranslationServer.hpp"
#include "core/MoverVehicleController.hpp"
#include "core/RailVehicle3D.hpp"
#include "core/ResourceCache.hpp"
#include "core/UserSettings.hpp"
#include "core/VehicleComponent.hpp"
#include "core/VehicleComponentModel.hpp"
#include "core/VehicleComponentType.hpp"
#include "core/VehicleController.hpp"
#include "core/VehicleModel.hpp"
#include "core/VehiclePhysicsNode.hpp"
#include "doors/MoverVehicleDoors.hpp"
#include "doors/VehicleDoors.hpp"
#include "e3d/E3DModel.hpp"
#include "e3d/E3DModelLightDefinition.hpp"
#include "e3d/E3DModelSmokeSourceDefinition.hpp"
#include "e3d/E3DRenderingServer.hpp"
#include "e3d/E3DSubModel.hpp"
#include "engines/MoverVehicleDieselElectricEngine.hpp"
#include "engines/MoverVehicleDieselEngine.hpp"
#include "engines/MoverVehicleElectricInductionEngine.hpp"
#include "engines/MoverVehicleElectricSeriesEngine.hpp"
#include "engines/VehicleDieselElectricEngine.hpp"
#include "engines/VehicleDieselEngine.hpp"
#include "engines/VehicleElectricEngine.hpp"
#include "engines/VehicleElectricInductionEngine.hpp"
#include "engines/VehicleElectricSeriesEngine.hpp"
#include "engines/VehicleEngine.hpp"
#include "heating/MoverVehicleHeating.hpp"
#include "heating/VehicleHeating.hpp"
#include "lighting/MoverVehicleLighting.hpp"
#include "lighting/VehicleLighting.hpp"
#include "load/MoverVehicleLoad.hpp"
#include "load/VehicleLoad.hpp"
#include "loaders/E3DResourceFormatLoader.hpp"
#include "loaders/OggVorbisFormatLoader.hpp"
#include "parsers/e3d_parser.hpp"
#include "parsers/maszyna_parser.hpp"
#include "physics/RailVehicleServer.hpp"
#include "physics/RailVehicleStepper.hpp"
#include "radio/MoverVehicleRadio.hpp"
#include "radio/VehicleRadio.hpp"
#include "register_types.h"
#include "resources/brakes/BrakePressureTableItem.hpp"
#include "resources/brakes/CompressorListItem.hpp"
#include "resources/controllers/UniversalControllerListItem.hpp"
#include "resources/engines/CurvePointItem.hpp"
#include "resources/engines/MotorParameter.hpp"
#include "resources/engines/RelayListItem.hpp"
#include "resources/engines/ThrottlePositionItem.hpp"
#include "resources/engines/WWListItem.hpp"
#include "resources/lighting/LightListItem.hpp"
#include "resources/load/LoadListItem.hpp"
#include "resources/switches/DimmerListItem.hpp"
#include "resources/wipers/WiperListItem.hpp"
#include "scenery/MaszynaTrianglesImporter.hpp"
#include "scenery/SceneryLoadingTaskQueue.hpp"
#include "scenery/SceneryStreamingServer.hpp"
#include "scenery/SceneryTrianglesBuilder.hpp"
#include "scripting/PythonScreenServer.hpp"
#include "semaphores/MaszynaLegacySemaphoreDelegate.hpp"
#include "semaphores/MaszynaLegacySemaphoreKindFactory.hpp"
#include "semaphores/SemaphoreAspect.hpp"
#include "semaphores/SemaphoreKind.hpp"
#include "semaphores/SemaphoreNode.hpp"
#include "semaphores/SemaphoreServer.hpp"
#include "semaphores/SemaphoreSystemDelegate.hpp"
#include "semaphores/SemaphoreSystemNode.hpp"
#include "scenario/MaszynaLegacyAnimationAction.hpp"
#include "scenario/MaszynaLegacyEventCondition.hpp"
#include "scenario/MaszynaLegacyLightsAction.hpp"
#include "scenario/MaszynaLegacyMemoryAction.hpp"
#include "scenario/MaszynaLegacyMultipleAction.hpp"
#include "scenario/MaszynaLegacySwitchAction.hpp"
#include "scenario/MaszynaLegacyTrackVelocityAction.hpp"
#include "scenario/MaszynaLegacyVoltageAction.hpp"
#include "scenario/ScenarioEventAction.hpp"
#include "scenario/ScenarioEventCondition.hpp"
#include "scenario/ScenarioEventServer.hpp"
#include "speed_control/MoverVehicleSpeedControl.hpp"
#include "speed_control/VehicleSpeedControl.hpp"
#include "switches/MoverVehicleSwitches.hpp"
#include "switches/VehicleSwitches.hpp"
#include "systems/MoverVehicleAIHints.hpp"
#include "systems/MoverVehicleHorns.hpp"
#include "systems/MoverVehicleSecuritySystem.hpp"
#include "systems/VehicleAIHints.hpp"
#include "systems/VehicleHorns.hpp"
#include "systems/VehicleSecuritySystem.hpp"
#include "tracks/SpatialIndex.hpp"
#include "tracks/TrackEndpointRef.hpp"
#include "tracks/TrackManager.hpp"
#include "traction/TractionPowerServer.hpp"
#include "wheels/MoverVehicleWheels.hpp"
#include "wheels/VehicleWheels.hpp"
#include "wipers/MoverVehicleWipers.hpp"
#include "wipers/VehicleWipers.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

GameLog *game_log_singleton = nullptr;
E3DParser *e3d_parser_singleton = nullptr;
UserSettings *user_settings_singleton = nullptr;
MaszynaRuntime *maszyna_runtime_singleton = nullptr;
E3DRenderingServer *e3d_rendering_server_singleton = nullptr;
TrackManager *track_manager_singleton = nullptr;
RailVehicleServer *rail_vehicle_server_singleton = nullptr;
TractionPowerServer *traction_power_server_singleton = nullptr;
SceneryStreamingServer *scenery_streaming_server_singleton = nullptr;
PythonScreenServer *python_screen_server_singleton = nullptr;
MaszynaTranslationServer *maszyna_translation_server_singleton = nullptr;
CabinHUDMouseSystem *cabin_hud_mouse_system_singleton = nullptr;
SemaphoreServer *semaphore_server_singleton = nullptr;
ScenarioEventServer *scenario_event_server_singleton = nullptr;
Ref<E3DResourceFormatLoader> e3d_resource_format_loader;
Ref<OggVorbisFormatLoader> ogg_vorbis_format_loader;

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        //         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(UserSettings);
        GDREGISTER_CLASS(MaszynaRuntime);
        GDREGISTER_CLASS(MaszynaTranslationServer);
        GDREGISTER_CLASS(ResourceCache);
        GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(E3DModel);
        GDREGISTER_CLASS(E3DParser);
        GDREGISTER_CLASS(E3DModelLightDefinition);
        GDREGISTER_CLASS(E3DModelSmokeSourceDefinition);
        GDREGISTER_CLASS(E3DRenderingServer);
        GDREGISTER_CLASS(E3DResourceFormatLoader);
        GDREGISTER_CLASS(RailVehicleServer);
        GDREGISTER_INTERNAL_CLASS(RailVehicleStepper);
        GDREGISTER_CLASS(TractionPowerServer);
        GDREGISTER_CLASS(SpatialIndex);
        GDREGISTER_CLASS(TrackEndpointRef);
        GDREGISTER_CLASS(TrackBranchNeighbors);
        GDREGISTER_CLASS(TrackManager);
        GDREGISTER_CLASS(SemaphoreServer);
        GDREGISTER_CLASS(SemaphoreAspect);
        GDREGISTER_CLASS(SemaphoreKind);
        GDREGISTER_ABSTRACT_CLASS(MaszynaLegacySemaphoreKindFactory);
        GDREGISTER_VIRTUAL_CLASS(SemaphoreSystemDelegate);
        GDREGISTER_CLASS(MaszynaLegacySemaphoreDelegate);
        GDREGISTER_CLASS(SemaphoreNode);
        GDREGISTER_CLASS(SemaphoreSystemNode);
        GDREGISTER_CLASS(ScenarioEventServer);
        GDREGISTER_VIRTUAL_CLASS(ScenarioEventAction);
        GDREGISTER_VIRTUAL_CLASS(ScenarioEventCondition);
        GDREGISTER_CLASS(MaszynaLegacyMemoryAction);
        GDREGISTER_CLASS(MaszynaLegacyMultipleAction);
        GDREGISTER_CLASS(MaszynaLegacyLightsAction);
        GDREGISTER_CLASS(MaszynaLegacySwitchAction);
        GDREGISTER_CLASS(MaszynaLegacyVoltageAction);
        GDREGISTER_CLASS(MaszynaLegacyTrackVelocityAction);
        GDREGISTER_CLASS(MaszynaLegacyAnimationAction);
        GDREGISTER_CLASS(MaszynaLegacyEventCondition);
        GDREGISTER_CLASS(MaszynaParser);
        GDREGISTER_CLASS(MaszynaTrianglesImporter);
        GDREGISTER_CLASS(SceneryLoadingTaskQueue);
        GDREGISTER_CLASS(SceneryStreamingServer);
        GDREGISTER_CLASS(PythonScreenServer);
        GDREGISTER_CLASS(SceneryTrianglesBuilder);
        GDREGISTER_CLASS(OggVorbisFormatLoader);
        GDREGISTER_ABSTRACT_CLASS(VehicleComponentType);
        GDREGISTER_CLASS(VehicleComponentModel);
        GDREGISTER_CLASS(VehicleModel);
        GDREGISTER_CLASS(VehiclePhysicsNode);
        GDREGISTER_ABSTRACT_CLASS(VehicleComponent);
        GDREGISTER_CLASS(GenericVehicleComponent);
        GDREGISTER_CLASS(GenericVehicleComponentNode);
        GDREGISTER_ABSTRACT_CLASS(VehicleBrake);
        GDREGISTER_CLASS(MoverVehicleBrake);
        GDREGISTER_ABSTRACT_CLASS(VehicleSpringBrake);
        GDREGISTER_CLASS(MoverVehicleSpringBrake);
        GDREGISTER_ABSTRACT_CLASS(VehicleDoors);
        GDREGISTER_CLASS(MoverVehicleDoors);
        GDREGISTER_ABSTRACT_CLASS(VehicleEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleDieselEngine);
        GDREGISTER_CLASS(MoverVehicleDieselEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleDieselElectricEngine);
        GDREGISTER_CLASS(MoverVehicleDieselElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleElectricSeriesEngine);
        GDREGISTER_CLASS(MoverVehicleElectricSeriesEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleElectricInductionEngine);
        GDREGISTER_CLASS(MoverVehicleElectricInductionEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleController);
        GDREGISTER_CLASS(MoverVehicleController);
        // the vehicles are simulated on the vendored Mover
        VehiclePhysicsNode::set_controller_implementation(MoverVehicleController::get_class_static());
        GDREGISTER_CLASS(Cabin3D);
        GDREGISTER_CLASS(CabinHUDMouseSystem);
        GDREGISTER_CLASS(RailVehicle3D);
        GDREGISTER_ABSTRACT_CLASS(VehicleHeating);
        GDREGISTER_CLASS(MoverVehicleHeating);
        GDREGISTER_ABSTRACT_CLASS(VehicleRadio);
        GDREGISTER_CLASS(MoverVehicleRadio);
        GDREGISTER_ABSTRACT_CLASS(VehicleWheels);
        GDREGISTER_CLASS(MoverVehicleWheels);
        GDREGISTER_ABSTRACT_CLASS(VehicleSecuritySystem);
        GDREGISTER_CLASS(MoverVehicleSecuritySystem);
        GDREGISTER_ABSTRACT_CLASS(VehicleHorns);
        GDREGISTER_CLASS(MoverVehicleHorns);
        GDREGISTER_ABSTRACT_CLASS(VehicleAIHints);
        GDREGISTER_CLASS(MoverVehicleAIHints);
        GDREGISTER_ABSTRACT_CLASS(VehicleLighting)
        GDREGISTER_CLASS(MoverVehicleLighting)
        GDREGISTER_CLASS(GameLog);
        GDREGISTER_CLASS(WWListItem);
        GDREGISTER_CLASS(MotorParameter);
        GDREGISTER_CLASS(LightListItem)
        GDREGISTER_ABSTRACT_CLASS(VehicleElectroPneumaticDynamicBrake)
        GDREGISTER_CLASS(MoverVehicleElectroPneumaticDynamicBrake)
        GDREGISTER_ABSTRACT_CLASS(VehicleLoad)
        GDREGISTER_CLASS(MoverVehicleLoad)
        GDREGISTER_CLASS(LoadListItem)
        GDREGISTER_ABSTRACT_CLASS(VehicleBuffCoupl)
        GDREGISTER_CLASS(MoverVehicleBuffCoupl)
        GDREGISTER_ABSTRACT_CLASS(VehicleSpeedControl)
        GDREGISTER_CLASS(MoverVehicleSpeedControl)
        GDREGISTER_ABSTRACT_CLASS(VehicleUniversalController)
        GDREGISTER_CLASS(MoverVehicleUniversalController)
        GDREGISTER_CLASS(UniversalControllerListItem)
        GDREGISTER_ABSTRACT_CLASS(VehicleWipers)
        GDREGISTER_CLASS(MoverVehicleWipers)
        GDREGISTER_CLASS(WiperListItem)
        GDREGISTER_ABSTRACT_CLASS(VehicleSwitches)
        GDREGISTER_CLASS(MoverVehicleSwitches)
        GDREGISTER_CLASS(DimmerListItem)
        GDREGISTER_CLASS(BrakePressureTableItem)
        GDREGISTER_CLASS(CompressorListItem)
        GDREGISTER_CLASS(RelayListItem)
        GDREGISTER_CLASS(CurvePointItem)
        GDREGISTER_CLASS(ThrottlePositionItem)

        user_settings_singleton = memnew(UserSettings);
        maszyna_runtime_singleton = memnew(MaszynaRuntime);
        game_log_singleton = memnew(GameLog);
        e3d_parser_singleton = memnew(E3DParser);
        scenery_streaming_server_singleton = memnew(SceneryStreamingServer);
        e3d_rendering_server_singleton = memnew(E3DRenderingServer);
        track_manager_singleton = memnew(TrackManager);
        traction_power_server_singleton = memnew(TractionPowerServer);
        python_screen_server_singleton = memnew(PythonScreenServer);
        cabin_hud_mouse_system_singleton = memnew(CabinHUDMouseSystem);

        Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);                      // 1
        Engine::get_singleton()->register_singleton("E3DParser", e3d_parser_singleton);                            // 2
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);                                // 3
        Engine::get_singleton()->register_singleton("SceneryStreamingServer", scenery_streaming_server_singleton); // 5
        Engine::get_singleton()->register_singleton("E3DRenderingServer", e3d_rendering_server_singleton);         // 6
        Engine::get_singleton()->register_singleton("MaszynaRuntime", maszyna_runtime_singleton);                  // 7
        Engine::get_singleton()->register_singleton("TrackManager", track_manager_singleton);                      // 8
        // after MaszynaRuntime is registered: the constructor follows its pause
        rail_vehicle_server_singleton = memnew(RailVehicleServer);
        Engine::get_singleton()->register_singleton("RailVehicleServer", rail_vehicle_server_singleton);     // 10
        Engine::get_singleton()->register_singleton("TractionPowerServer", traction_power_server_singleton); // 11
        Engine::get_singleton()->register_singleton("PythonScreenServer", python_screen_server_singleton);   // 12
        // after UserSettings is registered: the constructor reads the game directory from it
        maszyna_translation_server_singleton = memnew(MaszynaTranslationServer);
        Engine::get_singleton()->register_singleton(
                "MaszynaTranslationServer", maszyna_translation_server_singleton);                            // 13
        Engine::get_singleton()->register_singleton("CabinHUDMouseSystem", cabin_hud_mouse_system_singleton); // 14
        // after E3DRenderingServer is registered: the constructor follows its freed instances
        semaphore_server_singleton = memnew(SemaphoreServer);
        Engine::get_singleton()->register_singleton("SemaphoreServer", semaphore_server_singleton); // 15
        // after MaszynaRuntime is registered: the constructor follows its pause and speed
        scenario_event_server_singleton = memnew(ScenarioEventServer);
        Engine::get_singleton()->register_singleton("ScenarioEventServer", scenario_event_server_singleton); // 16

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

    if (ogg_vorbis_format_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(ogg_vorbis_format_loader);
        ogg_vorbis_format_loader.unref();
    }

    if (e3d_resource_format_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(e3d_resource_format_loader);
        e3d_resource_format_loader.unref();
    }

    if (Engine::get_singleton()->has_singleton("ScenarioEventServer")) {
        Engine::get_singleton()->unregister_singleton("ScenarioEventServer"); // 16
    }
    if (scenario_event_server_singleton != nullptr) {
        memdelete(scenario_event_server_singleton);
        scenario_event_server_singleton = nullptr;
    }

    if (Engine::get_singleton()->has_singleton("SemaphoreServer")) {
        Engine::get_singleton()->unregister_singleton("SemaphoreServer"); // 15
    }
    if (semaphore_server_singleton != nullptr) {
        memdelete(semaphore_server_singleton);
        semaphore_server_singleton = nullptr;
    }

    if (Engine::get_singleton()->has_singleton("CabinHUDMouseSystem")) {
        Engine::get_singleton()->unregister_singleton("CabinHUDMouseSystem"); // 14
    }
    if (cabin_hud_mouse_system_singleton != nullptr) {
        memdelete(cabin_hud_mouse_system_singleton);
        cabin_hud_mouse_system_singleton = nullptr;
    }

    if (Engine::get_singleton()->has_singleton("MaszynaTranslationServer")) {
        Engine::get_singleton()->unregister_singleton("MaszynaTranslationServer"); // 13
    }
    if (maszyna_translation_server_singleton != nullptr) {
        memdelete(maszyna_translation_server_singleton);
        maszyna_translation_server_singleton = nullptr;
    }

    if (Engine::get_singleton()->has_singleton("PythonScreenServer")) {
        Engine::get_singleton()->unregister_singleton("PythonScreenServer"); // 12
    }
    if (python_screen_server_singleton != nullptr) {
        memdelete(python_screen_server_singleton);
        python_screen_server_singleton = nullptr;
    }

    if (Engine::get_singleton()->has_singleton("RailVehicleServer")) {
        if (Engine::get_singleton()->has_singleton("TractionPowerServer")) {
            Engine::get_singleton()->unregister_singleton("TractionPowerServer"); // 11
        }
        if (traction_power_server_singleton != nullptr) {
            memdelete(traction_power_server_singleton);
            traction_power_server_singleton = nullptr;
        }
        Engine::get_singleton()->unregister_singleton("RailVehicleServer"); // 10
    }

    if (Engine::get_singleton()->has_singleton("TrackManager")) {
        Engine::get_singleton()->unregister_singleton("TrackManager"); // 8
    }

    if (Engine::get_singleton()->has_singleton("MaszynaRuntime")) {
        Engine::get_singleton()->unregister_singleton("MaszynaRuntime"); // 7
    }

    if (Engine::get_singleton()->has_singleton("E3DRenderingServer")) {
        Engine::get_singleton()->unregister_singleton("E3DRenderingServer"); // 6
    }

    if (Engine::get_singleton()->has_singleton("SceneryStreamingServer")) {
        Engine::get_singleton()->unregister_singleton("SceneryStreamingServer"); // 5
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

    if (rail_vehicle_server_singleton != nullptr) { // 10
        memdelete(rail_vehicle_server_singleton);
        rail_vehicle_server_singleton = nullptr;
    }

    if (track_manager_singleton != nullptr) { // 8
        memdelete(track_manager_singleton);
        track_manager_singleton = nullptr;
    }

    if (maszyna_runtime_singleton != nullptr) { // 7
        memdelete(maszyna_runtime_singleton);
        maszyna_runtime_singleton = nullptr;
    }

    if (e3d_rendering_server_singleton != nullptr) { // 6
        memdelete(e3d_rendering_server_singleton);
        e3d_rendering_server_singleton = nullptr;
    }

    if (scenery_streaming_server_singleton != nullptr) { // 5
        memdelete(scenery_streaming_server_singleton);
        scenery_streaming_server_singleton = nullptr;
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
}
extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT libmaszyna_library_init(
            const GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library,
            GDExtensionInitialization *p_r_initialization) {
        const GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, p_r_initialization);

        init_obj.register_initializer(initialize_libmaszyna_module);
        init_obj.register_terminator(uninitialize_libmaszyna_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
