#pragma once
#include <vector>
#include <map>
#include <string>
#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "scriptmachine.hpp"

/*manages all script-related things*/
class script_engine
{
private:
	friend class ::script_machine;
	class inventory
	{
	private:
		friend class script_engine;
		vector< script_data > vecScriptData;
		vector< script_environment > vecRoutines;
		vector< block > vecBlocks;
		vector< script_machine > vecMachines;
		vector< size_t > vecScriptDataGarbage;
		vector< size_t > vecRoutinesGabage;
		vector< size_t > vecMachinesGarbage;
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
		type_data getArrayType( size_t element ); //an array of some type
	};

	inventory battery;
	script_type_manager typeManager;
	size_t currentRunningMachine;

	size_t fetchBlock();
	block & getBlock( size_t index );
	size_t fetchScriptData();
	size_t fetchScriptData( float real );
	size_t fetchScriptData( char character );
	size_t fetchScriptData( bool boolean );
	size_t fetchScriptData( std::string const & string );
	size_t fetchScriptData( size_t objIndex );
	script_data & getScriptData( size_t index );
	void addRefScriptData( size_t & index );
	void releaseScriptData( size_t & index );
	void scriptDataAssign( size_t & dst, size_t & src );
	void copyScriptData( size_t & dst, size_t & src );
	void uniqueize( size_t & dst );
	void disposeScriptData( size_t index );
	size_t fetchScriptEnvironment( size_t blockIndex );
	script_environment & getScriptEnvironment( size_t index );
	void disposeScriptEnvironment( size_t index );
	size_t fetchScriptMachine();
	script_machine & getScriptMachine( size_t index );
	void terminateMachine( size_t index ); //Finalize runs
	void terminateImmediateMachine( size_t index ); //Finalize doesn't run (usually for off-screen termination)
	void disposeScriptMachine( size_t index );

public:
	script_engine();
	script_engine( script_engine const & source );
	script_engine & operator = ( script_engine const & source );
	~script_engine();
	void cleanEngine();
};