#include "MotorParameter.hpp"

namespace godot {
    void MotorParameter::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_shunting_up", "shunting_up"), &MotorParameter::set_shunting_up);
        ClassDB::bind_method(D_METHOD("get_shunting_up"), &MotorParameter::get_shunting_up);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shunting_up"), "set_shunting_up", "get_shunting_up");

        ClassDB::bind_method(D_METHOD("set_shunting_down", "shunting_down"), &MotorParameter::set_shunting_down);
        ClassDB::bind_method(D_METHOD("get_shunting_down"), &MotorParameter::get_shunting_down);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shunting_down"), "set_shunting_down", "get_shunting_down");

        ClassDB::bind_method(D_METHOD("set_mfi", "mfi"), &MotorParameter::set_mfi);
        ClassDB::bind_method(D_METHOD("get_mfi"), &MotorParameter::get_mfi);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mfi"), "set_mfi", "get_mfi");

        ClassDB::bind_method(D_METHOD("set_mIsat", "mIsat"), &MotorParameter::set_mIsat);
        ClassDB::bind_method(D_METHOD("get_mIsat"), &MotorParameter::get_mIsat);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mIsat"), "set_mIsat", "get_mIsat");

        ClassDB::bind_method(D_METHOD("set_mfi0", "mfi0"), &MotorParameter::set_mfi0);
        ClassDB::bind_method(D_METHOD("get_mfi0"), &MotorParameter::get_mfi0);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mfi0"), "set_mfi0", "get_mfi0");

        ClassDB::bind_method(D_METHOD("set_fi", "fi"), &MotorParameter::set_fi);
        ClassDB::bind_method(D_METHOD("get_fi"), &MotorParameter::get_fi);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fi"), "set_fi", "get_fi");

        ClassDB::bind_method(D_METHOD("set_Isat", "Isat"), &MotorParameter::set_Isat);
        ClassDB::bind_method(D_METHOD("get_Isat"), &MotorParameter::get_Isat);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Isat"), "set_Isat", "get_Isat");

        ClassDB::bind_method(D_METHOD("set_fi0", "fi0"), &MotorParameter::set_fi0);
        ClassDB::bind_method(D_METHOD("get_fi0"), &MotorParameter::get_fi0);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fi0"), "set_fi0", "get_fi0");

        ClassDB::bind_method(D_METHOD("set_auto_switch", "auto_switch"), &MotorParameter::set_auto_switch);
        ClassDB::bind_method(D_METHOD("get_auto_switch"), &MotorParameter::get_auto_switch);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_switch"), "set_auto_switch", "get_auto_switch");
    }

    void MotorParameter::set_shunting_up(const double p_shunting_up) {
        shunting_up = p_shunting_up;
    }

    void MotorParameter::set_shunting_down(const double p_shunting_down) {
        shunting_down = p_shunting_down;
    }

    void MotorParameter::set_mfi(const double p_mfi) {
        mfi = p_mfi;
    }

    void MotorParameter::set_mIsat(const double p_mIsat) {
        mIsat = p_mIsat;
    }

    void MotorParameter::set_mfi0(const double p_mfi0) {
        mfi0 = p_mfi0;
    }

    void MotorParameter::set_fi(const double p_fi) {
        fi = p_fi;
    }

    void MotorParameter::set_Isat(const double p_Isat) {
        Isat = p_Isat;
    }

    void MotorParameter::set_fi0(const double p_fi0) {
        fi0 = p_fi0;
    }

    void MotorParameter::set_auto_switch(const bool p_auto_switch) {
        auto_switch = p_auto_switch;
    }

    double MotorParameter::get_shunting_up() const {
        return shunting_up;
    }

    double MotorParameter::get_shunting_down() const {
        return shunting_down;
    }

    double MotorParameter::get_mfi() const {
        return mfi;
    }

    double MotorParameter::get_mIsat() const {
        return mIsat;
    }

    double MotorParameter::get_mfi0() const {
        return mfi0;
    }

    double MotorParameter::get_fi() const {
        return fi;
    }

    double MotorParameter::get_Isat() const {
        return Isat;
    }

    double MotorParameter::get_fi0() const {
        return fi0;
    }

    bool MotorParameter::get_auto_switch() const {
        return auto_switch;
    }
} // namespace godot
