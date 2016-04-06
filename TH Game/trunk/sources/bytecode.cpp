#include <bytecode.hpp>
#include <scriptengine.hpp>
#include <iostream>
#include <random>
#include <assert.h>

type_data::type_data() : kind( tk_invalid ), element( -1 )
{
}
type_data::type_data( type_kind k, size_t e ) : kind( k ), element( e )
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

script_data::script_data() : type( type_data::tk_invalid, -1 )
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
script_data::script_data( ObjType typeobj, size_t elementIndex ) : objtype(typeobj), type( type_data::tk_misc, elementIndex )
{
}

code::code() : command( vc_invalid )
{
}
code::code( instruction c ) : command( c )
{
}
code code::varSub( instruction c, size_t varIndex, size_t subIndex )
{
	code res = code( c );
	res.variableIndex = varIndex;
	res.blockIndex = subIndex;
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

script_data_manager::script_data_manager( Direct3DEngine * draw_mgr, script_engine * eng ) : draw_mgr( draw_mgr ), eng( eng )
{
}

void natives::_add( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real + eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_subtract( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real - eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_multiply( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real * eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_divide( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real / eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_modulus( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( fmod( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real, eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_negative( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( -(eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_power( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( pow( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real, eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_concatenate( script_engine * eng, size_t * argv )
{
	eng->scriptdata_mgr.uniqueizeScriptData( argv[ 0 ] );
	unsigned s = eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).vec.size();
	for( unsigned i = 0; i < s; ++i )
	{
		size_t idx = -1;
		eng->scriptdata_mgr.scriptDataAssign( idx, eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).vec[ i ] );
		eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).vec.push_back( idx );
	}
}
void natives::_absolute( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( abs( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_not( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( !(eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real == eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareNotEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real != eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareGreater( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real > eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareGreaterEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real >= eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareLess( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real < eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_compareLessEqual( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real <= eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_logicOr( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real || eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_logicAnd( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real && eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_roof( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ceil( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_floor( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( floor( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).real ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_index( script_engine * eng, size_t * argv )
{
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).vec[ (unsigned)eng->scriptdata_mgr.getScriptData( argv[ 1 ] ).real ] );
}
void natives::_appendArray( script_engine * eng, size_t * argv )
{
	eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).vec.push_back( -1 );
	eng->scriptdata_mgr.scriptDataAssign( eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).vec.back(), argv[ 1 ] );
}
void natives::_uniqueize( script_engine * eng, size_t * argv )
{
	eng->scriptdata_mgr.uniqueizeScriptData( argv[ 0 ] );
}
void natives::_rand( script_engine * eng, size_t * argv )
{
	std::uniform_real_distribution< float > rand_real( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( rand_real( eng->num_generator ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_rand_int( script_engine * eng, size_t * argv )
{
	std::uniform_int_distribution< int > rand_int(
		(int)eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), (int)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( (float)rand_int( eng->num_generator ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_print( script_engine * eng, size_t * argv )
{
	std::string str = eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] );
	std::cout << str << std::endl;
}
void natives::_true( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( true );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_false( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( false );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_PI( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( D3DX_PI );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_OBJ_SHOT( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ObjShot );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_OBJ_EFFECT( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ObjEffect );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_OBJ_PLAYER( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ObjPlayer );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_OBJ_FONT( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ObjFont );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_increment( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( 1.f + eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );	
}
void natives::_decrement( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( -1.f + eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );	
}
void natives::_ToString( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_cos( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( cos( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_sin( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( sin( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_tan( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( tan( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_atan( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( atan( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_atan2( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( atan2( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_length( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( (float)eng->scriptdata_mgr.getScriptData( argv[ 0 ] ).vec.size() );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_KeyDown( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( (GetKeyState( (int)eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] )) & 0x8000 ) != 0 );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_KeyToggled( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( (GetKeyState( (int)eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] )) & 0x1 ) != 0 );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_KeyPressed( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( GetAsyncKeyState( (int)eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] )) == -32767 );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_CreateEnemyFromScript( script_engine * eng, size_t * argv )
{
	size_t scriptIndex = eng->findScript( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
	if( CheckValidIdx( scriptIndex ) )
	{
		size_t new_machine = eng->fetchScriptContext();
		eng->initialize_script_context( new_machine, scriptIndex, script_type::enemy_script, argv[ 1 ] );
	}
}
void natives::_CreateEnemyFromFile( script_engine * eng, size_t * argv )
{
	std::string scriptPath = eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] );
	parser p( eng );
	p.parseScript( scriptPath );
	size_t scriptIndex;
	if( CheckValidIdx( (scriptIndex = eng->findScriptFromFile( scriptPath )) ) )
	{
		size_t new_machine = eng->fetchScriptContext();
		eng->initialize_script_context( new_machine, scriptIndex, script_type::enemy_script, argv[ 1 ] );
	}
}
void natives::_TerminateScript( script_engine * eng, size_t * argv )
{
	eng->removing_machine = true;
	eng->finishAllThreads( eng->currentRunningMachine );
}
void natives::_GetCurrentScriptDirectory( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->getCurrentScriptDirectory( eng->currentRunningMachine ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_LoadSound( script_engine * eng, size_t * argv )
{
 	eng->get_drawmgr()->LoadSound( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
}
void natives::_PlaySound( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->PlaySound( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ), eng->game_properties->get_volume() );
}
void natives::_SetSoundVolume( script_engine * eng, size_t * argv )
{
	eng->game_properties->set_volume( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) );
}
void natives::_StopSound( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->StopSound( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
}
void natives::_DeleteSound( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->DeleteSound( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
}
void natives::_GetScore( script_engine * eng, size_t *argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( (float)eng->get_game_properties()->get_points() );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_SetScore( script_engine * eng, size_t * argv )
{
	eng->get_game_properties()->set_points( (size_t)eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ) );
}
void natives::_Obj_Create( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->scriptdata_mgr.getObjTypeScriptData( argv[ 0 ] ), eng->currentRunningMachine );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_Obj_Delete( script_engine * eng, size_t * argv )
{
	unsigned objHandle = eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] );
	eng->get_drawmgr()->DestroyObject( objHandle );
}
void natives::_Obj_BeDeleted( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) == 0 );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_Obj_SetPosition( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetPosition( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), 0.0f ) );
}
void natives::_Obj_SetPosition3D( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ))
		obj->SetPosition( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetSpeed( script_engine * eng, size_t * argv )
{
	Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetSpeed( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetAcceleration( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetAccel( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetAngle( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetAngle( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetVelocity( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetVelocity( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetAutoDelete( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->FlagScreenDeletable( (int)eng->scriptdata_mgr.getBooleanScriptData( argv[ 1 ] ) );
}
void natives::_Obj_ScriptLatch( script_engine * eng, size_t * argv )
{
	eng->latchScriptObjectToMachine( argv[ 0 ], eng->currentRunningMachine );
}
void natives::_Obj_GetX( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		size_t tmp = eng->scriptdata_mgr.fetchScriptData( obj->position.x );
		eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
		eng->scriptdata_mgr.releaseScriptData( tmp );
	}
}
void natives::_Obj_GetY( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		size_t tmp = eng->scriptdata_mgr.fetchScriptData( obj->position.y );
		eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
		eng->scriptdata_mgr.releaseScriptData( tmp );
	}
}
void natives::_Obj_GetZ( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		size_t tmp = eng->scriptdata_mgr.fetchScriptData( obj->position.z );
		eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
		eng->scriptdata_mgr.releaseScriptData( tmp );
	}
}
void natives::_Obj_GetSpeed( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		size_t tmp = eng->scriptdata_mgr.fetchScriptData( obj->GetSpeed() );
		eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
		eng->scriptdata_mgr.releaseScriptData( tmp );
	}
}
void natives::_Obj_GetAngle( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		size_t tmp = eng->scriptdata_mgr.fetchScriptData( obj->GetAngle() );
		eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
		eng->scriptdata_mgr.releaseScriptData( tmp );
	}
}
void natives::_ObjEffect_SetTexture( script_engine * eng, size_t * argv )
{
	if( ObjMgr * objmgr = eng->get_drawmgr()->GetObjMgr( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		objmgr->pTexture = eng->get_drawmgr()->GetTexture( eng->scriptdata_mgr.getStringScriptData( argv[ 1 ] ) );
}
void natives::_ObjEffect_CreateVertex( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_CreateVertex( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjEffect_SetPrimitiveType( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetPrimitiveType( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getPrimitiveTypeScriptData( argv[ 1 ] ) );
}
void natives::_ObjEffect_SetRenderState( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetRenderState( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getBlendModeScriptData( argv[ 1 ] ) );
}
void natives::_ObjEffect_SetVertexUV( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetVertexUV( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), D3DXVECTOR2( eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_ObjEffect_SetVertexXY( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetVertexXYZ( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), 0.f ) );
}
void natives::_ObjEffect_SetVertexXYZ( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetVertexXYZ( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) ) );
}
void natives::_ObjEffect_SetVertexColor( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetVertexColor( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), D3DCOLOR_RGBA( (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 5 ] ) ) );
}
void natives::_ObjEffect_SetLayer( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjEffect_SetLayer( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjEffect_SetScale( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetScale( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), 1.f ) );
}
void natives::_ObjEffect_SetScale3D( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetScale( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_ObjEffect_SetRotationAxis( script_engine * eng, size_t * argv )
{
	if( Object * obj = eng->get_drawmgr()->GetObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetRotationEx( D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjShot_SetGraphic( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjShot_SetGraphic( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjFont_SetRect( script_engine * eng, size_t * argv )
{
	if( FontObject * obj = eng->get_drawmgr()->GetFontObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
	{
		RECT r = { (LONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), (LONG)eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ),
			(LONG)eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), (LONG)eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) };
		obj->Rect = r;
	}
}
void natives::_ObjFont_SetString( script_engine * eng, size_t * argv )
{
	if( FontObject * obj = eng->get_drawmgr()->GetFontObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->String = eng->scriptdata_mgr.getStringScriptData( argv[ 1 ] );
}
void natives::_ObjFont_SetColor( script_engine * eng, size_t * argv )
{
	if( FontObject * obj = eng->get_drawmgr()->GetFontObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->Color = D3DCOLOR_RGBA( (UINT)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), (UINT)eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ),
			(UINT)eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), (UINT)eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) );
}
void natives::_ObjFont_SetSize( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjFont_SetSize( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjFont_SetFaceName( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->ObjFont_SetFaceName( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getStringScriptData( argv[ 1 ] ) );
}
void natives::_ObjFont_SetAlignmentX( script_engine * eng, size_t * argv )
{
	if( FontObject * obj = eng->get_drawmgr()->GetFontObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetAlignmentX( (int)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ObjFont_SetAlignmentY( script_engine * eng, size_t * argv )
{
	if( FontObject * obj = eng->get_drawmgr()->GetFontObject( eng->scriptdata_mgr.getObjHandleScriptData( argv[ 0 ] ) ) )
		obj->SetAlignmentY( (int)eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}
void natives::_ALPHA_BLEND( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	argv[ 0 ] = eng->scriptdata_mgr.fetchScriptData( BlendAlpha );
}
void natives::_ADDITIVE_BLEND( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	argv[ 0 ] = eng->scriptdata_mgr.fetchScriptData( BlendAdd );
}
void natives::_PRIMITIVE_TRIANGLELIST( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	argv[ 0 ] = eng->scriptdata_mgr.fetchScriptData( D3DPT_TRIANGLELIST );
}	
void natives::_PRIMITIVE_TRIANGLESTRIP( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	argv[ 0 ] = eng->scriptdata_mgr.fetchScriptData( D3DPT_TRIANGLESTRIP );
}
void natives::_PRIMITIVE_TRIANGLEFAN( script_engine * eng, size_t * argv )
{
	assert( !CheckValidIdx( argv[ 0 ] ) );
	argv[ 0 ] = eng->scriptdata_mgr.fetchScriptData( D3DPT_TRIANGLEFAN );
}
void natives::_LoadTexture( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->LoadTexture( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
}
void natives::_DeleteTexture( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->DeleteTexture( eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] ) );
}
void natives::_LoadUserShotData( script_engine * eng, size_t * argv )
{
	parser p( eng );
	std::string scriptPath = eng->scriptdata_mgr.getStringScriptData( argv[ 0 ] );
	p.parseShotScript( scriptPath );
}
void natives::_CreateShot01( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->scriptdata_mgr.fetchScriptData( ObjShot, eng->currentRunningMachine );
	unsigned objHandle = eng->scriptdata_mgr.getObjHandleScriptData( tmp );
	eng->get_drawmgr()->ObjShot_SetGraphic( objHandle, (ULONG)eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) );
	Object * obj = eng->get_drawmgr()->GetObject( objHandle );
	obj->position = D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), 0.f );
	obj->SetSpeed( eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ) );
	obj->SetAngle( eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ) );
	eng->scriptdata_mgr.scriptDataAssign( argv[ 0 ], tmp );
	eng->scriptdata_mgr.releaseScriptData( tmp );
}
void natives::_TerminateProgram( script_engine * eng, size_t * argv )
{
	eng->finished = true;
	for( size_t machineIdx = 0; machineIdx < eng->vecContexts.size(); ++machineIdx )
		eng->finishAllThreads( machineIdx );
}
void natives::_SetEyeView( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->SetLookAtViewMatrix
		(
			D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ) ),
			D3DXVECTOR3( eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 5 ] ) )
		);
}
void natives::_SetFog( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->SetFog
		(
			eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ), eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ),
			eng->scriptdata_mgr.getRealScriptData( argv[ 2 ] ),
			eng->scriptdata_mgr.getRealScriptData( argv[ 3 ] ),
			eng->scriptdata_mgr.getRealScriptData( argv[ 4 ] ) 
		);
}
void natives::_SetPerspectiveClip( script_engine * eng, size_t * argv )
{
	eng->get_drawmgr()->SetPerspectiveClip( eng->scriptdata_mgr.getRealScriptData( argv[ 0 ] ),
		eng->scriptdata_mgr.getRealScriptData( argv[ 1 ] ) );
}