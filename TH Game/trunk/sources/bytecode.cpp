#include <bytecode.hpp>
#include <scriptengine.hpp>
#include <iostream>
#include <assert.h>

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
	size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real + fmod( (float)rand(), 1 + domain ) );
	eng->scriptDataAssign( argv[0], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_rand_int( script_engine * eng, size_t * argv )
{
	float domain = floor( eng->getScriptData( argv[1] ).real - eng->getScriptData( argv[0] ).real );
	size_t tmp = eng->fetchScriptData( floor( eng->getScriptData( argv[0] ).real ) + fmod( (float)rand(), 1 + domain ) );
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
void natives::_cos( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( cos( eng->getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_sin( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( sin( eng->getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_tan( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( tan( eng->getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_atan( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( atan( eng->getRealScriptData( argv[ 0 ] ) ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_atan2( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( atan2( eng->getRealScriptData( argv[ 0 ] ), eng->getRealScriptData( argv[ 1 ] ) ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
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
void natives::_QueueScriptTermination( script_engine * eng, size_t * argv )
{
	script_queue const queue = { script_queue::Termination, eng->currentRunningMachine };
	eng->setQueueScriptMachine( queue );
}
void natives::_LoadSound( script_engine * eng, size_t * argv )
{
	eng->LoadSound( eng->getStringScriptData( argv[ 0 ] ) );
}
void natives::_PlaySound( script_engine * eng, size_t * argv )
{
	eng->PlaySound( eng->getStringScriptData( argv[ 0 ] ) );
}
void natives::_DeleteSound( script_engine * eng, size_t * argv )
{
	eng->DeleteSound( eng->getStringScriptData( argv[ 0 ] ) );
}
void natives::_Obj_Create( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( (size_t)eng->getRealScriptData( argv[ 0 ] ) );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_Obj_Delete( script_engine * eng, size_t * argv )
{
	unsigned objHandle = eng->getObjHandleScriptData( argv[ 0 ] );
	eng->ReleaseObject( objHandle );
}
void natives::_Obj_BeDeleted( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData( eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) ) == 0 );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}
void natives::_Obj_SetPosition( script_engine * eng, size_t * argv )
{
	Object * obj = eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetPosition( D3DXVECTOR3( eng->getRealScriptData( argv[ 1 ] ), eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetTexture( script_engine * eng, size_t * argv )
{
	ObjMgr * objmgr = eng->GetObjMgr( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( objmgr )
		objmgr->SetTexture( eng->GetTexture( eng->getStringScriptData( argv[ 1 ] ) ) );
}
void natives::_Obj_SetSpeed( script_engine * eng, size_t * argv )
{
	Object * obj = eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetSpeed( eng->getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetAcceleration( script_engine * eng, size_t * argv )
{
	Object * obj = eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetAccel( D3DXVECTOR3( eng->getRealScriptData( argv[ 1 ] ), eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetAngle( script_engine * eng, size_t * argv )
{
	Object * obj = eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetAngle( eng->getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetVelocity( script_engine * eng, size_t * argv )
{
	Object * obj = eng->GetObject( eng->getObjHandleScriptData( argv[ 0 ] ) );
	if( obj )
		obj->SetVelocity( D3DXVECTOR3( eng->getRealScriptData( argv[ 1 ] ), eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_CreateVertex( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_CreateVertex( eng->getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetPrimitiveType( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetPrimitiveType( eng->getObjHandleScriptData( argv[ 0 ] ), eng->getPrimitiveTypeScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetRenderState( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetRenderState( eng->getObjHandleScriptData( argv[ 0 ] ), eng->getBlendModeScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetVertexUV( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetVertexUV( eng->getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->getRealScriptData( argv[ 1 ] ), D3DXVECTOR2( eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetVertexXY( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetVertexXY( eng->getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->getRealScriptData( argv[ 1 ] ), D3DXVECTOR2( eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_Obj_SetVertexColor( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetVertexColor( eng->getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->getRealScriptData( argv[ 1 ] ), D3DCOLOR_RGBA( (ULONG)eng->getRealScriptData( argv[ 2 ] ), (ULONG)eng->getRealScriptData( argv[ 3 ] ), (ULONG)eng->getRealScriptData( argv[ 4 ] ), (ULONG)eng->getRealScriptData( argv[ 5 ] ) ) );
}
void natives::_Obj_SetLayer( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetLayer( eng->getObjHandleScriptData( argv[ 0 ] ), (ULONG)eng->getRealScriptData( argv[ 1 ] ) );
}
void natives::_Obj_SetScale( script_engine * eng, size_t * argv )
{
	eng->ObjEffect_SetScale( eng->getObjHandleScriptData( argv[ 0 ] ), D3DXVECTOR3( eng->getRealScriptData( argv[ 1 ] ), eng->getRealScriptData( argv[ 2 ] ), eng->getRealScriptData( argv[ 3 ] ) ) );
}
void natives::_ALPHA_BLEND( script_engine * eng, size_t * argv )
{
	assert( argv[ 0 ] == invalidIndex );
	argv[ 0 ] = eng->fetchScriptData( BlendAlpha );
}
void natives::_ADDITIVE_BLEND( script_engine * eng, size_t * argv )
{
	assert( argv[ 0 ] == invalidIndex );
	argv[ 0 ] = eng->fetchScriptData( BlendAdd );
}
void natives::_PRIMITIVE_TRIANGLELIST( script_engine * eng, size_t * argv )
{
	assert( argv[ 0 ] == invalidIndex );
	argv[ 0 ] = eng->fetchScriptData( D3DPT_TRIANGLELIST );
}	
void natives::_PRIMITIVE_TRIANGLESTRIP( script_engine * eng, size_t * argv )
{
	assert( argv[ 0 ] == invalidIndex );
	argv[ 0 ] = eng->fetchScriptData( D3DPT_TRIANGLESTRIP );
}
void natives::_PRIMITIVE_TRIANGLEFAN( script_engine * eng, size_t * argv )
{
	assert( argv[ 0 ] == invalidIndex );
	argv[ 0 ] = eng->fetchScriptData( D3DPT_TRIANGLEFAN );
}
void natives::_LoadTexture( script_engine * eng, size_t * argv )
{
	eng->LoadTexture( eng->getStringScriptData( argv[ 0 ] ) );
}
void natives::_LoadUserShotData( script_engine * eng, size_t * argv )
{
	std::string scriptPath = eng->getStringScriptData( argv[ 0 ] );
	eng->parseShotScript( scriptPath );
}
void natives::_CreateShot01( script_engine * eng, size_t * argv )
{
	size_t tmp = eng->fetchScriptData();
	eng->getScriptData( tmp ).type = eng->getObjectType();
	eng->getScriptData( tmp ).objIndex = eng->CreateShot01( D3DXVECTOR2( eng->getRealScriptData( argv[ 0 ] ), eng->getRealScriptData( argv[ 1 ] ) ), eng->getRealScriptData( argv[ 2 ] ),
		eng->getRealScriptData( argv[ 3 ] ), eng->getRealScriptData( argv[ 4 ] )  );
	eng->scriptDataAssign( argv[ 0 ], tmp );
	eng->releaseScriptData( tmp );
}