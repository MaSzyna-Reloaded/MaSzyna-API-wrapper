#include "../mover/MoverComponent.hpp"
#include "../mover/MoverTypes.hpp"
#include "MoverVehicleController.hpp"
#include "maszyna/utilities.h"
#include <cmath>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    std::unordered_map<const TMoverParameters *, MoverVehicleController *> MoverVehicleController::controllers_by_mover;

    void MoverVehicleController::_bind_methods() {}

    /* release() runs from VehicleController's NOTIFICATION_PREDELETE, which the editor skips; the
     * Mover must not outlive the vehicle either way. */
    MoverVehicleController::~MoverVehicleController() {
        if (mover != nullptr) {
            controllers_by_mover.erase(mover);
            delete mover;
            mover = nullptr;
        }
    }

    /* A Mover* component reaches this vehicle's Mover through here from now on. `dynamic_cast`
     * rather than `Object::cast_to`, because MoverComponent is not an Object - see its own
     * header for why that is the right tool and what it costs to get wrong. */
    void MoverVehicleController::_component_attached(VehicleComponent *p_component) {
        if (MoverComponent *mover_component = dynamic_cast<MoverComponent *>(p_component); mover_component != nullptr) {
            mover_component->set_mover_controller(this);
        }
    }

    void MoverVehicleController::_component_detached(VehicleComponent *p_component) {
        if (MoverComponent *mover_component = dynamic_cast<MoverComponent *>(p_component); mover_component != nullptr) {
            mover_component->set_mover_controller(nullptr);
        }
    }

    TMoverParameters *MoverVehicleController::get_mover() const {
        return mover;
    }

    // the end of the coupled vehicle facing this one (TCoupling::ConnectedNr), -1 when not coupled
    int MoverVehicleController::get_coupled_end(const int p_end) const {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return -1;
        }
        return mover->Couplers[p_end].ConnectedNr;
    }

    VehicleController *MoverVehicleController::get_coupled_controller(const int p_end) const {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return nullptr;
        }
        const auto it = controllers_by_mover.find(mover->Couplers[p_end].Connected);
        return it == controllers_by_mover.end() ? nullptr : it->second;
    }

    void MoverVehicleController::initialize_mover_state() {
        const bool driver_active = get_initial_velocity() != 0.0;

        mover->MainCtrlPos = mover->MainCtrlNoPowerPos();
        mover->LocalBrakePosA = 0.0;
        mover->BrakeCtrlPos = static_cast<int>(
                std::floor(mover->Handle->GetPos(driver_active && get_driver_type() != DRIVER_NOBODY ? bh_RP : bh_NP)));
        mover->BrakeLevelSet(mover->BrakeCtrlPos);
    }

    void MoverVehicleController::_initialize_simulation() {
        // the vehicle used to be named by its node; what identifies one now is its train id
        mover = new TMoverParameters(
                get_initial_velocity(), std::string(get_type_name().utf8().get_data()),
                std::string(get_train_id().utf8().get_data()),
                get_occupied_cab()); // the cab as TMoverParameters::CabActivisation counts it
        controllers_by_mover[mover] = this;

        apply_configuration();

        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(get_initial_velocity() != 0.0, 0); // FIXME: brakujace parametery

        /* CheckLocomotiveParameters() will reset some parameters, so the changes
         * must be applied second time */

        apply_configuration();
        initialize_mover_state();

        // Original engine: Load() (Mover.cpp:11692) calls ComputeConstans() once, after every
        // physical parameter (TotalMass, Dim, Cx, BearingType, NPoweredAxles, TrackW - all
        // already applied above by the two apply_configuration() passes) is settled -
        // it derives FrictConst1/FrictConst2s/FrictConst2d, the per-vehicle rolling/air-drag
        // resistance coefficients FrictionForce() (called every tick from ComputeTotalForce())
        // actually uses. Never called anywhere else in the original either (a single call at
        // load time is correct - the original itself never updates curve-dependent resistance
        // terms after that point). Without this, every one of this wrapper's vehicles ran with
        // zero rolling/air resistance: free acceleration to unrealistic speeds and near-zero
        // coasting deceleration, since FrictConst1/2s/2d all silently stayed at their
        // compiled-zero defaults.
        mover->ComputeConstans();

        // Original engine: the scenery's driver type picks the cab (DynObj.cpp:1812-1825).
        // FIXME: a vehicle without a driver stays in cab 0 there; here it still starts in cab 1.
        if (mover->CabOccupied == 0) {
            mover->CabOccupied = 1;
        }
        // only a driven vehicle gets its cab activated by the driver (Driver.cpp:2126); an unmanned
        // one stays inactive, so ComputeTotalForce() can switch its physics off
        if (get_driver_type() != DRIVER_NOBODY) {
            mover->CabActivisation();
        }
        /* What the scenery loaded the vehicle with. The backend takes the cargo's name and its
         * amount together and reads more than cargo out of them - `pantstate` is how a scenery
         * starts a locomotive with raised pantographs (Mover.cpp:7647). */
        if (!get_load_name().is_empty()) {
            mover->AssignLoad(std::string(get_load_name().utf8().ptr()), static_cast<float>(get_load_amount()));
        }

        /* switch_physics() raczej trzeba zostawic */
        mover->switch_physics(true);

        DEBUG("[MaSzyna::TMoverParameters] Mover initialized successfully");
        emit_signal(simulation_initialized_signal);
    }

    /* The base lets go of the components and the registration; the Mover goes last. */
    void MoverVehicleController::release() {
        VehicleController::release();
        if (mover == nullptr) {
            return;
        }
        controllers_by_mover.erase(mover);
        delete mover;
        mover = nullptr;
    }

    bool MoverVehicleController::is_simulation_ready() const {
        return mover != nullptr;
    }

    /* Whether the vehicle still has anything to integrate. A braked standing vehicle stays active
     * in the original too (Mover.cpp:4603). */
    bool MoverVehicleController::is_physics_active() const {
        return mover != nullptr && mover->PhysicActivation;
    }

    // Original engine: TDynamicObject::Move sets Loc = {-x, z, y} (DynObj.cpp:2334); dMoveLen collects the
    // movement of one simulation frame and is reset after it (ResetdMoveLen, DynObj.cpp:3473)
    void MoverVehicleController::update_location() {
        if (mover == nullptr) {
            return;
        }
        const Vector3 position = get_world_position();
        mover->Loc = {-position.x, position.z, position.y};
        mover->dMoveLen = 0.0;
    }

    // Original engine: TDynamicObject::update_neighbours() (DynObj.cpp:7135); the track scan itself
    // (find_vehicle) is done by RailVehiclePhysicsServer, which passes the center to center track distance
    void MoverVehicleController::update_neighbour(
            const int p_end, VehicleController *p_other, const int p_other_end, const double p_track_distance) {
        if (mover == nullptr) {
            return;
        }
        neighbour_data &neighbour = mover->Neighbours[p_end];
        const TCoupling &coupler = mover->Couplers[p_end];

        if (coupler.Connected != nullptr) {
            // physical connection with another vehicle locks down collision source on this end
            neighbour.vehicle = coupler.Connected;
            neighbour.vehicle_end = coupler.ConnectedNr;
            neighbour.distance = static_cast<float>(
                    TMoverParameters::CouplerDist(mover, coupler.Connected) - coupler.adapter_length -
                    coupler.Connected->Couplers[coupler.ConnectedNr].adapter_length);
            return;
        }

        neighbour = neighbour_data();
        const MoverVehicleController *other = Object::cast_to<MoverVehicleController>(p_other);
        if (other == nullptr || other->mover == nullptr) {
            return;
        }
        TMoverParameters *other_mover = other->mover;
        const TCoupling &other_coupler = other_mover->Couplers[p_other_end];
        neighbour.vehicle = other_mover;
        neighbour.vehicle_end = p_other_end;
        neighbour.distance = static_cast<float>(p_track_distance - 0.5 * (mover->Dim.L + other_mover->Dim.L));
        if (neighbour.distance < (other_mover->CategoryFlag == 2 ? 50 : 100)) {
            // at short distances (re)calculate range between couplers directly
            neighbour.distance = static_cast<float>(
                    TMoverParameters::CouplerDist(mover, other_mover) - coupler.adapter_length -
                    other_coupler.adapter_length);
        }
    }

    void MoverVehicleController::compute_forces(const double p_delta) {
        if (mover == nullptr) {
            return;
        }
        mover->ComputeTotalForce(p_delta);
    }

    void MoverVehicleController::compute_movement(const double p_delta) {
        _integrate(p_delta, Integration::FULL);
        // the Hasler recorder is vehicle state, not integration - it stays here until the state
        // registry takes it over
        _update_tachometer(p_delta);
    }

    /// The cheap movement of the intermediate physics iterations: the original runs UpdateForce +
    /// FastUpdate for every sub-iteration and the full Update() only once per frame
    /// (DynObj.cpp:8195-8210), where FastUpdate calls Mover::FastComputeMovement()
    /// (DynObj.cpp:4086) instead of the full ComputeMovement().
    void MoverVehicleController::compute_fast_movement(const double p_delta) {
        _integrate(p_delta, Integration::FAST);
    }

    void MoverVehicleController::_integrate(const double p_delta, const Integration p_integration) {
        // a standing vehicle switched off by ComputeTotalForce() is not moved at all
        // (DynObj.cpp:4059 FastUpdate, DynObj.cpp:2940 Update)
        if (mover == nullptr || !mover->PhysicActivation) {
            return;
        }
        TRotation rotation;
        if (p_integration == Integration::FULL) {
            mover->ComputeMovement(
                    p_delta, p_delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mover->Loc,
                    rotation);
        } else {
            mover->FastComputeMovement(p_delta, mover->RunningShape, mover->RunningTrack, mover->Loc, rotation);
        }
        // the vehicle is moved by this distance (DynObj.cpp:2439), front-relative
        mover->dMoveLen += mover->V * p_delta;
        // TTrain::add_distance (Train.cpp:10309) - counted towards the occupied cab, and switched
        // off for good whenever the low voltage goes
        if (distance_counter >= 0.0 && (mover->Power24vIsAvailable || mover->Power110vIsAvailable)) {
            distance_counter += mover->V * p_delta * mover->CabOccupied;
        } else {
            distance_counter = DISTANCE_COUNTER_OFF;
        }
    }

    // Original engine: TDynamicObject::AttachNext() couples with Enforce, without sound (DynObj.cpp:2590)
    void MoverVehicleController::couple(
            VehicleController *p_other, const int p_end, const int p_other_end, const int p_coupling_type) {
        MoverVehicleController *other = Object::cast_to<MoverVehicleController>(p_other);
        if (mover == nullptr || other == nullptr || other->mover == nullptr) {
            UtilityFunctions::push_error("Cannot couple vehicles without initialized movers.");
            return;
        }
        int coupling_type = p_coupling_type;
        // a coupler allowing only permanent coupling keeps it permanent (simulationstateserializer.cpp:990)
        if (coupling_type != coupling::faux && (mover->Couplers[p_end].AllowedFlag & coupling::permanent) != 0) {
            coupling_type |= coupling::permanent;
        }
        mover->Attach(p_end, p_other_end, other->mover, coupling_type, true, false);
        // the original re-inspects the consist on a coupling change (CheckVehicles(), Driver.cpp:2622)
        emit_signal(consist_changed_signal);
        other->emit_signal(consist_changed_signal);
    }

    void MoverVehicleController::uncouple(const int p_end) {
        if (mover == nullptr || mover->Couplers[p_end].Connected == nullptr) {
            return;
        }
        mover->Dettach(p_end);
        emit_signal(consist_changed_signal);
    }

    bool MoverVehicleController::is_coupled(const int p_end) const {
        return mover != nullptr && mover->Couplers[p_end].Connected != nullptr;
    }

    bool MoverVehicleController::is_coupled_by(const int p_end, const CouplingElement p_element) const {
        // indexed by CouplingElement
        static constexpr int flags[] = {coupling::coupler, coupling::brakehose, coupling::mainhose, coupling::control,
                                        coupling::gangway, coupling::heating,   coupling::permanent};
        return mover != nullptr && TestFlag(mover->Couplers[p_end].CouplingFlag, flags[p_element]);
    }

    // p_where is a coupler end (0 front, 1 rear) or a world position - then the vehicle end nearest to
    // it is used, like the walk mode commands of the original (ABuScanNearestObject, Train.cpp:6213)
    int MoverVehicleController::_resolve_coupler_end(const Variant &p_where) const {
        if (p_where.get_type() != Variant::VECTOR3) {
            return CLAMP(static_cast<int>(p_where), 0, 1);
        }
        const Transform3D transform = get_world_transform();
        // vehicles face -Z; the front coupler (end 0) is half the length ahead of the center
        const Vector3 front = transform.origin - transform.basis.get_column(2).normalized() * (0.5 * mover->Dim.L);
        const Vector3 rear = transform.origin + transform.basis.get_column(2).normalized() * (0.5 * mover->Dim.L);
        const Vector3 position = p_where;
        return position.distance_squared_to(front) <= position.distance_squared_to(rear) ? 0 : 1;
    }

    // Original engine: TDynamicObject::couple() (DynObj.cpp:1509) - one more coupling type per call,
    // with the vehicle detected at that end
    void MoverVehicleController::coupler_connect(const Variant &p_where) {
        if (mover == nullptr) {
            return;
        }
        const int side = _resolve_coupler_end(p_where);
        const neighbour_data &neighbour = mover->Neighbours[side];
        if (neighbour.vehicle == nullptr) {
            return;
        }
        const TCoupling &coupler = mover->Couplers[side];
        const TCoupling &other_coupler = neighbour.vehicle->Couplers[neighbour.vehicle_end];
        const int allowed = coupler.AllowedFlag & other_coupler.AllowedFlag;

        if (coupler.CouplingFlag == coupling::faux && (allowed & coupling::coupler) == coupling::coupler &&
            mover->Attach(side, neighbour.vehicle_end, neighbour.vehicle, coupling::coupler)) {
            return;
        }
        for (const int flag:
             {coupling::brakehose, coupling::mainhose, coupling::control, coupling::gangway, coupling::heating}) {
            if ((coupler.CouplingFlag & flag) == flag || (allowed & flag) != flag) {
                continue;
            }
            if (flag == coupling::control && coupler.control_type != other_coupler.control_type) {
                continue;
            }
            if (mover->Attach(side, neighbour.vehicle_end, neighbour.vehicle, coupler.CouplingFlag | flag)) {
                return;
            }
        }
    }

    // Original engine: TDynamicObject::uncouple() (DynObj.cpp:1614)
    void MoverVehicleController::coupler_disconnect(const Variant &p_where) {
        if (mover == nullptr) {
            return;
        }
        const int side = _resolve_coupler_end(p_where);
        if (mover->DettachStatus(side) >= 0 || (mover->Couplers[side].CouplingFlag & coupling::permanent) != 0) {
            return;
        }
        mover->Dettach(side);
    }

    // Original engine: TTrain::Update() Hasler block (Train.cpp:6917-6940) and its tachoclock
    // sound gate (Train.cpp:8323-8335).
    void MoverVehicleController::_update_tachometer(const double p_delta) {
        const double max_tachometer = 3.0;
        tachometer_velocity = std::min(std::abs(11.31 * mover->WheelDiameter * mover->nrot), mover->Vmax * 1.05);

        // the needle jumps once per simulation second, with a small random error
        const double previous_second = std::floor(tachometer_time);
        tachometer_time += p_delta;
        if (std::floor(tachometer_time) != previous_second) {
            tachometer_velocity_jump = tachometer_velocity > 1.0
                                               ? tachometer_velocity + (2.0 - UtilityFunctions::randf_range(0.0, 3.0) +
                                                                        UtilityFunctions::randf_range(0.0, 3.0)) *
                                                                               0.5
                                               : 0.0;
        }

        // ticking starts ~1 s after moving off and fades out slowly after stopping
        if (tachometer_velocity > 1.0) {
            tachometer_count = std::min(max_tachometer, tachometer_count + p_delta * 3.0);
        } else if (tachometer_count > 0.0) {
            tachometer_count = std::max(0.0, tachometer_count - p_delta * 0.66);
        }
        if (tachometer_count >= 3.0) {
            tachometer_clock_active = true;
        } else if (tachometer_count < 1.0) {
            tachometer_clock_active = false;
        }
    }

    /* The coupler events are consumed first, then the vehicle compares what it announces. */
    void MoverVehicleController::update_state() {
        if (mover != nullptr) {
            _consume_coupler_sounds();
        }
        VehicleController::update_state();
    }

    // The elements follow the original's coupling:: flags (Mover.cpp:590).
    // Original engine: coupler attach/detach sounds (DynObj.cpp:4855-4905) - each request of the mover
    // (TCoupling::sounds) bumps a counter the sound triggers play on; the flags are consumed as there.
    //
    // Consuming is a tick job, not a read job: this clears the mover's flags, so doing it while
    // filling the state dictionary made the events belong to whoever happened to read first.
    void MoverVehicleController::_consume_coupler_sounds() {
        static const int flags[] = {sound::attachcoupler, sound::attachbrakehose, sound::attachmainhose,
                                    sound::attachcontrol, sound::attachgangway,   sound::attachheating};
        for (TCoupling &coupler: mover->Couplers) {
            if (coupler.sounds == sound::none) {
                continue;
            }
            const bool detaching = (coupler.sounds & sound::detach) != 0;
            for (int index = 0; index < 6; ++index) {
                if ((coupler.sounds & flags[index]) != 0) {
                    emit_signal(
                            detaching ? coupler_detached_signal : coupler_attached_signal,
                            static_cast<CouplingElement>(index));
                }
            }
            coupler.sounds = sound::none;
        }
    }

    double MoverVehicleController::process_movement(const double p_delta) {
        return mover != nullptr ? mover->V * p_delta : 0.0;
    }

    void MoverVehicleController::apply_config() {
        if (mover == nullptr) {
            UtilityFunctions::push_warning("VehicleController::apply_config() failed: internal mover not initialized");
            return;
        }
        mover->Mass = get_mass();
        mover->Power = get_power();
        mover->Vmax = get_max_velocity();
        mover->Mred = get_reduced_mass();

        mover->ComputeMass();

        mover->CategoryFlag = get_category();
        mover->TrainType = get_train_type();
        mover->SandCapacity = static_cast<int>(get_sand_capacity());
        mover->HeatingPower = get_heating_power();
        mover->LightPower = get_light_power();

        mover->Dim.L = get_dimensions_length();
        mover->Dim.H = get_dimensions_height();
        mover->Dim.W = get_dimensions_width();
        mover->Cx = get_dimensions_drag_coefficient();
        mover->Floor = static_cast<float>(get_dimensions_floor_height());

        mover->BatteryStart = mover_start_mode(get_cntrl_battery_start_mode());
        // a Cntrl. key of every vehicle, not of an electric engine (Mover.cpp:10909 LoadFIZ_Cntrl) -
        // a diesel-electric's compressor runs off the converter too (CompressorPower=Converter)
        mover->ConverterStart = mover_start_mode(get_cntrl_converter_start_mode());
        mover->ConverterStartDelay = static_cast<float>(get_cntrl_converter_start_delay());
        mover->GroundRelayStart = mover_start_mode(get_cntrl_ground_relay_start_mode());
        mover->CompartmentLights.start_type = mover_start_mode(get_cntrl_compartment_lights_start_mode());
        mover->AutomaticCabActivation = get_cntrl_automatic_cab_activation();
        mover->InactiveCabFlag = get_cntrl_inactive_cab_flag();

        // FIXME: move to TrainPower
        mover->BatteryVoltage = get_battery_voltage();
        // the nominal voltage, which BatteryVoltage then drains from (Mover.cpp:946)
        mover->NominalBatteryVoltage = static_cast<float>(get_battery_voltage()); // LoadFIZ_Light
        emit_config_changed();

        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(get_initial_velocity() != 0.0, 0); // FIXME: brakujace parametery
        initialize_mover_state();
    }

    void MoverVehicleController::_fill_config_dictionary(Dictionary &p_config) const {
        if (mover == nullptr) {
            return;
        }
        // Vehicle-wide, not brake-specific - mover->Vmax is set from this same max_velocity
        // property (see apply_config() below), so this is a thin alias, not new derivation.
        p_config["max_speed"] = get_max_velocity();
        p_config["power"] = mover->Power;
        p_config["length"] = mover->Dim.L;
    }

    double MoverVehicleController::get_tachometer_speed() const {
        return mover != nullptr ? tachometer_velocity : 0.0;
    }

    double MoverVehicleController::get_tachometer_speed_jump() const {
        return mover != nullptr ? tachometer_velocity_jump : 0.0;
    }

    double MoverVehicleController::get_tachometer_clock_speed() const {
        return mover != nullptr ? tachometer_clock_active ? tachometer_velocity : 0.0 : 0.0;
    }

    int MoverVehicleController::get_direction_absolute() const {
        return mover != nullptr ? mover->DirAbsolute : 0;
    }

    int MoverVehicleController::get_cabin() const {
        return mover != nullptr ? mover->CabActive : 0;
    }

    bool MoverVehicleController::get_cabin_controleable() const {
        return mover != nullptr ? mover->IsCabMaster() : false;
    }

    int MoverVehicleController::get_cabin_occupied() const {
        return mover != nullptr ? mover->CabOccupied : 0;
    }

    double MoverVehicleController::get_live_battery_voltage() const {
        return mover != nullptr ? mover->BatteryVoltage : 0.0;
    }

    bool MoverVehicleController::get_battery_enabled() const {
        return mover != nullptr ? mover->Battery : false;
    }

    double MoverVehicleController::get_power24_voltage() const {
        return mover != nullptr ? mover->Power24vVoltage : 0.0;
    }

    bool MoverVehicleController::get_power24_available() const {
        return mover != nullptr ? mover->Power24vIsAvailable : false;
    }

    bool MoverVehicleController::get_power110_available() const {
        return mover != nullptr ? mover->Power110vIsAvailable : false;
    }

    double MoverVehicleController::get_current0() const {
        return mover != nullptr ? mover->ShowCurrent(0) : 0.0;
    }

    double MoverVehicleController::get_current1() const {
        return mover != nullptr ? mover->ShowCurrent(1) : 0.0;
    }

    double MoverVehicleController::get_current2() const {
        return mover != nullptr ? mover->ShowCurrent(2) : 0.0;
    }

    bool MoverVehicleController::get_relay_novolt() const {
        return mover != nullptr ? mover->NoVoltRelay : false;
    }

    bool MoverVehicleController::get_relay_overvoltage() const {
        return mover != nullptr ? mover->OvervoltageRelay : false;
    }

    bool MoverVehicleController::get_relay_ground() const {
        return mover != nullptr ? mover->GroundRelay : false;
    }

    int MoverVehicleController::get_train_damage() const {
        return mover != nullptr ? mover->DamageFlag : 0;
    }

    int MoverVehicleController::get_controller_second_position() const {
        return mover != nullptr ? mover->ScndCtrlPos : 0;
    }

    int MoverVehicleController::get_controller_main_position() const {
        return mover != nullptr ? mover->MainCtrlPos : 0;
    }

    int MoverVehicleController::get_controller_joint_position() const {
        return mover != nullptr
                       ? mover->LocalBrakePosA > 0.0
                                 ? static_cast<int>(std::round(-mover->LocalBrakePosA * LocalBrakePosNo))
                                 : (mover->CoupledCtrl ? mover->MainCtrlPos + mover->ScndCtrlPos : mover->MainCtrlPos)
                       : 0;
    }

    int MoverVehicleController::get_controller_main_actual_position() const {
        return mover != nullptr ? mover->MainCtrlActualPos : 0;
    }

    int MoverVehicleController::get_circuit_rlist_size() const {
        return mover != nullptr ? mover->RlistSize : 0;
    }

    double MoverVehicleController::get_velocity() const {
        return mover != nullptr ? mover->V : 0.0;
    }

    double MoverVehicleController::get_speed() const {
        return mover != nullptr ? mover->Vel : 0.0;
    }

    double MoverVehicleController::get_acceleration() const {
        return mover != nullptr ? mover->AccS : 0.0;
    }

    double MoverVehicleController::get_mass_total() const {
        return mover != nullptr ? mover->TotalMass : 0.0;
    }

    double MoverVehicleController::get_total_distance() const {
        return mover != nullptr ? mover->DistCounter : 0.0;
    }

    int MoverVehicleController::get_direction() const {
        return mover != nullptr ? mover->DirActive : 0;
    }

    void MoverVehicleController::battery(const bool p_enabled) const {
        mover->BatterySwitch(p_enabled);
    }

    // Original engine: OnCommand_cabactivationenable/disable (Train.cpp:2430-2472)
    void MoverVehicleController::cab_activation(const bool p_enabled) const {
        if (p_enabled) {
            mover->CabActivisation();
            return;
        }
        mover->CabDeactivisation();
    }

    // Original engine: taking over a vehicle activates its cab if the FIZ allows automatic
    // activation (Train.cpp:9086, 9147); otherwise the driver uses cab_activation
    void MoverVehicleController::cab_activation_auto() const {
        mover->CabActivisationAuto(true);
    }

    // Original engine: TTrain::CabChange() (Train.cpp:8516) - steps 1 -> 0 (machine room) -> -1.
    void MoverVehicleController::cab_change(const int p_direction) const {
        mover->CabDeactivisationAuto();
        mover->ChangeCab(p_direction);
        mover->CabActivisationAuto();
    }

    void MoverVehicleController::main_controller_increase(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->IncMainCtrl(step);
    }

    void MoverVehicleController::main_controller_decrease(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->DecMainCtrl(step);
    }

    // Original engine: OnCommand_secondcontrollerincrease/decrease (Train.cpp:1188, 1349), regular mode
    void MoverVehicleController::second_controller_increase(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->IncScndCtrl(step);
    }

    void MoverVehicleController::second_controller_decrease(const int p_step) const {
        const int step = p_step > 0 ? p_step : 1;
        mover->DecScndCtrl(step);
    }

    void MoverVehicleController::direction_increase() const {
        mover->DirectionForward();
    }

    void MoverVehicleController::direction_decrease() const {
        mover->DirectionBackward();
    }

    // Original engine: TTrain::OnCommand_distancecounteractivate (Train.cpp:1552), single-press form
    void MoverVehicleController::distance_counter_activate(const bool p_pressed) {
        if (p_pressed) {
            distance_counter = 0.0;
        }
    }

    double MoverVehicleController::get_distance_counter() const {
        return distance_counter;
    }
} // namespace godot
