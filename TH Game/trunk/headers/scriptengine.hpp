#pragma once
#include <map>
#include <string>
#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "scriptmachine.hpp"
#include "parser.hpp"
#include "Direct3DEngine.hpp"

struct eng_exception
{
	enum Reason
	{
		eng_error, finalizing_machine
	};
	Reason throw_reason;
	eng_exception();
	eng_exception( Reason const r );
};

class script_engine : protected virtual inventory, private parser
{
private:
	friend class script_machine;
	friend class parser;
	friend struct natives;
	using inventory::getBlock;
	
	bool error;
	std::string errorMessage;
	script_type_manager typeManager;
	size_t currentRunningMachine;

	void callSub( size_t machineIndex, script_container::sub AtSub );
public:
	script_engine();
	script_engine( script_engine const & source );
	script_engine & operator = ( script_engine const & source );
	~script_engine();
	void cleanEngine(); //remove all cache
	void start();
	bool advance(); //true if finished (i.e. no script executers left to run)
	void raise_exception( eng_exception const & eng_except );
};