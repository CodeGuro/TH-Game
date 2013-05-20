#include <bytecode.hpp>

type_data::type_data() : kind( tk_invalid ), element( invalidIndex )
{
}
type_data::type_data( type_kind k, size_t e ) : kind(k), element(e)
{
}
type_data::type_kind type_data::get_kind() const
{
	return kind;
}
size_t type_data::get_element() const
{
	return element;
}


script_data::script_data() : type( type_data::tk_invalid, invalidIndex )
{
}
script_data::script_data( float real, size_t elementIndex ) : real(real), type(type_data::tk_real, elementIndex)
{
}
script_data::script_data( char character, size_t elementIndex ) : character(character), type(type_data::tk_char, elementIndex)
{
}
script_data::script_data( bool boolean, size_t elementIndex ) : real( boolean? 1.f:0.f ), type(type_data::tk_boolean, elementIndex)
{
}
script_data::script_data( size_t objIndex, size_t elementIndex ) : objIndex(objIndex), type(type_data::tk_object, elementIndex)
{
}

code::code() : command( vc_invalid )
{
}
code::code( instruction c ) : command( c )
{
}
code code::varLev( instruction c, size_t varIndex, size_t levelUp )
{
	code res = code( c );
	res.variableIndex = varIndex;
	res.variableLevel = levelUp;
	return res;
}
code code::subArg( instruction c, size_t subIndex, size_t subArgc )
{
	code res = code( c );
	res.subIndex = subIndex;
	res.argc = subArgc;
	return res;
}
code code::loop( instruction c, size_t loopBackIndex )
{
	code res = code( c );
	res.loopBackIndex = loopBackIndex;
	return res;
}
code code::dat( instruction c, size_t scriptDataIdx )
{
	code res = code( c );
	res.variableIndex = scriptDataIdx;
	return res;
}