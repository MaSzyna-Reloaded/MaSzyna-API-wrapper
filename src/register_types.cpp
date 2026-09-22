#include "brakes/VehicleBrake.hpp"
#include "brakes/VehicleElectroPneumaticDynamicBrake.hpp"
#include "brakes/MoverVehicleSpringBrake.hpp"
#include "brakes/VehicleSpringBrake.hpp"
#include "buffers/VehicleBuffCoupl.hpp"
#include "controllers/MoverVehicleUniversalController.hpp"
#include "controllers/VehicleUniversalController.hpp"
#include "core/GameLog.hpp"
#include "core/GenericVehicleComponent.hpp"
#include "core/RailVehicle3D.hpp"
#include "core/ResourceCache.hpp"
#include "core/VehicleController.hpp"
#include "core/VehicleComponent.hpp"
#include "core/TrainSystem.hpp"
#include "core/MaszynaRuntime.hpp"
#include "core/UserSettings.hpp"
#include "doors/VehicleDoors.hpp"
#include "e3d/E3DModel.hpp"
#include "e3d/E3DModelLightDefinition.hpp"
#include "e3d/E3DModelSmokeSourceDefinition.hpp"
#include "e3d/E3DRenderingServer.hpp"
#include "e3d/E3DSubModel.hpp"
#include "engines/VehicleDieselElectricEngine.hpp"
#include "engines/VehicleDieselEngine.hpp"
#include "engines/VehicleElectricEngine.hpp"
#include "engines/VehicleElectricInductionEngine.hpp"
#include "engines/VehicleElectricSeriesEngine.hpp"
#include "engines/VehicleEngine.hpp"
#include "heating/MoverVehicleHeating.hpp"
#include "heating/VehicleHeating.hpp"
#include "lighting/VehicleLighting.hpp"
#include "load/MoverVehicleLoad.hpp"
#include "load/VehicleLoad.hpp"
#include "loaders/E3DResourceFormatLoader.hpp"
#include "loaders/OggVorbisFormatLoader.hpp"
#include "parsers/e3d_parser.hpp"
#include "parsers/maszyna_parser.hpp"
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
#include "speed_control/MoverVehicleSpeedControl.hpp"
#include "speed_control/VehicleSpeedControl.hpp"
#include "switches/MoverVehicleSwitches.hpp"
#include "switches/VehicleSwitches.hpp"
#include "systems/MoverVehicleAIHints.hpp"
#include "systems/VehicleAIHints.hpp"
#include "systems/MoverVehicleHorns.hpp"
#include "systems/VehicleHorns.hpp"
#include "systems/VehicleSecuritySystem.hpp"
#include "wheels/MoverVehicleWheels.hpp"
#include "wheels/VehicleWheels.hpp"
#include "wipers/MoverVehicleWipers.hpp"
#include "wipers/VehicleWipers.hpp"
#include "physics/BaseVehiclePhysicsServer.hpp"
#include "physics/MaszynaMoverPhysicsServer.hpp"
#include "physics/RailVehicleServer.hpp"
#include "tracks/SpatialIndex.hpp"
#include "tracks/TrackEndpointRef.hpp"
#include "tracks/TrackManager.hpp"
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
MaszynaRuntime *maszyna_runtime_singleton = nullptr;
E3DRenderingServer *e3d_rendering_server_singleton = nullptr;
TrackManager *track_manager_singleton = nullptr;
MaszynaMoverPhysicsServer *mover_physics_server_singleton = nullptr;
RailVehicleServer *rail_vehicle_server_singleton = nullptr;
SceneryStreamingServer *scenery_streaming_server_singleton = nullptr;
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
        GDREGISTER_CLASS(ResourceCache);
        GDREGISTER_CLASS(E3DSubModel);
        GDREGISTER_CLASS(E3DModel);
        GDREGISTER_CLASS(E3DParser);
        GDREGISTER_CLASS(E3DModelLightDefinition);
        GDREGISTER_CLASS(E3DModelSmokeSourceDefinition);
        GDREGISTER_CLASS(E3DRenderingServer);
        GDREGISTER_CLASS(E3DResourceFormatLoader);
        GDREGISTER_ABSTRACT_CLASS(BaseVehiclePhysicsServer);
        GDREGISTER_CLASS(MaszynaMoverPhysicsServer);
        GDREGISTER_CLASS(RailVehicleServer);
        GDREGISTER_CLASS(SpatialIndex);
        GDREGISTER_CLASS(TrackEndpointRef);
        GDREGISTER_CLASS(TrackBranchNeighbors);
        GDREGISTER_CLASS(TrackManager);
        GDREGISTER_CLASS(MaszynaParser);
        GDREGISTER_CLASS(MaszynaTrianglesImporter);
        GDREGISTER_CLASS(SceneryLoadingTaskQueue);
        GDREGISTER_CLASS(SceneryStreamingServer);
        GDREGISTER_CLASS(SceneryTrianglesBuilder);
        GDREGISTER_CLASS(OggVorbisFormatLoader);
        GDREGISTER_ABSTRACT_CLASS(VehicleComponent);
        GDREGISTER_CLASS(GenericVehicleComponent);
        GDREGISTER_CLASS(VehicleBrake);
        GDREGISTER_ABSTRACT_CLASS(VehicleSpringBrake);
        GDREGISTER_CLASS(MoverVehicleSpringBrake);
        GDREGISTER_CLASS(VehicleDoors);
        GDREGISTER_ABSTRACT_CLASS(VehicleEngine);
        GDREGISTER_CLASS(VehicleDieselEngine);
        GDREGISTER_CLASS(VehicleDieselElectricEngine);
        GDREGISTER_ABSTRACT_CLASS(VehicleElectricEngine);
        GDREGISTER_CLASS(VehicleElectricSeriesEngine);
        GDREGISTER_CLASS(VehicleElectricInductionEngine);
        GDREGISTER_CLASS(VehicleController);
        GDREGISTER_CLASS(RailVehicle3D);
        GDREGISTER_ABSTRACT_CLASS(VehicleHeating);
        GDREGISTER_CLASS(MoverVehicleHeating);
        GDREGISTER_ABSTRACT_CLASS(VehicleWheels);
        GDREGISTER_CLASS(MoverVehicleWheels);
        GDREGISTER_CLASS(VehicleSecuritySystem);
        GDREGISTER_ABSTRACT_CLASS(VehicleHorns);
        GDREGISTER_CLASS(MoverVehicleHorns);
        GDREGISTER_ABSTRACT_CLASS(VehicleAIHints);
        GDREGISTER_CLASS(MoverVehicleAIHints);
        GDREGISTER_CLASS(TrainSystem);
        GDREGISTER_CLASS(VehicleLighting)
        GDREGISTER_CLASS(GameLog);
        GDREGISTER_CLASS(WWListItem);
        GDREGISTER_CLASS(MotorParameter);
        GDREGISTER_CLASS(LightListItem)
        GDREGISTER_CLASS(VehicleElectroPneumaticDynamicBrake)
        GDREGISTER_ABSTRACT_CLASS(VehicleLoad)
        GDREGISTER_CLASS(MoverVehicleLoad)
        GDREGISTER_CLASS(LoadListItem)
        GDREGISTER_CLASS(VehicleBuffCoupl)
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
        train_system_singleton = memnew(TrainSystem);
        game_log_singleton = memnew(GameLog);
        e3d_parser_singleton = memnew(E3DParser);
        scenery_streaming_server_singleton = memnew(SceneryStreamingServer);
        e3d_rendering_server_singleton = memnew(E3DRenderingServer);
        track_manager_singleton = memnew(TrackManager);
        mover_physics_server_singleton = memnew(MaszynaMoverPhysicsServer);
        rail_vehicle_server_singleton = memnew(RailVehicleServer);

        Engine::get_singleton()->register_singleton("UserSettings", user_settings_singleton);                      // 1
        Engine::get_singleton()->register_singleton("E3DParser", e3d_parser_singleton);                            // 2
        Engine::get_singleton()->register_singleton("GameLog", game_log_singleton);                                // 3
        Engine::get_singleton()->register_singleton("TrainSystem", train_system_singleton);                        // 4
        Engine::get_singleton()->register_singleton("SceneryStreamingServer", scenery_streaming_server_singleton); // 5
        Engine::get_singleton()->register_singleton("E3DRenderingServer", e3d_rendering_server_singleton);         // 6
        Engine::get_singleton()->register_singleton("MaszynaRuntime", maszyna_runtime_singleton);                  // 7
        Engine::get_singleton()->register_singleton("TrackManager", track_manager_singleton);                      // 8
        Engine::get_singleton()->register_singleton(
                "MaszynaMoverPhysicsServer", mover_physics_server_singleton); // 9
        Engine::get_singleton()->register_singleton("RailVehicleServer", rail_vehicle_server_singleton); // 10

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

    if (Engine::get_singleton()->has_singleton("RailVehicleServer")) {
        Engine::get_singleton()->unregister_singleton("RailVehicleServer"); // 10
    }

    if (Engine::get_singleton()->has_singleton("MaszynaMoverPhysicsServer")) {
        Engine::get_singleton()->unregister_singleton("MaszynaMoverPhysicsServer"); // 9
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

    if (rail_vehicle_server_singleton != nullptr) { // 10
        memdelete(rail_vehicle_server_singleton);
        rail_vehicle_server_singleton = nullptr;
    }

    if (mover_physics_server_singleton != nullptr) { // 9
        memdelete(mover_physics_server_singleton);
        mover_physics_server_singleton = nullptr;
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
