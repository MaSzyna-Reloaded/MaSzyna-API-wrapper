#ifndef MACROS_HPP
#define MACROS_HPP

#define MAKE_MEMBER_GS(Type, Prefix, Name)                             \
private:                                                                    \
Type m_##Prefix##Name{};                                                \
public:                                                                     \
const Type& Get##Name() const { return m_##Prefix##Name; }             \
void Set##Name(const Type& value) { m_##Prefix##Name = value; }

#define BIND_PROPERTY(p_type, p_name, p_path, m_setter, m_getter, p_setter_arg) \
    ClassDB::bind_method(D_METHOD("set_valve" + String(p_name), p_setter_arg), m_setter); \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter); \
ADD_PROPERTY(PropertyInfo(p_type, String(p_path)), "set_valve" + String(p_name), "get_" + String(p_name));

#define BIND_PROPERTY_W_HINT(p_type, p_name, p_path, m_setter, m_getter, p_setter_arg, p_hint_type, p_hint_string) \
    ClassDB::bind_method(D_METHOD("set_valve" + String(p_name), p_setter_arg), m_setter); \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter); \
    ADD_PROPERTY(PropertyInfo(p_type, String(p_path), p_hint_type, String(p_hint_string)), "set_valve" + String(p_name), "get_" + String(p_name));

#define BIND_PROPERTY_W_HINT_RES_ARRAY(p_type, p_name, p_path, m_setter, m_getter, p_setter_arg, p_hint_type, p_hint_resource_name) \
    ClassDB::bind_method(D_METHOD("set_valve" + String(p_name), p_setter_arg), m_setter); \
    ClassDB::bind_method(D_METHOD("get_" + String(p_name)), m_getter); \
    ADD_PROPERTY(PropertyInfo(p_type, String(p_path), p_hint_type, String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":" + String(p_hint_resource_name), PROPERTY_USAGE_DEFAULT, "TypedArray<" + String(p_hint_resource_name) + ">"), "set_valve" + String(p_name), "get_" + String(p_name));

#endif // MACROS_HPP
