#pragma once
#include <string>
#include <d3d9.h>
#include "defstypedefs.hpp"
#include "ObjMgr.hpp"

enum instruction
{
	//virtual commands
	vc_assign, vc_overWrite, vc_pushVal, vc_pushVar, vc_duplicate, vc_callFunction, vc_callFunctionPush, vc_callTask,
	vc_loopIf, vc_loopBack, vc_loopAscent, vc_loopDescent, vc_yield,
	vc_checkIf, vc_caseBegin, vc_caseNext, vc_caseEnd, vc_gotoEnd,
	vc_breakRoutine, vc_breakLoop,
	vc_invalid, 
};

struct type_data
{
	enum type_kind
	{
		tk_real, tk_boolean, tk_char, tk_array, tk_object, tk_misc, tk_invalid
	};
	type_kind kind;
	size_t element; //vector index
	type_data();
	type_data( type_kind k, size_t e );
	type_kind get_kind() const;
	size_t get_element() const;
};

class script_type_manager
{
private:
	vector< type_data > types;
public:
	script_type_manager();
	type_data getRealType() const;
	type_data getBooleanType() const;
	type_data getCharacterType() const;
	type_data getStringType() const;
	type_data getObjectType() const;
	type_data getMiscType() const;
	type_data getArrayType() const;
	type_data getArrayType( size_t element ); //an array of some type
};

struct script_data
{
	size_t refCount;
	type_data type;
	union
	{
		char character;
		float real; // boolean evaluated by checking if it's a nonzero value
		unsigned objIndex;
		D3DPRIMITIVETYPE primitiveType;
		BlendType blendMode;
		ObjType objtype;
	};
	vector< size_t > vec;
	script_data();
	script_data( float real, size_t elementIndex );
	script_data( char character, size_t elementIndex );
	script_data( bool boolean, size_t elementIndex );
	script_data( size_t objIndex, size_t elementIndex );
	script_data( ObjType typeobj, size_t elementIndex );
};

class script_data_manager
{
private:
	friend class parser;
	friend struct natives;
	vector< script_data > vecScriptData;
	vector< size_t > vecScriptDataGarbage;
	script_type_manager type_mgr;
	class Direct3DEngine * draw_mgr;
	class script_engine * eng;
public:
	script_data_manager( Direct3DEngine * draw_mgr, script_engine * eng );
	size_t fetchScriptData();
	size_t fetchScriptData( float real );
	size_t fetchScriptData( char character );
	size_t fetchScriptData( bool boolean );
	size_t fetchScriptData( std::string const & string );
	size_t fetchScriptData( ObjType typeobj, size_t MachineIdx );
	size_t fetchScriptData( D3DPRIMITIVETYPE primType );
	size_t fetchScriptData( BlendType blend );
	size_t fetchScriptData( ObjType typeobj );
	script_data & getScriptData( size_t index );
	void addRefScriptData( size_t index );
	void scriptDataAssign( size_t & dst, size_t src );
	void copyScriptData( size_t & dst, size_t & src );
	void uniqueizeScriptData( size_t & dst );
	float getRealScriptData( size_t index ) const;
	char getCharacterScriptData( size_t index ) const;
	bool getBooleanScriptData( size_t index ) const;
	unsigned getObjHandleScriptData( size_t index ) const;
	D3DPRIMITIVETYPE getPrimitiveTypeScriptData( size_t index ) const;
	BlendType getBlendModeScriptData( size_t index ) const;
	ObjType getObjTypeScriptData( size_t index ) const;
	std::string getStringScriptData( size_t index );
	void releaseScriptData( size_t & index );
};

struct code
{
	instruction command;
	union
	{
		struct //for assigning data on a virtual stack
		{
			size_t variableIndex; //in the stack
			size_t blockIndex; // # of routines up from the callee
		};
		struct //for calling routines
		{
			size_t subIndex;
			size_t argc;
		};
		union
		{
			struct //for looping instructions
			{
				size_t loopBackIndex;
			};
			struct //for immediate assignment (like push)
			{
				size_t scriptDataIndex; //in the engine
			};
		};
	};
	code();
	code( instruction c );
	//named constructor idom
	static code varSub( instruction c, size_t varIndex, size_t subIndex );
	static code subArg( instruction c, size_t subIndex, size_t subArgc );
	static code loop( instruction c, size_t loopBackIndex );
	static code dat( instruction c, size_t scriptDataIdx );
};

