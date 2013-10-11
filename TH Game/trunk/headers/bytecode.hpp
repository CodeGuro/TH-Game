#pragma once
#include <string>
#include <d3d9.h>
#include "defstypedefs.hpp"
#include "ObjMgr.hpp"
enum instruction
{
	//virtual commands
	vc_assign, vc_overWrite, vc_pushVal, vc_pushVar, vc_callFunction, vc_callFunctionPush, vc_callTask,
	vc_loopIfDecr, vc_loopIf, vc_loopBack, vc_yield,
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
	};
	vector< size_t > vec;
	script_data();
	script_data( float real, size_t elementIndex );
	script_data( char character, size_t elementIndex );
	script_data( bool boolean, size_t elementIndex );
	script_data( size_t objIndex, size_t elementIndex );
};

struct code
{
	instruction command;
	union
	{
		struct //for assigning data on a virtual stack
		{
			size_t variableIndex; //in the stack
			size_t variableLevel; // # of routines up from the callee
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
	static code varLev( instruction c, size_t varIndex, size_t levelUp );
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

struct script_container
{
	enum sub
	{
		AtInitialize, AtMainLoop, AtFinalize, AtBackGround
	};
	size_t ScriptBlock;
	size_t InitializeBlock;
	size_t MainLoopBlock;
	size_t FinalizeBlock;
	size_t BackGroundBlock;
};

struct natives
{
private:
	friend class parser;
	static void _add( script_engine * eng, size_t * argv );
	static void _subtract( script_engine * eng, size_t * argv );
	static void _multiply( script_engine * eng, size_t * argv );
	static void _divide( script_engine * eng, size_t * argv );
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
	static void _rand_int( script_engine * eng, size_t * argv );
	static void _print( script_engine * eng, size_t * argv );
	static void _true( script_engine * eng, size_t * argv );
	static void _false( script_engine * eng, size_t * argv );
	static void _increment( script_engine * eng, size_t * argv );
	static void _decrement( script_engine * eng, size_t * argv );
	static void _ToString( script_engine * eng, size_t * argv );
	static void _CreateEnemyFromScript( script_engine * eng, size_t * argv );
	static void _CreateEnemyFromFile( script_engine * eng, size_t * argv );
	static void _QueueScriptTermination( script_engine * eng, size_t * argv );
	static void _Obj_Create( script_engine * eng, size_t * argv );
	static void _Obj_Delete( script_engine * eng, size_t * argv );
	static void _Obj_SetPosition( script_engine * eng, size_t * argv );
	static void _Obj_SetTexture( script_engine * eng, size_t * argv );
	static void _Obj_SetSpeed( script_engine * eng, size_t * argv );
	static void _Obj_SetAcceleration( script_engine * eng, size_t * argv );
	static void _Obj_SetAngle( script_engine * eng, size_t * argv );
	static void _Obj_SetVelocity( script_engine * eng, size_t * argv );
	static void _Obj_CreateVertex( script_engine * eng, size_t * argv );
	static void _Obj_SetPrimitiveType( script_engine * eng, size_t * argv );
	static void _Obj_SetRenderState( script_engine * eng, size_t * argv );
	static void _Obj_SetVertexUV( script_engine * eng, size_t * argv );
	static void _Obj_SetVertexXYZ( script_engine * eng, size_t * argv );
	static void _Obj_SetVertexColor( script_engine * eng, size_t * argv );
	static void _ALPHA_BLEND( script_engine * eng, size_t * argv );
	static void _ADDITIVE_BLEND( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLELIST( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLESTRIP( script_engine * eng, size_t * argv );
	static void _PRIMITIVE_TRIANGLEFAN( script_engine * eng, size_t * argv );
};

struct script_queue
{
	enum scriptQueueType
	{
		Initialization, Termination
	};
	scriptQueueType queueType;
	size_t index;
};