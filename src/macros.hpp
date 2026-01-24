#ifndef MACROS_HPP
#define MACROS_HPP

/**
 * Macro for generating private members with their setters and getters
 * @param type Member type
 * @param name Member name
 * @param default_value Default value
 */
#define MAKE_MEMBER_GS(type, name, default_value)                                                                      \
private:                                                                                                               \
    type name = default_value;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
    const type &get_##name() const {                                                                                   \
        return name;                                                                                                   \
    }                                                                                                                  \
    void set_##name(const type &value) {                                                                               \
        name = value;                                                                                                  \
    }

/**
 * Macro for generating private synchronized members with their setters and getters. Those members will ALWAYS be
 * synchronized with internal mover
 * @param type Member type
 * @param name Member name
 * @param default_value Default value
 */
#define MAKE_MEMBER_GS_DIRTY(type, name, default_value)                                                                \
private:                                                                                                               \
    type name = default_value;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
    const type &get_##name() const {                                                                                   \
        return name;                                                                                                   \
    }                                                                                                                  \
    void set_##name(const type &value) {                                                                               \
        name = value;                                                                                                  \
        _dirty = true;                                                                                                 \
    }

/**
 * Macro for generating private members with their setters and getters without a const reference
 * @param type Member type
 * @param name Member name
 * @param default_value Default value
 */
#define MAKE_MEMBER_GS_NR(type, name, default_value)                                                                   \
private:                                                                                                               \
    type name = default_value;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
    type get_##name() const {                                                                                    \
        return name;                                                                                                   \
    }                                                                                                                  \
    void set_##name(type value) {                                                                                      \
        name = value;                                                                                                  \
    }

#define MAKE_MEMBER_GS_NR_DIRTY(type, name, default_value)                                                             \
private:                                                                                                               \
    type name = default_value;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
    type get_##name() const {                                                                                          \
        return name;                                                                                                   \
    }                                                                                                                  \
    void set_##name(type value) {                                                                                      \
        name = value;                                                                                                  \
        _dirty = true;                                                                                                 \
    }

/**
 * Generates Godot setters and getters with the desired property using the following:
 * @param p_type Property type
 * @param p_name Property full name, used to generate setter and getter method names
 * @param p_path Property path in the inspector
 * @param m_setter Setter reference
 * @param m_getter Getter reference
 * @param p_setter_arg Argument name for setter
 * <example>
 * BIND_PROPERTY(Variant::FLOAT, "ca_max_hold_time", "ca_max_hold_time", &TrainSecuritySystem::SetCAMaxHoldTime,
 * &TrainSecuritySystem::GetCAMaxHoldTime, "delay");
 * </example>
 */
#define BIND_PROPERTY(p_type, p_name, p_path, m_setter, m_getter, p_setter_arg)                                        \
    ClassDB::bind_method(D_METHOD("set_" + String(p_name), p_setter_arg), m_setter);                                   \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter);                                                 \
    ADD_PROPERTY(PropertyInfo(p_type, String(p_path)), "set_" + String(p_name), "get_" + String(p_name));

/**
 * Generates Godot setters and getters with the desired property using the following:
 * @param p_name Property full name, used to generate setter and getter method names
 * @param p_path Property path in the inspector
 * @param m_setter Setter reference
 * @param m_getter Getter reference
 * @param p_setter_arg Argument name for setter
 * <example>
 * BIND_PROPERTY(Variant::FLOAT, "ca_max_hold_time", "ca_max_hold_time", &TrainSecuritySystem::SetCAMaxHoldTime,
 * &TrainSecuritySystem::GetCAMaxHoldTime, "delay");
 * </example>
 */
#define BIND_PROPERTY_ARRAY(p_name, p_path, m_setter, m_getter, p_setter_arg)                                          \
    ClassDB::bind_method(D_METHOD("set_" + String(p_name), p_setter_arg), m_setter);                                   \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter);                                                 \
    ADD_PROPERTY(                                                                                                      \
            PropertyInfo(Variant::ARRAY, String(p_path), PROPERTY_HINT_ARRAY_TYPE), "set_" + String(p_name),           \
            "get_" + String(p_name));

/**
 * Generates Godot setters and getters with the desired property using the following with a hint:
 * @param p_type Property type (Variant type)
 * @param p_name Property full name, used to generate setter and getter method names
 * @param p_path Property path in the inspector
 * @param m_setter Setter reference
 * @param m_getter Getter reference
 * @param p_setter_arg Argument name for setter
 * @param p_hint_type Property hint type
 * @param p_hint_string Property hint string
 * <example>
 * BIND_PROPERTY_W_HINT(Variant::INT, "selector_position", "selector_position",
 * &TrainLighting::SetSelectorPosition, &TrainLighting::GetSelectorPosition, "position",
 * PROPERTY_HINT_RANGE, "0,4,1");
 * </example>
 */
#define BIND_PROPERTY_W_HINT(p_type, p_name, p_path, m_setter, m_getter, p_setter_arg, p_hint_type, p_hint_string)     \
    ClassDB::bind_method(D_METHOD("set_" + String(p_name), p_setter_arg), m_setter);                                   \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter);                                                 \
    ADD_PROPERTY(                                                                                                      \
            PropertyInfo(p_type, String(p_path), p_hint_type, String(p_hint_string)), "set_" + String(p_name),         \
            "get_" + String(p_name));

/**
 * Generates Godot setters and getters for resource array properties:
 * @param p_type Property type (Variant type)
 * @param p_name Property full name, used to generate setter and getter method names
 * @param p_path Property path in the inspector
 * @param m_setter Setter reference
 * @param m_getter Getter reference
 * @param p_setter_arg Argument name for setter
 * @param p_hint_type Property hint type
 * @param p_hint_resource_name Resource type name
 * <example>
 * BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "lights", "lights",
 * &TrainLighting::SetLightPositionList, &TrainLighting::GetLightPositionList, "list",
 * PROPERTY_HINT_ARRAY_TYPE, "LightListItem");
 * </example>
 */
#define BIND_PROPERTY_W_HINT_RES_ARRAY(                                                                                \
        p_type, p_name, p_path, m_setter, m_getter, p_setter_arg, p_hint_type, p_hint_resource_name)                   \
    ClassDB::bind_method(D_METHOD("set_" + String(p_name), p_setter_arg), m_setter);                                   \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter);                                                 \
    ADD_PROPERTY(                                                                                                      \
            PropertyInfo(                                                                                              \
                    p_type, String(p_path), p_hint_type,                                                               \
                    String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":" +              \
                            String(p_hint_resource_name),                                                              \
                    PROPERTY_USAGE_DEFAULT, "TypedArray<" + String(p_hint_resource_name) + ">"),                       \
            "set_" + String(p_name), "get_" + String(p_name));

#endif // MACROS_HPP
