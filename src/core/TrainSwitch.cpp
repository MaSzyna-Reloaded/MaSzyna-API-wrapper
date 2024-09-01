#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainSwitch.hpp"

namespace godot {
    TrainSwitch::TrainSwitch() = default;

    void TrainSwitch::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_pushed", "pushed"), &TrainSwitch::set_pushed);
        ClassDB::bind_method(D_METHOD("get_pushed"), &TrainSwitch::get_pushed);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pushed"), "set_pushed", "get_pushed");
    }

    void TrainSwitch::set_pushed(const bool p_state) {
        pushed = p_state;
        _dirty = true;
    }
    bool TrainSwitch::get_pushed() const {
        return pushed;
    }

} // namespace godot
