#include <bytecode.hpp>

type_data::type_data() : kind( tk_invalid ), element( invalidIndex )
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