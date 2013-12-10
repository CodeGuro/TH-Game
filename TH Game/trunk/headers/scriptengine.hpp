#pragma once
#include <map>
#include <string>
#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "scriptmachine.hpp"
#include "parser.hpp"
#include "Direct3DEngine.hpp"

class script_engine : protected virtual Battery, protected virtual inventory, private parser
{
private:
	friend class script_machine;
	friend class parser;
	friend struct natives;
	using inventory::getBlock;
	
	bool error;
	std::string errorMessage;
	size_t currentRunningMachine;

	void callSub( size_t machineIndex, script_container::sub AtSub );
public:
	script_engine();
	void cleanEngine(); //remove all cache
	void start();
	bool advance(); //true if finished (i.e. no script executers left to run)
	void raise_exception( eng_exception const & eng_except );
};