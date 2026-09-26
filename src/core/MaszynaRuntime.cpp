#include "MaszynaRuntime.hpp"

#include "SimulationClock.hpp"
#include "UserSettings.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *MaszynaRuntime::cache_clear_requested_signal = "cache_clear_requested";
    const char *MaszynaRuntime::language_changed_signal = "language_changed";
    const char *MaszynaRuntime::paused_signal = "paused";
    const char *MaszynaRuntime::unpaused_signal = "unpaused";
    const char *MaszynaRuntime::simulation_speed_changed_signal = "simulation_speed_changed";
    const char *MaszynaRuntime::time_of_day_changed_signal = "time_of_day_changed";
    const char *MaszynaRuntime::simulation_advanced_signal = "simulation_advanced";

    namespace {
        constexpr const char *LANGUAGE_SECTION = "maszyna";
        constexpr const char *LANGUAGE_KEY = "language";
    } // namespace

    void MaszynaRuntime::_bind_methods() {
        ClassDB::bind_method(D_METHOD("clear_cache"), &MaszynaRuntime::clear_cache);
        ClassDB::bind_method(D_METHOD("get_build_number"), &MaszynaRuntime::get_build_number);
        ClassDB::bind_method(D_METHOD("check_build_version"), &MaszynaRuntime::check_build_version);

        ClassDB::bind_method(D_METHOD("set_time_of_day", "hours"), &MaszynaRuntime::set_time_of_day);
        ClassDB::bind_method(D_METHOD("get_time_of_day"), &MaszynaRuntime::get_time_of_day);
        ClassDB::bind_method(D_METHOD("set_simulation_speed", "speed"), &MaszynaRuntime::set_simulation_speed);
        ClassDB::bind_method(D_METHOD("get_simulation_speed"), &MaszynaRuntime::get_simulation_speed);
        ClassDB::bind_method(D_METHOD("set_light_level", "level"), &MaszynaRuntime::set_light_level);
        ClassDB::bind_method(D_METHOD("get_light_level"), &MaszynaRuntime::get_light_level);
        ClassDB::bind_method(D_METHOD("set_air_temperature", "temperature"), &MaszynaRuntime::set_air_temperature);
        ClassDB::bind_method(D_METHOD("get_air_temperature"), &MaszynaRuntime::get_air_temperature);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_of_day"), "set_time_of_day", "get_time_of_day");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "simulation_speed"), "set_simulation_speed", "get_simulation_speed");
        ADD_SIGNAL(MethodInfo(simulation_speed_changed_signal));
        ADD_SIGNAL(MethodInfo(time_of_day_changed_signal));
        ClassDB::bind_method(D_METHOD("get_simulation_time"), &MaszynaRuntime::get_simulation_time);
        ClassDB::bind_method(D_METHOD("clock_hold"), &MaszynaRuntime::clock_hold);
        ClassDB::bind_method(D_METHOD("clock_release"), &MaszynaRuntime::clock_release);
        ClassDB::bind_method(D_METHOD("advance", "frame_delta"), &MaszynaRuntime::advance);
        ADD_SIGNAL(MethodInfo(simulation_advanced_signal, PropertyInfo(Variant::FLOAT, "seconds")));
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_level"), "set_light_level", "get_light_level");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_temperature"), "set_air_temperature", "get_air_temperature");
        ClassDB::bind_method(D_METHOD("set_language", "language"), &MaszynaRuntime::set_language);
        ClassDB::bind_method(D_METHOD("get_language"), &MaszynaRuntime::get_language);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "language"), "set_language", "get_language");
        ADD_SIGNAL(MethodInfo(language_changed_signal));

        ADD_SIGNAL(MethodInfo(cache_clear_requested_signal));

        ClassDB::bind_method(D_METHOD("pause"), &MaszynaRuntime::pause);
        ClassDB::bind_method(D_METHOD("unpause"), &MaszynaRuntime::unpause);
        ClassDB::bind_method(D_METHOD("is_paused"), &MaszynaRuntime::is_paused);
        ADD_SIGNAL(MethodInfo(paused_signal));
        ADD_SIGNAL(MethodInfo(unpaused_signal));
    }

    void MaszynaRuntime::set_time_of_day(const double p_hours) {
        if (time_of_day == p_hours) {
            return;
        }
        time_of_day = p_hours;
        emit_signal(time_of_day_changed_signal);
    }

    double MaszynaRuntime::get_time_of_day() const {
        return time_of_day;
    }

    double MaszynaRuntime::get_simulation_time() const {
        return simulation_time;
    }

    void MaszynaRuntime::clock_hold() {
        ++clock_holders;
        _refresh_clock();
    }

    void MaszynaRuntime::clock_release() {
        ERR_FAIL_COND(clock_holders <= 0);
        --clock_holders;
        _refresh_clock();
    }

    /// The node ticks while the clock is held and the runtime is not paused
    void MaszynaRuntime::_refresh_clock() {
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

    void MaszynaRuntime::advance(const double p_frame_delta) {
        const double seconds = MIN(p_frame_delta * simulation_speed, MAX_FRAME_TIME);
        if (seconds <= 0.0) {
            return;
        }
        simulation_time += seconds;
        // the launchers look at whole minutes (EvLaunch.cpp:197-211): the clock says so when one
        // passes, not every frame
        const double previous = time_of_day;
        time_of_day = Math::fposmod(time_of_day + seconds / SECONDS_PER_HOUR, HOURS_PER_DAY);
        if (!(Math::floor(previous * MINUTES_PER_HOUR) == Math::floor(time_of_day * MINUTES_PER_HOUR))) {
            emit_signal(time_of_day_changed_signal);
        }
        emit_signal(simulation_advanced_signal, seconds);
    }

    void MaszynaRuntime::set_simulation_speed(const double p_speed) {
        if (simulation_speed == p_speed) {
            return;
        }
        simulation_speed = p_speed;
        emit_signal(simulation_speed_changed_signal);
    }

    double MaszynaRuntime::get_simulation_speed() const {
        return simulation_speed;
    }

    void MaszynaRuntime::set_light_level(const double p_level) {
        light_level = p_level;
    }

    double MaszynaRuntime::get_light_level() const {
        return light_level;
    }

    void MaszynaRuntime::set_air_temperature(const double p_temperature) {
        air_temperature = p_temperature;
    }

    double MaszynaRuntime::get_air_temperature() const {
        return air_temperature;
    }

    void MaszynaRuntime::set_language(const String &p_language) {
        if (p_language == get_language()) {
            return;
        }
        UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL(user_settings);
        user_settings->save_setting(LANGUAGE_SECTION, LANGUAGE_KEY, p_language);
        emit_signal(language_changed_signal);
    }

    String MaszynaRuntime::get_language() const {
        const UserSettings *user_settings = UserSettings::get_instance();
        ERR_FAIL_NULL_V(user_settings, DEFAULT_LANGUAGE);
        return user_settings->get_setting(LANGUAGE_SECTION, LANGUAGE_KEY, DEFAULT_LANGUAGE);
    }

    void MaszynaRuntime::pause() {
        if (paused) {
            return;
        }
        paused = true;
        _refresh_clock();
        emit_signal(paused_signal);
    }

    void MaszynaRuntime::unpause() {
        if (!paused) {
            return;
        }
        paused = false;
        _refresh_clock();
        emit_signal(unpaused_signal);
    }

    bool MaszynaRuntime::is_paused() const {
        return paused;
    }

    void MaszynaRuntime::clear_cache() {
        emit_signal(cache_clear_requested_signal);
    }

    /// Stamped by the build itself (cmake/write_build_number.cmake), empty in a checkout that was
    /// never built.
    String MaszynaRuntime::get_build_number() {
        if (build_number_read) {
            return build_number;
        }

        build_number_read = true;
        build_number = FileAccess::get_file_as_string(BUILD_NUMBER_PATH).strip_edges();
        return build_number;
    }

    /// Clears every cache once per run when the build behind them is not the one that wrote them.
    /// A cache on disk outlives the code that produced it, so a new build starts from clean data.
    bool MaszynaRuntime::check_build_version() {
        if (build_version_checked) {
            return false;
        }

        build_version_checked = true;

        const String current = get_build_number();
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
                "[MaszynaRuntime] Build changed (" + (stored.is_empty() ? String("none") : stored) + " -> " + current +
                "), clearing cache...");
        clear_cache();
        settings->save_setting(BUILD_SECTION, BUILD_NUMBER_KEY, current);
        return true;
    }

} // namespace godot
