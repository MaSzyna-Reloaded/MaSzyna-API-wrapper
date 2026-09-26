#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

    /// Runtime-wide services shared by every cache: the single "throw the caches away" entry point
    /// and the build stamp those caches are keyed against. It also carries the state of the world
    /// the whole simulation shares - the light level and the air temperature as the environment
    /// publishes them (Global.fLuminance, Global.AirTemperature) - and runs the one clock of the
    /// simulation (Timer::UpdateTimers(), Timer.cpp:70-92): the physics, the events, the drivers
    /// and the time of day all advance by the same seconds, read here.
    class MaszynaRuntime : public Object {
            GDCLASS(MaszynaRuntime, Object)

        private:
            static constexpr const char *BUILD_NUMBER_PATH = "res://build_number.txt";
            static constexpr const char *BUILD_SECTION = "app";
            static constexpr const char *BUILD_NUMBER_KEY = "build_number";

            String build_number;
            bool build_number_read = false;
            bool build_version_checked = false;
            /// Timer.cpp:84: a frame advances the simulation by at most this [s] - a machine that
            /// cannot keep up makes the simulation run slower than the wall clock, it never jumps
            static constexpr double MAX_FRAME_TIME = 1.0;
            static constexpr double SECONDS_PER_HOUR = 3600.0;
            static constexpr double MINUTES_PER_HOUR = 60.0;
            static constexpr double HOURS_PER_DAY = 24.0;

            double time_of_day = 0.0;
            double simulation_time = 0.0;
            double simulation_speed = 1.0;
            /// Who needs the clock running, and the node that ticks it while they do
            int clock_holders = 0;
            ObjectID clock_id;
            double light_level = 1.0;
            double air_temperature = 0.0;
            bool paused = false;

            void _refresh_clock();

        protected:
            static void _bind_methods();

        public:
            static const char *simulation_advanced_signal;
            static const char *cache_clear_requested_signal;
            static const char *language_changed_signal;
            static const char *paused_signal;
            static const char *unpaused_signal;
            static const char *simulation_speed_changed_signal;
            static const char *time_of_day_changed_signal;
            /// The original's own strings, untranslated - no catalogue needed
            static constexpr const char *DEFAULT_LANGUAGE = "en";

            static MaszynaRuntime *get_instance() {
                return dynamic_cast<MaszynaRuntime *>(Engine::get_singleton()->get_singleton("MaszynaRuntime"));
            }

            void clear_cache();
            String get_build_number();
            bool check_build_version();

            /// Hours since midnight, fractional. Setting it jumps the clock (a scenario's start, the
            /// player's change, the system time); running, it advances with the simulation.
            void set_time_of_day(double p_hours);
            double get_time_of_day() const;
            /// Simulated seconds so far (fSimulationTime, Timer.cpp:87)
            double get_simulation_time() const;
            /// The clock runs while somebody holds it and the runtime is not paused: whoever needs
            /// time to pass holds it while it does, and lets it go
            void clock_hold();
            void clock_release();
            /// One frame of the clock, `p_frame_delta` real seconds (Timer::UpdateTimers(),
            /// Timer.cpp:79-87): that times the simulation speed, at most MAX_FRAME_TIME, added to
            /// the simulation time and the time of day, then `simulation_advanced(seconds)`.
            /// Called by the clock's node, processed before every other.
            void advance(double p_frame_delta);
            /// How many simulated seconds pass in one real second (Global.fTimeSpeed, Timer.cpp:80)
            void set_simulation_speed(double p_speed);
            double get_simulation_speed() const;
            /// How bright the scene is, 0-1 (Global.fLuminance, simulationenvironment.cpp:184)
            void set_light_level(double p_level);
            double get_light_level() const;
            /// Degrees Celsius
            void set_air_temperature(double p_temperature);
            double get_air_temperature() const;
            /// The language of the game's strings, by the name of its catalogue in `lang/`
            /// ("pl" for lang/pl.po; Global.asLang, Globals.cpp:137). Kept in the user settings.
            void set_language(const String &p_language);
            String get_language() const;
            /// Stops the world while something covers it (a loading screen, the spinner of "Exit to
            /// menu"): whoever runs a part of the simulation, or its sound, holds it on "paused"
            /// and lets it go on "unpaused"
            void pause();
            void unpause();
            bool is_paused() const;
    };

} // namespace godot
