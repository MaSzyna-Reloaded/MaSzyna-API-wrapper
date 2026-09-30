#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {
    /* What a kind of rail vehicle looks like: the models it is drawn with and the submodels of its
     * exterior model that move - bogies, wheels, pantograph arms, wipers, mirrors. Shared by every
     * vehicle of the kind and handed to RailVehicleRenderingServer, which builds and animates the
     * vehicle from it. Submodels are named as the model names them; a missing one is an empty
     * name. */
    class RailVehicleAppearance : public Resource {
            GDCLASS(RailVehicleAppearance, Resource)

        public:
            /* The submodel a head display material is drawn on - the wrapper's own convention: the
             * original swaps replaceable skin 4 instead (update_destinations(), DynObj.cpp:3055) */
            static constexpr const char *DEFAULT_HEAD_DISPLAY_SUBMODEL = "tablice_relacyjne";
            /* How bright the low-poly interior glows with the roof light on [emission energy], and
             * how fast it follows it [s] - the wrapper's own */
            static constexpr double DEFAULT_LOW_POLY_EMISSION_ENERGY = 0.2;
            static constexpr double DEFAULT_LOW_POLY_EMISSION_FADE_TIME = 0.2;
            /* The slider's width [m] for a vehicle whose FIZ declares none (CSW, MOVER.h:881) - the
             * wrapper's own default */
            static constexpr double DEFAULT_PANTOGRAPH_COLLECTOR_WIDTH = 0.5;

        private:
            String data_path;
            String model_filename;
            String low_poly_model_filename;
            String passengers_model_filename;
            PackedStringArray skins;
            Transform3D model_transform;
            String front_bogie;
            String rear_bogie;
            PackedStringArray front_rolling_wheels;
            PackedStringArray powered_wheels;
            PackedStringArray rear_rolling_wheels;
            PackedStringArray pantograph_front_arms;
            PackedStringArray pantograph_rear_arms;
            PackedStringArray wiper_arms;
            PackedStringArray mirrors;
            String head_display_submodel = DEFAULT_HEAD_DISPLAY_SUBMODEL;
            double pantograph_collector_width = DEFAULT_PANTOGRAPH_COLLECTOR_WIDTH;
            double low_poly_emission_energy = DEFAULT_LOW_POLY_EMISSION_ENERGY;
            double low_poly_emission_fade_time = DEFAULT_LOW_POLY_EMISSION_FADE_TIME;
            bool joint_cabs = false;

        protected:
            static void _bind_methods();

        public:
            /* Where the model files are, and the exterior, the low-poly interior seen through the
             * windows and the passengers; skins by number of the model's replaceable materials */
            void set_data_path(const String &p_value);
            String get_data_path() const;
            void set_model_filename(const String &p_value);
            String get_model_filename() const;
            void set_low_poly_model_filename(const String &p_value);
            String get_low_poly_model_filename() const;
            void set_passengers_model_filename(const String &p_value);
            String get_passengers_model_filename() const;
            void set_skins(const PackedStringArray &p_value);
            PackedStringArray get_skins() const;
            /* Where every model of the vehicle sits in the vehicle's own frame */
            void set_model_transform(const Transform3D &p_value);
            Transform3D get_model_transform() const;
            void set_front_bogie(const String &p_value);
            String get_front_bogie() const;
            void set_rear_bogie(const String &p_value);
            String get_rear_bogie() const;
            void set_front_rolling_wheels(const PackedStringArray &p_value);
            PackedStringArray get_front_rolling_wheels() const;
            void set_powered_wheels(const PackedStringArray &p_value);
            PackedStringArray get_powered_wheels() const;
            void set_rear_rolling_wheels(const PackedStringArray &p_value);
            PackedStringArray get_rear_rolling_wheels() const;
            /* Lower arm, its pair, upper arm, its pair, slider (TAnimPant, DynObj.cpp:5414) */
            void set_pantograph_front_arms(const PackedStringArray &p_value);
            PackedStringArray get_pantograph_front_arms() const;
            void set_pantograph_rear_arms(const PackedStringArray &p_value);
            PackedStringArray get_pantograph_rear_arms() const;
            /* Arm 1, arm 2 and blade of every wiper (DynObj.cpp:5838-5870) */
            void set_wiper_arms(const PackedStringArray &p_value);
            PackedStringArray get_wiper_arms() const;
            /* In the original's order, odd on the left, even on the right (DynObj.cpp:5887-5910) */
            void set_mirrors(const PackedStringArray &p_value);
            PackedStringArray get_mirrors() const;
            void set_head_display_submodel(const String &p_value);
            String get_head_display_submodel() const;
            void set_pantograph_collector_width(double p_value);
            double get_pantograph_collector_width() const;
            void set_low_poly_emission_energy(double p_value);
            double get_low_poly_emission_energy() const;
            void set_low_poly_emission_fade_time(double p_value);
            double get_low_poly_emission_fade_time() const;
            /* One low-poly cab for both ends (jointcabs:, DynObj.cpp:2236-2250) */
            void set_joint_cabs(bool p_value);
            bool get_joint_cabs() const;
    };
} // namespace godot
