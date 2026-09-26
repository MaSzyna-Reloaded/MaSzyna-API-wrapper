#include "SemaphoreKind.hpp"

namespace godot {
    void SemaphoreKind::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_aspects", "aspects"), &SemaphoreKind::set_aspects);
        ClassDB::bind_method(D_METHOD("get_aspects"), &SemaphoreKind::get_aspects);
        ClassDB::bind_method(D_METHOD("set_blink_on_time", "time"), &SemaphoreKind::set_blink_on_time);
        ClassDB::bind_method(D_METHOD("get_blink_on_time"), &SemaphoreKind::get_blink_on_time);
        ClassDB::bind_method(D_METHOD("set_blink_off_time", "time"), &SemaphoreKind::set_blink_off_time);
        ClassDB::bind_method(D_METHOD("get_blink_off_time"), &SemaphoreKind::get_blink_off_time);
        ClassDB::bind_method(D_METHOD("get_aspect_names"), &SemaphoreKind::get_aspect_names);
        ClassDB::bind_method(D_METHOD("has_aspect", "aspect"), &SemaphoreKind::has_aspect);
        ClassDB::bind_method(D_METHOD("get_aspect", "aspect"), &SemaphoreKind::get_aspect);

        ADD_PROPERTY(
                PropertyInfo(Variant::DICTIONARY, "aspects", PROPERTY_HINT_DICTIONARY_TYPE, "StringName;SemaphoreAspect"),
                "set_aspects", "get_aspects");
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "blink_on_time", PROPERTY_HINT_RANGE, "0.01,10,0.01,suffix:s"),
                "set_blink_on_time", "get_blink_on_time");
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "blink_off_time", PROPERTY_HINT_RANGE, "0.01,10,0.01,suffix:s"),
                "set_blink_off_time", "get_blink_off_time");
    }

    void SemaphoreKind::set_aspects(const Dictionary &p_aspects) {
        aspects = p_aspects;
    }

    Dictionary SemaphoreKind::get_aspects() const {
        return aspects;
    }

    void SemaphoreKind::set_blink_on_time(const float p_time) {
        blink_on_time = p_time;
    }

    float SemaphoreKind::get_blink_on_time() const {
        return blink_on_time;
    }

    void SemaphoreKind::set_blink_off_time(const float p_time) {
        blink_off_time = p_time;
    }

    float SemaphoreKind::get_blink_off_time() const {
        return blink_off_time;
    }

    PackedStringArray SemaphoreKind::get_aspect_names() const {
        PackedStringArray names;
        const Array keys = aspects.keys();
        for (int i = 0; i < keys.size(); i++) {
            names.push_back(keys[i]);
        }
        return names;
    }

    bool SemaphoreKind::has_aspect(const StringName &p_aspect) const {
        return aspects.has(p_aspect);
    }

    Ref<SemaphoreAspect> SemaphoreKind::get_aspect(const StringName &p_aspect) const {
        return aspects.get(p_aspect, Ref<SemaphoreAspect>());
    }
} // namespace godot