class script_engine;

struct block
{
	enum block_kind
	{
		bk_normal, bk_loop, bk_function, bk_task, bk_sub
	};
	vector< code > vecCodes;
	void (*nativeCallBack)( script_engine * eng, size_t * args);
	std::string name;
	size_t argc;
	block_kind kind;
};

struct script_environment
{
	vector< size_t > stack;
	vector< size_t > values;
	size_t codeIndex;
	size_t blockIndex;
	size_t parentIndex;
	size_t refCount;
	bool hasResult;
};

enum script_type
{
	menu_script, stage_script, enemy_script, player_script, other_script
};

struct script_container
{
	enum sub
	{
		AtInitialize, AtMainLoop, AtFinalize, AtBackGround, AtHit
	};
	size_t ScriptBlock;
	size_t InitializeBlock;
	size_t MainLoopBlock;
	size_t FinalizeBlock;
	size_t BackGroundBlock;
	size_t HitBlock;
	size_t ScriptDirectory;
};

struct natives
{
private:
	friend class parser;
	static void _add( script_engine * eng, size_t * argv );
	static void _subtract( script_engine * eng, size_t * argv );
	static void _multiply( script_engine * eng, size_t * argv );
	static void _divide( script_engine * eng, size_t * argv );
	static void _modulus( script_engine * eng, size_t * argv );
	static void _negative( script_engine * eng, size_t * argv );
	static void _power( script_engine * eng, size_t * argv );
	static void _concatenate( script_engine * eng, size_t * argv );
	static void _absolute( script_engine * eng, size_t * argv );
	static void _not( script_engine * eng, size_t * argv );
	static void _compareEqual( script_engine * eng, size_t * argv );
	static void _compareNotEqual( script_engine * eng, size_t * argv );
	static void _compareGreater( script_engine * eng, size_t * argv );
	static void _compareGreaterEqual( script_engine * eng, size_t * argv );
	static void _compareLess( script_engine * eng, size_t * argv );
	static void _compareLessEqual( script_engine * eng, size_t * argv );
	static void _logicOr( script_engine * eng, size_t * argv );
	static void _logicAnd( script_engine * eng, size_t * argv );
	static void _roof( script_engine * eng, size_t * argv );
	static void _floor( script_engine * eng, size_t * argv );
	static void _index( script_engine * eng, size_t * argv );
	static void _appendArray( script_engine * eng, size_t * argv );
	static void _uniqueize( script_engine * eng, size_t * argv );
	static void _rand( script_engine * eng, size_t * argv );
	static void _rand_norm( script_engine * eng, size_t * argv );
	static void _rand_int( script_engine * eng, size_t * argv );
	static void _print( script_engine * eng, size_t * argv );
	static void _true( script_engine * eng, size_t * argv );
	static void _false( script_engine * eng, size_t * argv );
	static void _PI( script_engine * eng, size_t * argv );
	static void _OBJ_SHOT( script_engine * eng, size_t * argv );
	static void _OBJ_EFFECT( script_engine * eng, size_t * argv );
	static void _OBJ_PLAYER( script_engine * eng, size_t * argv );
	static void _OBJ_FONT( script_engine * eng, size_t * argv );
	static void _increment( script_engine * eng, size_t * argv );
	static void _decrement( script_engine * eng, size_t * argv );
	static void _ToString( script_engine * eng, size_t * argv );
	static void _cos( script_engine * eng, size_t * argv );
	static void _sin( script_engine * eng, size_t * argv );
	static void _tan( script_engine * eng, size_t * argv );
	static void _atan( script_engine * eng, size_t * argv );
	static void _atan2( script_engine * eng, size_t * argv );
	static void _length( script_engine * eng, size_t * argv );
	static void _rotatePoint( script_engine * eng, size_t * argv );
	static void _KeyDown( script_engine * eng, size_t * argv );
	static void _KeyToggled( script_engine * eng, size_t * argv );
	static void _KeyPressed( script_engine * eng, size_t * argv );
	static void _CreateEnemyFromScript( script_engine * eng, size_t * argv );
	static void _CreateEnemyFromFile( script_engine * eng, size_t * argv );
	static void _LaunchScriptFromName( script_engine * eng, size_t * argv );
	static void _LaunchScriptFromFile( script_engine * eng, size_t * argv );
	static void _TerminateScript( script_engine * eng, size_t * argv );
	static void _GetScriptArgument( script_engine * eng, size_t * argv );
	static void _GetCurrentScriptDirectory( script_engine * eng, size_t * argv );
	static void _LoadSound( script_engine * eng, size_t * argv );
	static void _PlaySound( script_engine * eng, size_t * argv );
	static void _SetSoundVolume( script_engine * eng, size_t * argv );
	static void _StopSound( script_engine * eng, size_t * argv );
	static void _DeleteSound( script_engine * eng, size_t * argv );
	static void _GetScore( script_engine * eng, size_t * argv );
	static void _SetScore( script_engine * eng, size_t * argv );
	static void _Obj_Create( script_engine * eng, size_t * argv );
	static void _Obj_Delete( script_engine * eng, size_t * argv );
	static void _Obj_BeDeleted( script_engine * eng, size_t * argv );
	static void _Obj_SetPosition( script_engine * eng, size_t * argv );
	static void _Obj_SetPosition3D( script_engine * eng, size_t * argv );
	static void _Obj_SetSpeed( script_engine * eng, size_t * argv );
	static void _Obj_SetAcceleration( script_engine * eng, size_t * argv );
	static void _Obj_SetAngle( script_engine * eng, size_t * argv );
	static void _Obj_SetVelocity( script_engine * eng, size_t * argv );
	static void _Obj_SetAutoDelete( script_engine * eng, size_t * argv );
	static void _Obj_ScriptLatch( script_engine * eng, size_t * argv );
	static void _Obj_GetX( script_engine * eng, size_t * argv );
	static void _Obj_GetY( script_engine * eng, size_t * argv );
	static void _Obj_GetZ( script_engine * eng, size_t * argv );
	static void _Obj_GetSpeed( script_engine * eng, size_t * argv );
	static void _Obj_GetAngle( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetTexture( script_engine * eng, size_t * argv );
	static void _ObjEffect_CreateVertex( script_engine * eng, size_t * argv );
	static void _ObjEffect_GetVertexCount( script_engine *eng, size_t * argv );
	static void _ObjEffect_SetPrimitiveType( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetRenderState( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetVertexUV( script_engine * eng, size_t * argv );
	static void _ObjEffect_GetVertexUV( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetVertexXY( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetVertexXYZ( script_engine * eng, size_t * argv );
	static void _ObjEffect_GetVertexXYZ( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetVertexColor( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetLayer( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetScale( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetScale3D( script_engine * eng, size_t * argv );
	static void _ObjEffect_SetRotationAxis( script_engine * eng, size_t * argv );
	static void _ObjShot_SetGraphic( script_engine * eng, size_t * argv );
	static void _ObjFont_SetRect( script_engine * eng, size_t * argv );
	static void _ObjFont_SetString( script_engine * eng, size_t * argv );
	static void _ObjFont_SetColor( script_engine * eng, size_t * argv );
	static void _ObjFont_SetSize( script_engine * eng, size_t * argv );
	static void _ObjFont_SetFaceName( script_engine * eng, size_t * argv );
	static void _ObjFont_SetAlignmentX( script_engine * eng, size_t * argv );
	static void _ObjFont_SetAlignmentY( script_engine * eng, size_t * argv );
	static void _ALPHA_BLEND( script_engine * eng, size_t * argv );
	static void _ADDITIVE_BLEND( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLELIST( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLESTRIP( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLEFAN( script_engine * eng, size_t * argv );
	static void _LoadTexture( script_engine * eng, size_t * argv );
	static void _DeleteTexture( script_engine * eng, size_t * argv );
	static void _LoadUserShotData( script_engine * eng, size_t * argv );
	static void _CreateShot01( script_engine * eng, size_t * argv );
	static void _TerminateProgram( script_engine * eng, size_t * argv );
	static void _SetEyeView( script_engine * eng, size_t *argv );
	static void _SetFog( script_engine * eng, size_t * argv );
	static void _SetPerspectiveClip( script_engine * eng, size_t *argv );
};

struct eng_exception
{
	enum Reason
	{
		invalid_reason
	};
	Reason throw_reason;
	std::string const Str;
	eng_exception();
	eng_exception( Reason const r );
	eng_exception( Reason const r, std::string const & String );
};