#pragma once
#include "defstypedefs.hpp"

class script_engine;

/*executes the bytecode*/
class script_machine
{
private:
	vector< size_t > threads;
	size_t current_thread_index;

public:
	void advance( script_engine & engine );
	void initialize( script_engine & engine );
	void clean( script_engine & engine );
};