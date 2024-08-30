#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../core/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include "../engines/TrainEngine.hpp"
#include "../core/TrainPart.hpp"
#include "../systems/TrainSecuritySystem.hpp"
#include "../core/TrainSwitch.hpp"
#include "maszyna/McZapkie/MOVER.h"

namespace godot {

    void TrainController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_brake"), &TrainController::get_brake);
        ClassDB::bind_method(D_METHOD("get_engine"), &TrainController::get_engine);
        ClassDB::bind_method(D_METHOD("get_security_system"), &TrainController::get_security_system);

        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainController::get_mover_state);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainController::update_mover);

        ClassDB::bind_method(
                D_METHOD("_on_train_part_config_changed"), &TrainController::_on_train_part_config_changed);
        ClassDB::bind_method(D_METHOD("set_type_name"), &TrainController::set_type_name);
        ClassDB::bind_method(D_METHOD("get_type_name"), &TrainController::get_type_name);
        ClassDB::bind_method(D_METHOD("set_mass"), &TrainController::set_mass);
        ClassDB::bind_method(D_METHOD("get_mass"), &TrainController::get_mass);
        ClassDB::bind_method(D_METHOD("set_battery_enabled"), &TrainController::set_battery_enabled);
        ClassDB::bind_method(D_METHOD("get_battery_enabled"), &TrainController::get_battery_enabled);
        ClassDB::bind_method(D_METHOD("set_nominal_battery_voltage"), &TrainController::set_nominal_battery_voltage);
        ClassDB::bind_method(D_METHOD("get_nominal_battery_voltage"), &TrainController::get_nominal_battery_voltage);

        ClassDB::bind_method(D_METHOD("set_engine_path"), &TrainController::set_engine_path);
        ClassDB::bind_method(D_METHOD("get_engine_path"), &TrainController::get_engine_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::NODE_PATH, "parts/engine", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "TrainEngine"),
                "set_engine_path", "get_engine_path");

        ClassDB::bind_method(D_METHOD("set_brake_path"), &TrainController::set_brake_path);
        ClassDB::bind_method(D_METHOD("get_brake_path"), &TrainController::get_brake_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::NODE_PATH, "parts/brake", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "TrainBrake"),
                "set_brake_path", "get_brake_path");

        ClassDB::bind_method(D_METHOD("set_security_system_path"), &TrainController::set_security_system_path);
        ClassDB::bind_method(D_METHOD("get_security_system_path"), &TrainController::get_security_system_path);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::NODE_PATH, "parts/security_system", PROPERTY_HINT_NODE_PATH_VALID_TYPES,
                        "TrainSecuritySystem"),
                "set_security_system_path", "get_security_system_path");

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "type_name"), "set_type_name", "get_type_name");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dimensions/mass"), "set_mass", "get_mass");

        /* FIXME: move to TrainPower section? */
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/battery_enabled"), "set_battery_enabled", "get_battery_enabled");
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "nominal_battery_voltage", PROPERTY_HINT_RANGE, "0,500,1"),
                "set_nominal_battery_voltage", "get_nominal_battery_voltage");
    }

    TrainController::TrainController() {
        // FIXME: move to _init or _ready signal?
        initialize_mover();
    }

    TMoverParameters *TrainController::get_mover() const {
        return mover;
    }

    void TrainController::set_brake_path(const NodePath &p_path) {
        if (brake_path != p_path) {
            brake_path = p_path;
            _brake = nullptr;
            _dirty = true;
        }
    }

    NodePath TrainController::get_brake_path() const {
        return brake_path;
    }

    TrainBrake *TrainController::get_brake() const {
        if (_brake) {
            return _brake;
        }

        if (!brake_path.is_empty()) {
            Node *node = get_node_or_null(brake_path);
            if (node) {
                _brake = cast_to<TrainBrake>(node);
            }
        }
        return _brake;
    }

    void TrainController::set_engine_path(const NodePath &p_path) {
        if (engine_path != p_path) {
            engine_path = p_path;
            _engine = nullptr;
            _dirty = true;
        }
    }

    NodePath TrainController::get_engine_path() const {
        return engine_path;
    }

    TrainEngine *TrainController::get_engine() const {
        if (_engine) {
            return _engine;
        }

        if (!engine_path.is_empty()) {
            Node *node = get_node_or_null(engine_path);
            if (node) {
                _engine = cast_to<TrainEngine>(node);
            }
        }
        return _engine;
    }

    void TrainController::set_security_system_path(const NodePath &p_path) {
        if (security_system_path != p_path) {
            security_system_path = p_path;
            _security_system = nullptr;
            _dirty = true;
        }
    }

    NodePath TrainController::get_security_system_path() const {
        return security_system_path;
    }

    TrainSecuritySystem *TrainController::get_security_system() const {
        if (_security_system) {
            return _security_system;
        }

        if (!security_system_path.is_empty()) {
            Node *node = get_node_or_null(security_system_path);
            if (node) {
                _security_system = cast_to<TrainSecuritySystem>(node);
            }
        }
        return _security_system;
    }

    void TrainController::initialize_mover() {
        mover = new TMoverParameters(
                this->initial_velocity, std::string(this->type_name.utf8().ptr()),
                std::string(this->get_name().left(this->get_name().length()).utf8().ptr()), this->cabin_number);


        /* FIXME: CheckLocomotiveParameters should be called after (re)initialization */
        mover->CheckLocomotiveParameters(true, 0); // FIXME: brakujace parametery

        /* FIXME: remove test data */
        mover->CabActive = 1;
        mover->CabMaster = true;
        mover->CabOccupied = 1;
        mover->AutomaticCabActivation = true;
        mover->CabActivisationAuto();
        mover->CabActivisation();

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
            part->connect("config_changed", _c);// Clang-Tidy: The value returned by this function should not be disregarded; neglecting it may lead to errors
        }
    }
    void TrainController::_ready() {
        /* nie daj borze w edytorze */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        _connect_signals_to_train_part(get_brake());
        _connect_signals_to_train_part(get_engine());
        _connect_signals_to_train_part(get_security_system());

        /* eksperymentalna obsluga switchy umieszczanych jako dzieci TrainControllera
         * automatycznie podpina sygnaly bez koniecznosci podpinania switchy przez NodePath/Assign */
        Vector<TrainSwitch *> switches = get_train_switches();
        for (const auto switche : switches) {
            TrainSwitch *_s = cast_to<TrainSwitch>(switche);
            if (_s) {
                _connect_signals_to_train_part(_s);
            }
        }

        _dirty = true;      // force refresh mover
        _dirty_prop = true; // force refresh mover

        UtilityFunctions::print("TrainController::_ready() signals connected to train parts");
    }

    void TrainController::_process(const double delta) {
        /* nie daj borze w edytorze */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (_dirty) {
            /* zmienia sie istotny property glownego kontrolera
             * trzeba wszystkie czesci odswiezyc
             */
            _on_train_part_config_changed(get_brake());
            _on_train_part_config_changed(get_engine());
            _on_train_part_config_changed(get_security_system());

            /* eksperymentalna obsluga switchy umieszczanych jako dzieci TrainControllera */
            Vector<TrainSwitch *> switches = get_train_switches();
            for (int i = 0; i < switches.size(); i++) {
                _on_train_part_config_changed(switches[i]);
            }

            // mover->CheckLocomotiveParameters(true, 0);

            _dirty = false;
            _dirty_prop = true; // sforsowanie odswiezenia stanu lokalnych propsow
        }

        if (_dirty_prop) {
            update_mover();
            _dirty_prop = false;
        }

        mover->compute_movement_(delta * 1000.0);
    }

    void TrainController::_do_update_internal_mover(TMoverParameters *mover) const {
        mover->Mass = mass;
        mover->ComputeMass();

        // FIXME: move to TrainPower
        mover->Battery = sw_battery_enabled;
        mover->NominalBatteryVoltage = nominal_battery_voltage;
        mover->BatteryVoltage = nominal_battery_voltage;
    }

    void TrainController::_on_train_part_config_changed(TrainPart *part) const {
        if (part == nullptr) {
            return;
        }

        /* TODO: initialize_mover()
         *
         * przy propsach ustawianych bezposrednio do movera skasuje dane w runtime
         *  mozna tak zrobic, gdy wszystkie parametry beda w private memebers wezlow godota
         * wtedy mozna byloby wywalic initialize_mover() z konstruktora
         */

        // initialize_mover();

        part->update_mover(this);

        /* switch_physics() raczej trzeba zostawic */
        mover->switch_physics(true);
    }

    double TrainController::get_nominal_battery_voltage() const {
        return nominal_battery_voltage;
    }
    void TrainController::set_brake_level(const double p_brake_level) {
        //@TODO: Implement
    }

    void TrainController::set_nominal_battery_voltage(const double p_nominal_battery_voltage) {
        nominal_battery_voltage = p_nominal_battery_voltage;
        _dirty_prop = true;
    }

    void TrainController::set_battery_enabled(const bool p_state) {
        sw_battery_enabled = p_state;
        _dirty_prop = true;
    }

    bool TrainController::get_battery_enabled() const {
        return sw_battery_enabled;
    }

    void TrainController::set_mass(const double p_mass) {
        mass = p_mass;
        _dirty_prop = true;
    }

    double TrainController::get_mass() const {
        return mass;
    }

    Vector<TrainSwitch *> TrainController::get_train_switches() {
        Vector<TrainSwitch *> train_switches;
        _collect_train_switches(this, train_switches);
        return train_switches;
    }

    void TrainController::_collect_train_switches(const Node *node, Vector<TrainSwitch *> &train_switches) {
        if (node == nullptr) {
            return;
        }
        for (int i = 0; i < node->get_child_count(); ++i) {
            Node *child = node->get_child(i);

            TrainSwitch *train_switch = dynamic_cast<TrainSwitch *>(child);
            if (train_switch != nullptr) {
                train_switches.push_back(train_switch);
            }
            _collect_train_switches(child, train_switches);
        }
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
        return state;
    }

    void TrainController::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["mass_total"] = mover->TotalMass;
        state["velocity"] = mover->V;
        state["speed"] = mover->Vel;
        state["direction"] = mover->DirActive;
        state["cabin"] = mover->CabActive;
        state["cabin_controleable"] = mover->IsCabMaster();
        state["cabin_occupied"] = mover->CabOccupied;

        /* FIXME: move to TrainPower section? */
        state["battery_voltage"] = mover->BatteryVoltage;

        /* FIXME: move to TrainPower section */
        state["power24_voltage"] = mover->Power24vVoltage;
        state["power24_available"] = mover->Power24vIsAvailable;
        state["power110_available"] = mover->Power110vIsAvailable;
        state["current0"] = mover->ShowCurrent(0);
        state["current1"] = mover->ShowCurrent(1);
        state["current2"] = mover->ShowCurrent(2);
    }


} // namespace godot
