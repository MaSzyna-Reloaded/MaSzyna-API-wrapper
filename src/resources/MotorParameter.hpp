#pragma once
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class MotorParameter : public Resource {
            GDCLASS(MotorParameter, Resource);

        public:
            static void _bind_methods();

            double shunting_up = 0.0;
            double shunting_down = 0.0;
            double mfi = 0.0;
            double mIsat = 0.0;
            double mfi0 = 0.0; // aproksymacja M(I) silnika} {dla dizla mIsat=przekladnia biegu
            double fi = 0.0;
            double Isat = 0.0;
            double fi0 = 0.0; // aproksymacja E(n)=fi*n}    {dla dizla fi, mfi: predkosci przelozenia biegu <->
            bool auto_switch = false;

            void set_shunting_up(double p_shunting_up);
            void set_shunting_down(double p_shunting_down);
            void set_mfi(double p_mfi);
            void set_mIsat(double p_mIsat);
            void set_mfi0(double p_mfi0);
            void set_fi(double p_fi);
            void set_Isat(double p_Isat);
            void set_fi0(double p_fi0);
            void set_auto_switch(bool p_auto_switch);
            double get_shunting_up() const;
            double get_shunting_down() const;
            double get_mfi() const;
            double get_mIsat() const;
            double get_mfi0() const;
            double get_fi() const;
            double get_Isat() const;
            double get_fi0() const;
            bool get_auto_switch() const;
    };
} // namespace godot
