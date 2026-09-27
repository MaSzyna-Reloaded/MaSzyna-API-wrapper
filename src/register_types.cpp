#include "legacy/vehicles/MoverRailVehicleBrake.hpp"
#include "legacy/vehicles/MoverRailVehicleElectroPneumaticDynamicBrake.hpp"
#include "legacy/vehicles/MoverRailVehicleSpringBrake.hpp"
#include "vehicles/rail/RailVehicleBrake.hpp"
#include "vehicles/rail/RailVehicleElectroPneumaticDynamicBrake.hpp"
#include "vehicles/rail/RailVehicleSpringBrake.hpp"
#include "legacy/vehicles/MoverRailVehicleBuffCoupl.hpp"
#include "vehicles/rail/RailVehicleBuffCoupl.hpp"
#include "cabin/Cabin3D.hpp"
#include "cabin/CabinHUDMouseSystem.hpp"
#include "legacy/vehicles/MoverRailVehicleMasterController.hpp"
#include "legacy/vehicles/MoverRailVehicleUniversalController.hpp"
#include "vehicles/rail/RailVehicleMasterController.hpp"
#include "vehicles/rail/RailVehicleUniversalController.hpp"
#include "logging/GameLog.hpp"
#include "vehicles/base/GenericVehicleComponent.hpp"
#include "vehicles/base/GenericVehicleComponentNode.hpp"
#include "simulation/SimulationServer.hpp"
#include "driver/DriverDelegate.hpp"
#include "driver/DriverSystem.hpp"
#include "utils/MaszynaTranslationServer.hpp"
#include "legacy/vehicles/MoverRailVehicleController.hpp"
#include "vehicles/rail/RailVehicle3D.hpp"
#include "cache/ResourceCache.hpp"
#include "utils/UserSettings.hpp"
#include "vehicles/rail/RailVehicleComponent.hpp"
#include "vehicles/base/VehicleComponent.hpp"
#include "vehicles/base/VehicleComponentModel.hpp"
#include "vehicles/base/VehicleComponentType.hpp"
#include "vehicles/base/VehicleController.hpp"
#include "vehicles/base/VehicleModel.hpp"
#include "vehicles/base/VehiclePhysicsNode.hpp"
#include "legacy/vehicles/MoverRailVehicleDoors.hpp"
#include "vehicles/rail/RailVehicleDoors.hpp"
#include "legacy/e3d/E3DModel.hpp"
#include "legacy/e3d/E3DModelLightDefinition.hpp"
#include "legacy/e3d/E3DModelSmokeSourceDefinition.hpp"
#include "legacy/e3d/E3DRenderingServer.hpp"
#include "legacy/e3d/E3DSubModel.hpp"
#include "legacy/vehicles/MoverRailVehicleDieselElectricEngine.hpp"
#include "legacy/vehicles/MoverRailVehicleDieselEngine.hpp"
#include "legacy/vehicles/MoverRailVehicleElectricInductionEngine.hpp"
#include "legacy/vehicles/MoverRailVehicleElectricSeriesEngine.hpp"
#include "vehicles/rail/RailVehicleDieselElectricEngine.hpp"
#include "vehicles/rail/RailVehicleDieselEngine.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"
#include "vehicles/rail/RailVehicleElectricInductionEngine.hpp"
#include "vehicles/rail/RailVehicleElectricSeriesEngine.hpp"
#include "vehicles/rail/RailVehicleEngine.hpp"
#include "legacy/vehicles/MoverRailVehicleHeating.hpp"
#include "vehicles/rail/RailVehicleHeating.hpp"
#include "legacy/vehicles/MoverRailVehicleLighting.hpp"
#include "vehicles/rail/RailVehicleLighting.hpp"
#include "legacy/vehicles/MoverRailVehicleLoad.hpp"
#include "vehicles/rail/RailVehicleLoad.hpp"
#include "legacy/e3d/E3DResourceFormatLoader.hpp"
#include "loaders/OggVorbisFormatLoader.hpp"
#include "legacy/e3d/e3d_parser.hpp"
#include "legacy/parsers/maszyna_parser.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include "simulation/SimulationClock.hpp"
#include "vehicles/rail/RailVehicleNeighbour.hpp"
#include "legacy/vehicles/MoverRailVehicleRadio.hpp"
#include "vehicles/rail/RailVehicleRadio.hpp"
#include "register_types.h"
#include "vehicles/rail/RailVehicleBrakePressureTableItem.hpp"
#include "vehicles/rail/RailVehicleCompressorListItem.hpp"
#include "vehicles/rail/RailVehicleUniversalControllerListItem.hpp"
#include "vehicles/base/VehicleCurvePointItem.hpp"
#include "vehicles/rail/RailVehicleMotorParameter.hpp"
#include "vehicles/rail/RailVehicleRelayListItem.hpp"
#include "vehicles/rail/RailVehicleThrottlePositionItem.hpp"
#include "vehicles/rail/RailVehicleWWListItem.hpp"
#include "vehicles/rail/RailVehicleLightListItem.hpp"
#include "vehicles/rail/RailVehicleLoadListItem.hpp"
#include "vehicles/rail/RailVehicleDimmerListItem.hpp"
#include "vehicles/rail/RailVehicleWiperListItem.hpp"
#include "legacy/scenery/MaszynaTrianglesImporter.hpp"
#include "scenery/SceneryLoadingTaskQueue.hpp"
#include "scenery/SceneryStreamingServer.hpp"
#include "scenery/SceneryTrianglesBuilder.hpp"
#include "legacy/cabin/PythonScreenServer.hpp"
#include "legacy/semaphores/MaszynaLegacySemaphoreDelegate.hpp"
#include "legacy/semaphores/MaszynaLegacySemaphoreKindFactory.hpp"
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
#include "scenario/MaszynaLegacyVehicleCommandAction.hpp"
#include "scenario/MaszynaLegacyVoltageAction.hpp"
#include "scenario/ScenarioEventAction.hpp"
#include "scenario/ScenarioEventCondition.hpp"
#include "scenario/ScenarioEventServer.hpp"
#include "scenario/Timetable.hpp"
#include "scenario/TimetableEntry.hpp"
#include "legacy/vehicles/MoverRailVehicleSpeedControl.hpp"
#include "vehicles/rail/RailVehicleSpeedControl.hpp"
#include "legacy/vehicles/MoverRailVehicleSwitches.hpp"
#include "vehicles/rail/RailVehicleSwitches.hpp"
#include "legacy/vehicles/MoverRailVehicleAIHints.hpp"
#include "legacy/vehicles/MoverRailVehicleHorns.hpp"
#include "legacy/vehicles/MoverRailVehicleSecuritySystem.hpp"
#include "vehicles/rail/RailVehicleAIHints.hpp"
#include "vehicles/rail/RailVehicleHorns.hpp"
#include "vehicles/rail/RailVehicleSecuritySystem.hpp"
#include "tracks/SpatialIndex.hpp"
#include "tracks/TrackEndpointRef.hpp"
#include "tracks/TrackServer.hpp"
#include "traction/TractionServer.hpp"
#include "legacy/vehicles/MoverRailVehicleWheels.hpp"
#include "vehicles/rail/RailVehicleWheels.hpp"
#include "legacy/vehicles/MoverRailVehicleWipers.hpp"
#include "vehicles/rail/RailVehicleWipers.hpp"
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
SimulationServer *simulation_server_singleton = nullptr;
E3DRenderingServer *e3d_rendering_server_singleton = nullptr;
TrackServer *track_server_singleton = nullptr;
RailVehicleServer *rail_vehicle_server_singleton = nullptr;
TractionServer *traction_server_singleton = nullptr;
SceneryStreamingServer *scenery_streaming_server_singleton = nullptr;
PythonScreenServer *python_screen_server_singleton = nullptr;
MaszynaTranslationServer *maszyna_translation_server_singleton = nullptr;
CabinHUDMouseSystem *cabin_hud_mouse_system_singleton = nullptr;
SemaphoreServer *semaphore_server_singleton = nullptr;
ScenarioEventServer *scenario_event_server_singleton = nullptr;
DriverSystem *driver_system_singleton = nullptr;
Ref<E3DResourceFormatLoader> e3d_resource_format_loader;
Ref<OggVorbisFormatLoader> ogg_vorbis_format_loader;

