#pragma once
#include "defstypedefs.hpp"

class script_engine;

/*executes the bytecode*/
class script_machine
{
private:
	vector< size_t > threads;
	size_t current_thread_index;
	size_t current_script_index;

public:
	bool advance( script_engine & engine ); //returns 1 when finished
	void initialize( script_engine & engine, size_t script_index );
	void clean( script_engine & engine );
};