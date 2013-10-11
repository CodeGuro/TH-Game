#pragma once
#include <map>
#include <string>

#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "scriptmachine.hpp"
#include "parser.hpp"

/*manages all script-related things*/
class script_engine
{
private:
	friend class script_machine;
	friend class parser;
	friend struct natives;
	class inventory
	{
	private:
		friend class script_engine;
		vector< script_data > vecScriptData;
		vector< script_environment > vecScriptEnvironment;
		vector< block > vecBlocks;
		vector< script_machine > vecMachines;
		vector< size_t > vecScriptDataGarbage;
		vector< size_t > vecRoutinesGabage;
		vector< size_t > vecMachinesGarbage;
		vector< script_container > vecScripts;
		std::map< std::string, size_t > mappedScripts;
		std::map< std::string, size_t > mappedMainScripts;
		vector< script_queue > vecQueuedScripts;
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
		type_data getArrayType() const;
		type_data getArrayType( size_t element ); //an array of some type
	};
	
	bool error;
	std::string errorMessage;
	inventory battery;
	script_type_manager typeManager;
	parser scriptParser;
	size_t currentRunningMachine;

	size_t fetchBlock();
	block & getBlock( size_t index );
	size_t fetchScriptData();
	size_t fetchScriptData( float real );
	size_t fetchScriptData( char character );
	size_t fetchScriptData( bool boolean );
	size_t fetchScriptData( std::string const & string );
	script_data & getScriptData( size_t index );
	void addRefScriptData( size_t index );
	void scriptDataAssign( size_t & dst, size_t src );
	void copyScriptData( size_t & dst, size_t & src );
	void uniqueizeScriptData( size_t & dst );
	float getRealScriptData( size_t index ) const;
	char getCharacterScriptData( size_t index ) const;
	bool getBooleanScriptData( size_t index ) const;
	std::string getStringScriptData( size_t index );
	void releaseScriptData( size_t & index );
	size_t fetchScriptEnvironment( size_t blockIndex );
	script_environment & getScriptEnvironment( size_t index );
	void addRefScriptEnvironment( size_t index );
	void releaseScriptEnvironment( size_t & index );
	size_t fetchScriptMachine();
	script_machine & getScriptMachine( size_t index );
	void setQueueScriptMachine( script_queue const queue );
	void releaseScriptMachine( size_t & index );
	size_t getBlockFromScript( std::string const & filePath, std::string const & scriptName );
	void registerScript( std::string const scriptName );
	void registerMainScript( std::string const scriptPath, std::string const scriptName );
	void registerInvalidMainScript( std::string const scriptPath );
	script_container * getScript( std::string const & scriptName );
	script_container & getScript( size_t index );
	size_t findScript( std::string const & scriptName );
	size_t findScriptFromFile( std::string const & scriptPath );
	void callSub( size_t machineIndex, script_container::sub AtSub );
	void raiseError( std::string const errorMsg );
public:
	script_engine();
	script_engine( script_engine const & source );
	script_engine & operator = ( script_engine const & source );
	~script_engine();
	void cleanEngine(); //remove all cache
	void start();
	bool advance(); //true if finished (i.e. no script executers left to run)
	void parseScriptFromFile( std::string const & scriptPath );
};