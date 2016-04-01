#pragma once
#include <map>
#include <random>
#include <string>
#include "defstypedefs.hpp"
#include "bytecode.hpp"
#include "parser.hpp"
#include "Direct3DEngine.hpp"
#include "FatalException.hpp"
#include "GameProperties.hpp"

class script_engine
{
private:
	friend struct natives;
	friend class script_data_manager;
	friend class parser;

	struct script_context
	{
		vector< size_t > threads;
		size_t current_thread_index;
		size_t current_script_index;
		size_t object_vector_index;
		size_t script_object;
		script_type type_script;
	};

	script_data_manager scriptdata_mgr;
	vector< script_environment > vecScriptEnvironment;
	vector< block > vecBlocks;
	vector< script_context > vecContexts;
	vector< size_t > vecRoutinesGabage;
	vector< vector< size_t > > vvecObjects;
	vector< size_t > vvecObjectsGarbage;
	vector< script_container > vecScripts;
	vector< std::string > vecScriptDirectories;
	std::map< std::string, size_t > mappedScripts;
	std::map< std::string, size_t > mappedMainScripts;
	std::map< std::string, size_t > mappedShotScripts;
	
	bool error;
	bool finished;
	std::string errorMessage;
	size_t currentRunningMachine;

	Direct3DEngine * draw_mgr;
	GameProperties * game_properties;

	std::default_random_engine num_generator;


	void callSub( size_t machineIndex, script_container::sub AtSub );

	bool advance(); //returns 1 when finished
	void initialize_script_context( size_t script_index, size_t context_index, script_type type_script );
	void clean_script_context( size_t context_index );

public:
	script_engine( Direct3DEngine * draw_mgr, GameProperties * properties );
	void cleanEngine(); //remove all cache
	bool start();
	bool run(); //true if finished (i.e. no script executers left to run)
	bool IsFinished();
	void raise_exception( eng_exception const & eng_except );
	void raise_exception( FatalException const & except );
	size_t fetchBlock();
	block & getBlock( size_t index );
	size_t fetchScriptEnvironment( size_t blockIndex );
	script_environment & getScriptEnvironment( size_t index );
	void addRefScriptEnvironment( size_t index );
	void releaseScriptEnvironment( size_t & index );
	size_t fetchScriptContext();
	script_context * getScriptContext( size_t index );
	void releaseScriptContext( size_t & index );
	size_t fetchObjectVector();
	void releaseObjectVector( size_t & index );
	void latchScriptObjectToMachine( size_t index, size_t machineIdx );
	void registerScript( std::string const scriptName );
	void registerMainScript( std::string const scriptPath, std::string const scriptName );
	void registerInvalidMainScript( std::string const scriptPath );
	script_container * getScript( std::string const & scriptName );
	script_container & getScript( size_t index );
	size_t findScript( std::string const & scriptName );
	size_t findScriptFromFile( std::string const & scriptPath );
	size_t findScriptDirectory( std::string const & scriptPath );
	std::string const & getCurrentScriptDirectory( size_t machineIdx ) const;
	Object * getObjFromScriptVector( size_t objvector, size_t Idx );
	void clearOutOfBoundObjects( size_t machineIdx );
	unsigned getContextCount() const;
	Direct3DEngine * get_drawmgr();
	GameProperties * get_game_properties();
};