#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include "../core/TrainPart.hpp"
#include "../engines/TrainEngine.hpp"
#include "../systems/TrainSecuritySystem.hpp"

namespace godot {

    const char *TrainController::MOVER_CONFIG_CHANGED_SIGNAL = "mover_config_changed";
    const char *TrainController::POWER_CHANGED_SIGNAL = "power_changed";
    const char *TrainController::COMMAND_RECEIVED = "command_received";

    void TrainController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_state"), &TrainController::set_state);
        ClassDB::bind_method(D_METHOD("get_state"), &TrainController::get_state);
        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "state"), "set_state", "get_state");

        ClassDB::bind_method(
                D_METHOD("receive_command", "command", "p1", "p2"), &TrainController::receive_command, DEFVAL(Variant()),
                DEFVAL(Variant()));

        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainController::get_mover_state);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainController::update_mover);

        ClassDB::bind_method(
                D_METHOD("_on_train_part_config_changed"), &TrainController::_on_train_part_config_changed);
        ClassDB::bind_method(D_METHOD("set_type_name"), &TrainController::set_type_name);
        ClassDB::bind_method(D_METHOD("get_type_name"), &TrainController::get_type_name);
        ClassDB::bind_method(D_METHOD("set_mass"), &TrainController::set_mass);
        ClassDB::bind_method(D_METHOD("get_mass"), &TrainController::get_mass);
        ClassDB::bind_method(D_METHOD("set_power"), &TrainController::set_power);
        ClassDB::bind_method(D_METHOD("get_power"), &TrainController::get_power);
        ClassDB::bind_method(D_METHOD("set_max_velocity"), &TrainController::set_max_velocity);
        ClassDB::bind_method(D_METHOD("get_max_velocity"), &TrainController::get_max_velocity);
        ClassDB::bind_method(D_METHOD("set_axle_arrangement"), &TrainController::set_axle_arrangement);
        ClassDB::bind_method(D_METHOD("get_axle_arrangement"), &TrainController::get_axle_arrangement);
        ClassDB::bind_method(D_METHOD("set_nominal_battery_voltage"), &TrainController::set_nominal_battery_voltage);
        ClassDB::bind_method(D_METHOD("get_nominal_battery_voltage"), &TrainController::get_nominal_battery_voltage);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "type_name"), "set_type_name", "get_type_name");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dimensions/mass"), "set_mass", "get_mass");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "power"), "set_power", "get_power");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_velocity"), "set_max_velocity", "get_max_velocity");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "axle_arrangement"), "set_axle_arrangement", "get_axle_arrangement");

        ADD_SIGNAL(MethodInfo(MOVER_CONFIG_CHANGED_SIGNAL));
        ADD_SIGNAL(MethodInfo(POWER_CHANGED_SIGNAL, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(
                COMMAND_RECEIVED, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"),
                PropertyInfo(Variant::NIL, "p2")));

        /* FIXME: move to TrainPower section? */
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "nominal_battery_voltage", PROPERTY_HINT_RANGE, "0,500,1"),
                "set_nominal_battery_voltage", "get_nominal_battery_voltage");
    }

    TrainController::TrainController() {
        // FIXME: move to _init or _ready signal?
    }

    TMoverParameters *TrainController::get_mover() const {
        return mover;
    }

    void TrainController::initialize_mover() {
        mover = new TMoverParameters(
                this->initial_velocity, std::string(this->type_name.utf8().ptr()),
                std::string(this->get_name().left(this->get_name().length()).utf8().ptr()), this->cabin_number);

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(true, 0); // FIXME: brakujace parametery

        /* CheckLocomotiveParameters() will reset some parameters, so the changes
         * must be applied second time */

        _dirty = true;
        _dirty_prop = true;
        _update_mover_config_if_dirty();

        /* FIXME: remove test data */
        mover->CabActive = 1;
        mover->CabMaster = true;
        mover->CabOccupied = 1;
        mover->AutomaticCabActivation = true;
        mover->CabActivisationAuto();
        mover->CabActivisation();

        /* switch_physics() raczej trzeba zostawic */
        mover->switch_physics(true);

        UtilityFunctions::print("[MaSzyna::TMoverParameters] Mover initialized successfully");
    }

    void TrainController::set_type_name(const String &p_type_name) {
        type_name = p_type_name;
    }

    String TrainController::get_type_name() const {
        return type_name;
    }

    void TrainController::_connect_signals_to_train_part(TrainPart *part) {
        if (part != nullptr) {
            const Callable _c = Callable(this, "_on_train_part_config_changed").bind(part);
            part->connect("config_changed", _c); // Clang-Tidy: The value returned by this function should not be
                                                 // disregarded; neglecting it may lead to errors
        }
    }
    void TrainController::_ready() {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        initialize_mover();

        UtilityFunctions::print("TrainController::_ready() signals connected to train parts");

        emit_signal(POWER_CHANGED_SIGNAL, is_powered);
    }

    void TrainController::_update_mover_config_if_dirty() {
        if (_dirty) {
            /* update all train parts
             */
            emit_signal(MOVER_CONFIG_CHANGED_SIGNAL);

            _dirty = false;
            _dirty_prop = true; // sforsowanie odswiezenia stanu lokalnych propsow
        }

        if (_dirty_prop) {
            update_mover();
            _dirty_prop = false;
        }
    }

    void TrainController::_process_mover(const double delta) {
        TLocation mock_location;
        TRotation mock_rotation;
        mover->ComputeTotalForce(delta);
        // mover->compute_movement_(delta);
        mover->ComputeMovement(
                delta, delta, mover->RunningShape, mover->RunningTrack, mover->RunningTraction, mock_location,
                mock_rotation);

        _handle_mover_update();
    }

    void TrainController::_handle_mover_update() {
        state.merge(get_mover_state(), true);

        const bool new_is_powered = (state.get("power24_available", false) || state.get("power110_available", false));
        if (is_powered != new_is_powered) {
            is_powered = new_is_powered;
            emit_signal(POWER_CHANGED_SIGNAL, is_powered);
        }
    }

    void TrainController::_process(const double delta) {
        /* nie daj borze w edytorze */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        _update_mover_config_if_dirty();
        _process_mover(delta);
    }

    void TrainController::_do_update_internal_mover(TMoverParameters *mover) const {
        mover->Mass = mass;
        mover->Power = power;
        mover->Vmax = max_velocity;

        mover->ComputeMass();
        mover->NPoweredAxles = Maszyna::s2NPW(axle_arrangement.ascii().get_data());
        mover->NAxles = mover->NPoweredAxles + Maszyna::s2NNW(axle_arrangement.ascii().get_data());

        // FIXME: move to TrainPower
        mover->NominalBatteryVoltage = nominal_battery_voltage;
        mover->BatteryVoltage = nominal_battery_voltage;
    }

    void TrainController::_on_train_part_config_changed(TrainPart *part) const {
        if (part == nullptr) {
            return;
        }
        part->update_mover();
    }

    double TrainController::get_nominal_battery_voltage() const {
        return nominal_battery_voltage;
    }

    void TrainController::set_nominal_battery_voltage(const double p_nominal_battery_voltage) {
        nominal_battery_voltage = p_nominal_battery_voltage;
        _dirty_prop = true;
    }

    void TrainController::set_mass(const double p_mass) {
        mass = p_mass;
        _dirty_prop = true;
    }

    double TrainController::get_mass() const {
        return mass;
    }

    void TrainController::set_power(const double p_power) {
        power = p_power;
        _dirty_prop = true;
    }

    double TrainController::get_power() const {
        return power;
    }

    void TrainController::set_max_velocity(const double p_value) {
        max_velocity = p_value;
        _dirty_prop = true;
    }

    double TrainController::get_max_velocity() const {
        return max_velocity;
    }

    void TrainController::set_axle_arrangement(String p_value) {
        axle_arrangement = p_value;
    }

    String TrainController::get_axle_arrangement() const {
        return axle_arrangement;
    }

    void TrainController::update_mover() const {
        TMoverParameters *mover = get_mover();
        if (mover != nullptr) {
            _do_update_internal_mover(mover);
        } else {
            UtilityFunctions::push_warning("TrainController::update_mover() failed: internal mover not initialized");
        }
    }

    Dictionary TrainController::get_mover_state() {
        TMoverParameters *mover = get_mover();
        if (mover != nullptr) {
            _do_fetch_state_from_mover(mover, state);
        } else {
            UtilityFunctions::push_warning("TrainController::get_mover_state() failed: internal mover not initialized");
        }
        return internal_state;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void TrainController::_do_fetch_state_from_mover(
            TMoverParameters *mover, Dictionary &state) { // NOLINT(*-convert-member-functions-to-static)
        internal_state["mass_total"] = mover->TotalMass;
        internal_state["velocity"] = mover->V;
        internal_state["speed"] = mover->Vel;
        internal_state["total_distance"] = mover->DistCounter;
        internal_state["direction"] = mover->DirActive;
        internal_state["cabin"] = mover->CabActive;
        internal_state["cabin_controleable"] = mover->IsCabMaster();
        internal_state["cabin_occupied"] = mover->CabOccupied;

        /* FIXME: should be just a config property IMO; better to fix AxleArangement */
        internal_state["axles_powered_count"] = mover->NPoweredAxles;
        internal_state["axles_count"] = mover->NAxles;

        /* FIXME: move to TrainPower section? */
        internal_state["battery_voltage"] = mover->BatteryVoltage;

        /* FIXME: move to TrainPower section */
        internal_state["power24_voltage"] = mover->Power24vVoltage;
        internal_state["power24_available"] = mover->Power24vIsAvailable;
        internal_state["power110_available"] = mover->Power110vIsAvailable;
        internal_state["current0"] = mover->ShowCurrent(0);
        internal_state["current1"] = mover->ShowCurrent(1);
        internal_state["current2"] = mover->ShowCurrent(2);
        internal_state["relay_novolt"] = mover->NoVoltRelay;
        internal_state["relay_overvoltage"] = mover->OvervoltageRelay;
        internal_state["relay_ground"] = mover->GroundRelay;
        internal_state["train_damage"] = mover->DamageFlag;
        internal_state["controller_second_position"] = mover->ScndCtrlPos;
        internal_state["controller_main_position"] = mover->MainCtrlPos;
    }

    void TrainController::set_state(const Dictionary &p_state) {
        state = p_state;
    }

    Dictionary TrainController::get_state() {
        return state;
    }

    void TrainController::receive_command(const StringName &command, const Variant &p1, const Variant &p2) {
        _on_command_received(String(command), p1, p2);
        emit_signal(COMMAND_RECEIVED, command, p1, p2);
        if(mover != nullptr) {
            _handle_mover_update();
        }
    }

    void TrainController::_on_command_received(const String &command, const Variant &p1, const Variant &p2) {
        if (!mover) {
            return;
        }
        if (command == "battery_enable") {
            mover->BatterySwitch((bool)p1);
        } else if(command == "main_controller_increase") {
            UtilityFunctions::print("main_controller_increase !");
            mover->IncMainCtrl(1);
        } else if(command == "main_controller_decrease") {
            UtilityFunctions::print("main_controller_decrease !");
            mover->DecMainCtrl(1);
        } else if(command == "forwarder_increase") {
            mover->DirectionForward();
        } else if(command == "forwarder_decrease") {
            mover->DirectionBackward();
        }
    }
} // namespace godot
