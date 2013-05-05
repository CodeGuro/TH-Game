#pragma once
#include <vector>
#include <map>
#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "scriptmachine.hpp"

class script_engine
{
private:
	friend class script_machine;
	class inventory
	{
	private:
		friend class script_engine;
		vector< block > vecBlocks;
		vector< script_data > vecScriptData;
		vector< script_environment > vecRoutines;
		vector< script_machine > vecMachines;
		vector< size_t > vecScriptDataGarbage;
		vector< size_t > vecRoutinesGabage;
		vector< size_t > vecMachinesGarbage;
	};
	inventory battery;
	size_t fetchBlock();
	block & getBlock( size_t index );
	size_t fetchScriptData();
	size_t fetchScriptData( float real );
	size_t fetchScriptData( char character );
	size_t fetchScriptData( std::string const & string );
	size_t fetchScriptData( size_t objIndex );
	script_data & getScriptData( size_t index );
	void disposeScriptData( size_t index );
	size_t fetchScriptEnvironment();
	script_environment & getScriptEnvironment( size_t index );
	void disposeScriptEnvironment( size_t index );
	size_t fetchScriptMachine();
	script_machine & getScriptMachine( size_t index );
	void disposeScriptMachine( size_t index );

	void assignScriptData(size_t dst, size_t src);
	void clean();
};