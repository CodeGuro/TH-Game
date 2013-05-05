#pragma once
#include <vector>
#include <string>
#include "defstypedefs.hpp"

enum instruction
{
};

struct type_data
{
	enum type_kind
	{
		tk_real, tk_boolean, tk_char, tk_array
	};
	type_kind kind;
	const type_data * element;
	type_data( type_kind k, type_data * e ) : kind(k), element(e)
	{
	}
	type_kind get_kind() const
	{
		return kind;
	}
	const type_data * get_elementType() const
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
		float real; //also used as a boolean
		size_t objIndex;
	};
	vector< size_t > array;
	script_data() : refCount(0)
	{
	}
	script_data( float real ) : real(real)
	{
	}
	script_data( char character ) : character(character)
	{
	}
	script_data( size_t objIndex ) : objIndex(objIndex)
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
};

struct block
{
	enum block_kind
	{
		bk_normal, bk_loop, bk_function, bk_task,
	};
	vector< code > vecCodes;
	std::string name;
	size_t argc;
	size_t level;
	block_kind kind;
	bool hasResult;
};

struct script_environment
{
	vector< size_t > stack;
};