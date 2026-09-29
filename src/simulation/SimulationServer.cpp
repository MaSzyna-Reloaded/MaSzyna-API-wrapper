#include "SimulationServer.hpp"

#include "SimulationClock.hpp"
#include "utils/UserSettings.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *SimulationServer::cache_clear_requested_signal = "cache_clear_requested";
    const char *SimulationServer::language_changed_signal = "language_changed";
    const char *SimulationServer::simulation_paused_signal = "simulation_paused";
    const char *SimulationServer::simulation_unpaused_signal = "simulation_unpaused";
    const char *SimulationServer::simulation_speed_changed_signal = "simulation_speed_changed";
    const char *SimulationServer::simulation_current_speed_changed_signal = "simulation_current_speed_changed";
    const char *SimulationServer::time_of_day_changed_signal = "time_of_day_changed";
    const char *SimulationServer::simulation_advanced_signal = "simulation_advanced";

    namespace {
        constexpr const char *LANGUAGE_SECTION = "maszyna";
        constexpr const char *LANGUAGE_KEY = "language";
    } // namespace

    void SimulationServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("cache_clear"), &SimulationServer::cache_clear);
        ClassDB::bind_method(D_METHOD("build_get_number"), &SimulationServer::build_get_number);
        ClassDB::bind_method(D_METHOD("build_check_version"), &SimulationServer::build_check_version);

        ClassDB::bind_method(D_METHOD("set_time_of_day", "hours"), &SimulationServer::set_time_of_day);
        ClassDB::bind_method(D_METHOD("get_time_of_day"), &SimulationServer::get_time_of_day);
        ClassDB::bind_method(D_METHOD("set_simulation_speed", "speed"), &SimulationServer::set_simulation_speed);
        ClassDB::bind_method(D_METHOD("get_simulation_speed"), &SimulationServer::get_simulation_speed);
        ClassDB::bind_method(D_METHOD("simulation_get_current_speed"), &SimulationServer::simulation_get_current_speed);
        ClassDB::bind_method(D_METHOD("simulation_reset_speed"), &SimulationServer::simulation_reset_speed);
        ClassDB::bind_method(D_METHOD("set_light_level", "level"), &SimulationServer::set_light_level);
        ClassDB::bind_method(D_METHOD("get_light_level"), &SimulationServer::get_light_level);
        ClassDB::bind_method(D_METHOD("set_air_temperature", "temperature"), &SimulationServer::set_air_temperature);
        ClassDB::bind_method(D_METHOD("get_air_temperature"), &SimulationServer::get_air_temperature);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_of_day"), "set_time_of_day", "get_time_of_day");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "simulation_speed"), "set_simulation_speed", "get_simulation_speed");
        ADD_SIGNAL(MethodInfo(simulation_speed_changed_signal));
        ADD_SIGNAL(MethodInfo(simulation_current_speed_changed_signal));
        ADD_SIGNAL(MethodInfo(time_of_day_changed_signal));
        ClassDB::bind_method(D_METHOD("simulation_get_time"), &SimulationServer::simulation_get_time);
        ClassDB::bind_method(D_METHOD("clock_hold"), &SimulationServer::clock_hold);
        ClassDB::bind_method(D_METHOD("clock_release"), &SimulationServer::clock_release);
        ClassDB::bind_method(D_METHOD("simulation_advance", "frame_delta"), &SimulationServer::simulation_advance);
        ADD_SIGNAL(MethodInfo(simulation_advanced_signal, PropertyInfo(Variant::FLOAT, "seconds")));
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_level"), "set_light_level", "get_light_level");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_temperature"), "set_air_temperature", "get_air_temperature");
        ClassDB::bind_method(D_METHOD("set_language", "language"), &SimulationServer::set_language);
        ClassDB::bind_method(D_METHOD("get_language"), &SimulationServer::get_language);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "language"), "set_language", "get_language");
        ADD_SIGNAL(MethodInfo(language_changed_signal));

        ADD_SIGNAL(MethodInfo(cache_clear_requested_signal));

        ClassDB::bind_method(D_METHOD("simulation_pause"), &SimulationServer::simulation_pause);
        ClassDB::bind_method(D_METHOD("simulation_unpause"), &SimulationServer::simulation_unpause);
        ClassDB::bind_method(D_METHOD("simulation_is_paused"), &SimulationServer::simulation_is_paused);
        ADD_SIGNAL(MethodInfo(simulation_paused_signal));
        ADD_SIGNAL(MethodInfo(simulation_unpaused_signal));
    }

    void SimulationServer::set_time_of_day(const double p_hours) {
        if (time_of_day == p_hours) {
            return;
        }
        time_of_day = p_hours;
        emit_signal(time_of_day_changed_signal);
    }

    double SimulationServer::get_time_of_day() const {
        return time_of_day;
    }

    double SimulationServer::simulation_get_time() const {
        return simulation_time;
    }

    void SimulationServer::clock_hold() {
        ++clock_holders;
        _refresh_clock();
    }

    void SimulationServer::clock_release() {
        ERR_FAIL_COND(clock_holders <= 0);
        --clock_holders;
        _refresh_clock();
    }

    /// The node ticks while the clock is held and the runtime is not paused
    void SimulationServer::_refresh_clock() {
        const bool ticking = clock_holders > 0 && !paused;
        Node *clock = Object::cast_to<Node>(ObjectDB::get_instance(clock_id));
        if (ticking == (clock != nullptr)) {
            return;
        }
        if (!ticking) {
            // at once, not at the end of the frame the node is freed in
            clock->set_process(false);
            clock->queue_free();
            clock_id = ObjectID();
            return;
        }
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            return;
        }
        SimulationClock *node = memnew(SimulationClock);
        clock_id = node->get_instance_id();
        // internal, so a node nobody declared does not turn up in the root's children and
        // surprise whatever walks the tree
        tree->get_root()->add_child(node, false, Node::INTERNAL_MODE_FRONT);
    }

    void SimulationServer::simulation_advance(const double p_frame_delta) {
        const double frame_delta = MIN(p_frame_delta, MAX_FRAME_DELTA);
        if (!(current_simulation_speed == simulation_speed)) {
            // a tape's motor: the running speed closes on the one set, most of the way in
            // speed_change_time
            const double closing = speed_change_time > 0.0 ? 1.0 - Math::exp(-frame_delta / speed_change_time) : 1.0;
            current_simulation_speed += (simulation_speed - current_simulation_speed) * closing;
            if (Math::abs(simulation_speed - current_simulation_speed) < SPEED_SETTLED) {
                current_simulation_speed = simulation_speed;
            }
            emit_signal(simulation_current_speed_changed_signal);
        }
        const double seconds = frame_delta * current_simulation_speed;
        if (seconds <= 0.0) {
            return;
        }
        // equal slices, so a frame of 0.25 s is 3 x 0.083 s and not 0.1 + 0.1 + 0.05
        const int slices = static_cast<int>(Math::ceil(seconds / MAX_SLICE_TIME));
        const double slice = seconds / slices;
        for (int index = 0; index < slices; ++index) {
            simulation_time += slice;
            // the launchers look at whole minutes (EvLaunch.cpp:197-211): the clock says so when
            // one passes, not every slice
            const double previous = time_of_day;
            time_of_day = Math::fposmod(time_of_day + (slice / SECONDS_PER_HOUR), HOURS_PER_DAY);
            if (!(Math::floor(previous * MINUTES_PER_HOUR) == Math::floor(time_of_day * MINUTES_PER_HOUR))) {
                emit_signal(time_of_day_changed_signal);
            }
            emit_signal(simulation_advanced_signal, slice);
        }
    }

    void SimulationServer::set_simulation_speed(const double p_speed) {
        if (simulation_speed == p_speed) {
            return;
        }
        simulation_speed = p_speed;
        speed_change_time =
                ProjectSettings::get_singleton()->get_setting(SPEED_CHANGE_TIME_SETTING, SPEED_CHANGE_TIME_DEFAULT);
        emit_signal(simulation_speed_changed_signal);
    }

    double SimulationServer::get_simulation_speed() const {
        return simulation_speed;
    }

    double SimulationServer::simulation_get_current_speed() const {
        return current_simulation_speed;
    }

    void SimulationServer::simulation_reset_speed() {
        set_simulation_speed(1.0);
        if (current_simulation_speed == simulation_speed) {
            return;
        }
        current_simulation_speed = simulation_speed;
        emit_signal(simulation_current_speed_changed_signal);
    }

    void SimulationServer::set_light_level(const double p_level) {
        light_level = p_level;
    }

    double SimulationServer::get_light_level() const {
        return light_level;
    }

    void SimulationServer::set_air_temperature(const double p_temperature) {
        air_temperature = p_temperature;
    }

    double SimulationServer::get_air_temperature() const {
        return air_temperature;
    }

    void SimulationServer::set_language(const String &p_language) {
        if (p_language == get_language()) {
            return;
        }
        UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL(user_settings);
        user_settings->save_setting(LANGUAGE_SECTION, LANGUAGE_KEY, p_language);
        emit_signal(language_changed_signal);
    }

    String SimulationServer::get_language() const {
        const UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL_V(user_settings, DEFAULT_LANGUAGE);
        return user_settings->get_setting(LANGUAGE_SECTION, LANGUAGE_KEY, DEFAULT_LANGUAGE);
    }

    void SimulationServer::simulation_pause() {
        if (paused) {
            return;
        }
        paused = true;
        _refresh_clock();
        emit_signal(simulation_paused_signal);
    }

    void SimulationServer::simulation_unpause() {
        if (!paused) {
            return;
        }
        paused = false;
        _refresh_clock();
        emit_signal(simulation_unpaused_signal);
    }

    bool SimulationServer::simulation_is_paused() const {
        return paused;
    }

    void SimulationServer::cache_clear() {
        emit_signal(cache_clear_requested_signal);
    }

    /// Stamped by the build itself (cmake/write_build_number.cmake), empty in a checkout that was
    /// never built.
    String SimulationServer::build_get_number() {
        if (build_number_read) {
            return build_number;
        }

        build_number_read = true;
        build_number = FileAccess::get_file_as_string(BUILD_NUMBER_PATH).strip_edges();
        return build_number;
    }

    /// Clears every cache once per run when the build behind them is not the one that wrote them.
    /// A cache on disk outlives the code that produced it, so a new build starts from clean data.
    bool SimulationServer::build_check_version() {
        if (build_version_checked) {
            return false;
        }

        build_version_checked = true;

        const String current = build_get_number();
        if (current.is_empty()) {
            return false;
        }

        UserSettings *settings = UserSettings::get_instance();
        ERR_FAIL_NULL_V(settings, false);

        const String stored = settings->get_setting(BUILD_SECTION, BUILD_NUMBER_KEY, "");
        if (stored == current) {
            return false;
        }

        UtilityFunctions::print(
                "[SimulationServer] Build changed (" + (stored.is_empty() ? String("none") : stored) + " -> " +
                current + "), clearing cache...");
        cache_clear();
        settings->save_setting(BUILD_SECTION, BUILD_NUMBER_KEY, current);
        return true;
    }

} // namespace godot
