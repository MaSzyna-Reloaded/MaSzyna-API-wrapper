#ifndef MACROS_HPP
#define MACROS_HPP

#define MAKE_MEMBER_GS(Type, Prefix, Name)                             \
private:                                                                    \
Type m_##Prefix##Name{};                                                \
public:                                                                     \
const Type& Get##Name() const { return m_##Prefix##Name; }             \
void Set##Name(const Type& value) { m_##Prefix##Name = value; }

#endif // MACROS_HPP