void initialize_libmaszyna_module(const ModuleInitializationLevel p_level) {
    UtilityFunctions::print("Initializing libmaszyna module on level " + String::num(p_level) + "...");

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        //         GDREGISTER_CLASS(DieselEngineMasterControllerPowerItemEditor);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(UserSettings);
        GDREGISTER_CLASS(SimulationServer);
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
        GDREGISTER_INTERNAL_CLASS(SimulationClock);
        GDREGISTER_CLASS(RailVehicleNeighbour);
        GDREGISTER_CLASS(TractionServer);
        GDREGISTER_CLASS(SpatialIndex);
        GDREGISTER_CLASS(TrackEndpointRef);
        GDREGISTER_CLASS(TrackRouteSegment);
        GDREGISTER_CLASS(TrackBranchNeighbors);
        GDREGISTER_CLASS(TrackServer);
        GDREGISTER_CLASS(SemaphoreServer);
        GDREGISTER_CLASS(SemaphoreAspect);
        GDREGISTER_CLASS(SemaphoreKind);
        GDREGISTER_ABSTRACT_CLASS(MaszynaLegacySemaphoreKindFactory);
        GDREGISTER_VIRTUAL_CLASS(SemaphoreSystemDelegate);
        GDREGISTER_CLASS(MaszynaLegacySemaphoreDelegate);
        GDREGISTER_CLASS(SemaphoreNode);
        GDREGISTER_CLASS(SemaphoreSystemNode);
        GDREGISTER_CLASS(ScenarioEventServer);
        GDREGISTER_CLASS(DriverSystem);
        GDREGISTER_VIRTUAL_CLASS(DriverDelegate);
        GDREGISTER_VIRTUAL_CLASS(ScenarioEventAction);
        GDREGISTER_VIRTUAL_CLASS(ScenarioEventCondition);
        GDREGISTER_CLASS(MaszynaLegacyMemoryAction);
        GDREGISTER_CLASS(MaszynaLegacyMultipleAction);
        GDREGISTER_CLASS(MaszynaLegacyLightsAction);
        GDREGISTER_CLASS(MaszynaLegacySwitchAction);
        GDREGISTER_CLASS(MaszynaLegacyVoltageAction);
        GDREGISTER_CLASS(MaszynaLegacyTrackVelocityAction);
        GDREGISTER_CLASS(MaszynaLegacyAnimationAction);
        GDREGISTER_CLASS(MaszynaLegacyVehicleCommandAction);
        GDREGISTER_CLASS(TimetableEntry);
        GDREGISTER_CLASS(Timetable);
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
        GDREGISTER_ABSTRACT_CLASS(RailVehicleComponent);
        GDREGISTER_CLASS(GenericVehicleComponent);
        GDREGISTER_CLASS(GenericVehicleComponentNode);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleBrake);
        GDREGISTER_CLASS(MoverRailVehicleBrake);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleSpringBrake);
        GDREGISTER_CLASS(MoverRailVehicleSpringBrake);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleDoors);
        GDREGISTER_CLASS(MoverRailVehicleDoors);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleEngine);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleDieselEngine);
        GDREGISTER_CLASS(MoverRailVehicleDieselEngine);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleDieselElectricEngine);
        GDREGISTER_CLASS(MoverRailVehicleDieselElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleElectricSeriesEngine);
        GDREGISTER_CLASS(MoverRailVehicleElectricSeriesEngine);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleElectricInductionEngine);
        GDREGISTER_CLASS(MoverRailVehicleElectricInductionEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleController);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleController);
        GDREGISTER_CLASS(MoverRailVehicleController);
        // the vehicles are simulated on the vendored Mover
        VehiclePhysicsNode::set_controller_implementation(MoverRailVehicleController::get_class_static());
        GDREGISTER_CLASS(Cabin3D);
        GDREGISTER_CLASS(CabinHUDMouseSystem);
        GDREGISTER_CLASS(RailVehicle3D);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleHeating);
        GDREGISTER_CLASS(MoverRailVehicleHeating);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleRadio);
        GDREGISTER_CLASS(MoverRailVehicleRadio);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleWheels);
        GDREGISTER_CLASS(MoverRailVehicleWheels);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleSecuritySystem);
        GDREGISTER_CLASS(MoverRailVehicleSecuritySystem);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleHorns);
        GDREGISTER_CLASS(MoverRailVehicleHorns);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleAIHints);
        GDREGISTER_CLASS(MoverRailVehicleAIHints);
        GDREGISTER_ABSTRACT_CLASS(RailVehicleLighting)
        GDREGISTER_CLASS(MoverRailVehicleLighting)
        GDREGISTER_CLASS(GameLog);
        GDREGISTER_CLASS(RailVehicleWWListItem);
        GDREGISTER_CLASS(RailVehicleMotorParameter);
        GDREGISTER_CLASS(RailVehicleLightListItem)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleElectroPneumaticDynamicBrake)
        GDREGISTER_CLASS(MoverRailVehicleElectroPneumaticDynamicBrake)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleLoad)
        GDREGISTER_CLASS(MoverRailVehicleLoad)
        GDREGISTER_CLASS(RailVehicleLoadListItem)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleBuffCoupl)
        GDREGISTER_CLASS(MoverRailVehicleBuffCoupl)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleSpeedControl)
        GDREGISTER_CLASS(MoverRailVehicleSpeedControl)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleUniversalController)
        GDREGISTER_CLASS(MoverRailVehicleUniversalController)
        GDREGISTER_CLASS(RailVehicleUniversalControllerListItem)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleMasterController)
        GDREGISTER_CLASS(MoverRailVehicleMasterController)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleWipers)
        GDREGISTER_CLASS(MoverRailVehicleWipers)
        GDREGISTER_CLASS(RailVehicleWiperListItem)
        GDREGISTER_ABSTRACT_CLASS(RailVehicleSwitches)
        GDREGISTER_CLASS(MoverRailVehicleSwitches)
        GDREGISTER_CLASS(RailVehicleDimmerListItem)
        GDREGISTER_CLASS(RailVehicleBrakePressureTableItem)
        GDREGISTER_CLASS(RailVehicleCompressorListItem)
        GDREGISTER_CLASS(RailVehicleRelayListItem)
        GDREGISTER_CLASS(VehicleCurvePointItem)
        GDREGISTER_CLASS(RailVehicleThrottlePositionItem)

        user_settings_singleton = memnew(UserSettings);
        simulation_server_singleton = memnew(SimulationServer);
        game_log_singleton = memnew(GameLog);
        e3d_parser_singleton = memnew(E3DParser);
        scenery_streaming_server_singleton = memnew(SceneryStreamingServer);
        e3d_rendering_server_singleton = memnew(E3DRenderingServer);
        track_server_singleton = memnew(TrackServer);
        traction_server_singleton = memnew(TractionServer);
        python_screen_server_singleton = memnew(PythonScreenServer);
        cabin_hud_mouse_system_singleton = memnew(CabinHUDMouseSystem);

        Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);                      // 1
        Engine::get_singleton()->register_singleton("E3DParser", e3d_parser_singleton);                            // 2
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);                                // 3
        Engine::get_singleton()->register_singleton("SceneryStreamingServer", scenery_streaming_server_singleton); // 5
        Engine::get_singleton()->register_singleton("E3DRenderingServer", e3d_rendering_server_singleton);         // 6
        Engine::get_singleton()->register_singleton("SimulationServer", simulation_server_singleton);                  // 7
        Engine::get_singleton()->register_singleton("TrackServer", track_server_singleton);                      // 8
        // after SimulationServer is registered: the constructor follows its pause
        rail_vehicle_server_singleton = memnew(RailVehicleServer);
        Engine::get_singleton()->register_singleton("RailVehicleServer", rail_vehicle_server_singleton);     // 10
        Engine::get_singleton()->register_singleton("TractionServer", traction_server_singleton); // 11
        Engine::get_singleton()->register_singleton("PythonScreenServer", python_screen_server_singleton);   // 12
        // after UserSettings is registered: the constructor reads the game directory from it
        maszyna_translation_server_singleton = memnew(MaszynaTranslationServer);
        Engine::get_singleton()->register_singleton(
                "MaszynaTranslationServer", maszyna_translation_server_singleton);                            // 13
        Engine::get_singleton()->register_singleton("CabinHUDMouseSystem", cabin_hud_mouse_system_singleton); // 14
        // after E3DRenderingServer is registered: the constructor follows its freed instances
        semaphore_server_singleton = memnew(SemaphoreServer);
        Engine::get_singleton()->register_singleton("SemaphoreServer", semaphore_server_singleton); // 15
        // after SimulationServer is registered: the constructor follows its pause and speed
        scenario_event_server_singleton = memnew(ScenarioEventServer);
        Engine::get_singleton()->register_singleton("ScenarioEventServer", scenario_event_server_singleton); // 16
        // after RailVehicleServer is registered: the constructor follows its freed vehicles
        driver_system_singleton = memnew(DriverSystem);
        Engine::get_singleton()->register_singleton("DriverSystem", driver_system_singleton); // 17

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

    if (Engine::get_singleton()->has_singleton("DriverSystem")) {
        Engine::get_singleton()->unregister_singleton("DriverSystem"); // 17
    }
    if (driver_system_singleton != nullptr) {
        memdelete(driver_system_singleton);
        driver_system_singleton = nullptr;
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
        if (Engine::get_singleton()->has_singleton("TractionServer")) {
            Engine::get_singleton()->unregister_singleton("TractionServer"); // 11
        }
        if (traction_server_singleton != nullptr) {
            memdelete(traction_server_singleton);
            traction_server_singleton = nullptr;
        }
        Engine::get_singleton()->unregister_singleton("RailVehicleServer"); // 10
    }

    if (Engine::get_singleton()->has_singleton("TrackServer")) {
        Engine::get_singleton()->unregister_singleton("TrackServer"); // 8
    }

    if (Engine::get_singleton()->has_singleton("SimulationServer")) {
        Engine::get_singleton()->unregister_singleton("SimulationServer"); // 7
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

    if (track_server_singleton != nullptr) { // 8
        memdelete(track_server_singleton);
        track_server_singleton = nullptr;
    }

    if (simulation_server_singleton != nullptr) { // 7
        memdelete(simulation_server_singleton);
        simulation_server_singleton = nullptr;
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
