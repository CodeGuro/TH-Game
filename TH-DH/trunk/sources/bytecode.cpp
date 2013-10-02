#include <bytecode.hpp>
#include <scriptengine.hpp>
#include <iostream>

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

void natives::_add( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real + eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_subtract( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real - eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_multiply( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real * eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_divide( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real / eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_negative( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( -(eng->getScriptData( argv[0] ).real) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_power( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( pow( eng->getScriptData( argv[0] ).real, eng->getScriptData( argv[1] ).real ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_concatenate( script_engine * eng, size_t * argv )
{
	eng->uniqueizeScriptData( argv[0] );
	unsigned s = eng->getScriptData( argv[1] ).vec.size();
	for( unsigned i = 0; i < s; ++i )
	{
		size_t idx = invalidIndex;
		eng->scriptDataAssign( idx, eng->getScriptData( argv[1] ).vec[i] );
		eng->getScriptData( argv[0] ).vec.push_back( idx );
	}
}
void natives::_absolute( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( abs( eng->getScriptData( argv[0] ).real ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_not( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( !(eng->getScriptData( argv[0] ).real) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real == eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareNotEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real != eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareGreater( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real > eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareGreaterEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real >= eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareLess( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real < eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_compareLessEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real <= eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_logicOr( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real || eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_logicAnd( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real && eng->getScriptData( argv[1] ).real );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_roof( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( ceil( eng->getScriptData( argv[0] ).real ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_floor( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( floor( eng->getScriptData( argv[0] ).real ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_index( script_engine * eng, size_t * argv )
{
	eng->scriptDataAssign( argv[0], eng->getScriptData( argv[0] ).vec[ (unsigned)eng->getScriptData( argv[1] ).real ] );
}
void natives::_appendArray( script_engine * eng, size_t * argv )
{
	size_t idx = invalidIndex;
	eng->scriptDataAssign( idx, argv[1] );
	eng->getScriptData( argv[0] ).vec.push_back( idx );
}
void natives::_uniqueize( script_engine * eng, size_t * argv )
{
	eng->uniqueizeScriptData( argv[0] );
}
void natives::_rand( script_engine * eng, size_t * argv )
{
	float domain = eng->getScriptData( argv[1] ).real - eng->getScriptData( argv[0] ).real;
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real + fmod( (float)rand() + 1.f, domain ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_rand_int( script_engine * eng, size_t * argv )
{
	float domain = floor( eng->getScriptData( argv[1] ).real - eng->getScriptData( argv[0] ).real );
	size_t tmp = eng->fetchScriptData( floor( eng->getScriptData( argv[0] ).real ) + fmod( (float)rand() + 1.f, domain ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_print( script_engine * eng, size_t * argv )
{
	std::string str = eng->getStringScriptData( argv[0] );
	std::cout << str << std::endl;
}
void natives::_true( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( true );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_false( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( false );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_increment( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( 1.f + eng->getRealScriptData( argv[0] ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );	
}
void natives::_decrement( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( -1.f + eng->getRealScriptData( argv[0] ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );	
}
void natives::_ToString( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->getStringScriptData( argv[0] ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_CreateEnemyFromScript( script_engine * eng, size_t * argv )
{
	size_t scriptIndex = eng->findScript( eng->getStringScriptData( argv[ 0 ] ) );
	if( scriptIndex != invalidIndex )
	{
		script_queue sq = { script_queue::Initialization, scriptIndex };
		eng->setQueueScriptMachine( sq );
	}
}
void natives::_CreateEnemyFromFile( script_engine * eng, size_t * argv )
{
	std::string scriptPath = eng->getStringScriptData( argv[ 0 ] );
	eng->parseScriptFromFile( scriptPath );
	size_t scriptIndex;
	if( (scriptIndex = eng->findScriptFromFile( scriptPath )) != invalidIndex )
	{
		script_queue sq = { script_queue::Initialization, scriptIndex };
		eng->setQueueScriptMachine( sq );
	}
}