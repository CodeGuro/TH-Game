#pragma once
#include <vector>
#include <string>
#include "defstypedefs.hpp"

enum instruction
{
	something
};

struct type_data
{
	enum type_kind
	{
		tk_real, tk_boolean, tk_char, tk_array, tk_object, tk_invalid
	};
	type_kind kind;
	size_t element; //offset to the type
	type_data( type_kind k, size_t e ) : kind(k), element(e)
	{
	}
	type_kind get_kind() const
	{
		return kind;
	}
	size_t get_element() const
	{
		return element;
	}
};

struct script_data
{
	size_t refCount;
	type_data type;
	union
	{
		char character;
		float real; // boolean evaluated by checking if it's a nonzero value
		size_t objIndex;
	};
	vector< size_t > vec;
	script_data() : refCount(0), type(type_data::tk_invalid, invalidIndex)
	{
	}
	script_data( float real, size_t elementIndex ) : real(real), type(type_data::tk_real, elementIndex)
	{
	}
	script_data( char character, size_t elementIndex ) : character(character), type(type_data::tk_char, elementIndex)
	{
	}
	script_data( bool boolean, size_t elementIndex ) : real( boolean? 1.f:0.f ), type(type_data::tk_boolean, elementIndex)
	{
	}
	script_data( size_t objIndex, size_t elementIndex ) : objIndex(objIndex), type(type_data::tk_object, elementIndex)
	{
	}
};

struct code
{
	instruction command;
	union
	{
		struct //for assigning data on the virtual stack
		{
			size_t variableIndex;
			size_t variableLevel;
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
				size_t scriptDataIndex;
			};
		};
	};
	static code varLev( instruction c, size_t varIndex, size_t varLevel );
	static code subArg( instruction c, size_t subIndex, size_t subArgc );
	static code loop( instruction c, size_t loopBackIndex );
	static code dat( instruction c, size_t scriptDataIdx );
};

struct block
{
	enum block_kind
	{
		bk_normal, bk_loop, bk_function, bk_task
	};
	vector< code > vecCodes;
	std::string name;
	size_t argc;
	block_kind kind;
	bool has_result;
};

struct script_environment
{
	vector< size_t > stack;
	size_t codeIndex;
	size_t blockIndex;
};