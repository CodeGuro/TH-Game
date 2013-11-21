#include <scriptengine.hpp>
#include <parser.hpp>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <Windows.h>

//script engine - public functions, called from the outside
script_engine::script_engine() : error( false ), currentRunningMachine( invalidIndex )
{
}
void script_engine::cleanEngine()
{
	currentRunningMachine = invalidIndex;
	typeManager = script_type_manager();
}
void script_engine::start()
{
	//map the scripts to individual units to be parsed
	char buff[ 1024 ] = { 0 };
	GetCurrentDirectory( sizeof( buff ), buff );
	std::string const path = std::string( buff ) + "\\script";

	std::string scriptPath;
	OPENFILENAMEA ofn ={ 0 };
	char buff2[1024] ={ 0 };
	ofn.lStructSize = sizeof( OPENFILENAMEA );
	ofn.lpstrFilter = "All Files\0*.*\0\0";
	ofn.lpstrFile = buff2;
	ofn.nMaxFile = sizeof( buff2 );
	ofn.lpstrTitle = "Open script...";
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST ;
	GetOpenFileNameA( &ofn );
	SetCurrentDirectory( buff );
	scriptPath= ofn.lpstrFile;
	if( !scriptPath.size() )
	{
		MessageBox( NULL, "File not selected", "script_engine::start", NULL );
		return;
	}
	parseScriptFromFile( scriptPath );
	if( error ) return;
	size_t scriptIdx = findScriptFromFile( getCurrentScriptPath() );
	if( scriptIdx == invalidIndex ) return;
	currentRunningMachine = fetchScriptMachine();
	script_machine & machine = getScriptMachine( currentRunningMachine );
	machine.initialize( *this, scriptIdx );
}
bool script_engine::advance()
{
	while( inventory::vecQueuedScripts.size() )
	{
		switch( inventory::vecQueuedScripts.front().queueType )
		{
		case script_queue::Initialization: //script index
			getScriptMachine( fetchScriptMachine() ).initialize( *this, inventory::vecQueuedScripts.front().index ); 
			break;
		case script_queue::Termination: //machine index
			callSub( inventory::vecQueuedScripts.front().index, script_container::AtFinalize );
			getScriptMachine( inventory::vecQueuedScripts.front().index ).clean( *this );
			releaseScriptMachine( inventory::vecQueuedScripts.front().index );
			break;
		}
		inventory::vecQueuedScripts.erase( inventory::vecQueuedScripts.begin() );
	}
	for( unsigned u = 0; u < inventory::vecMachines.size(); ++u )
	{
		if( error ) return false;
		if( inventory::vecMachines[ u ].isOperable() )
			callSub( u, script_container::AtMainLoop );
	}
	return true;
}

void script_engine::callSub( size_t machineIndex, script_container::sub AtSub )
{
	unsigned prevMachine = currentRunningMachine;
	currentRunningMachine = machineIndex;
	script_machine & m = getScriptMachine( machineIndex );
	assert( m.current_thread_index != invalidIndex && m.current_script_index != invalidIndex );
	size_t blockIndex = invalidIndex;
	script_container & sc = vecScripts[ m.current_script_index ];
	//initializing
	if( !m.threads.size() )
	{
		m.threads.push_back( fetchScriptEnvironment( getScript( m.current_script_index ).ScriptBlock ) );
		getScriptEnvironment( m.threads[ 0 ] ).parentIndex = invalidIndex;
		getScriptEnvironment( m.threads[ 0 ] ).hasResult = false;
		m.current_thread_index = 0;
		while( !m.advance( *this ) );
		callSub( machineIndex, script_container::AtInitialize );
		currentRunningMachine = prevMachine;
		return;
	}
	switch( AtSub )
	{
	case script_container::AtInitialize:
		blockIndex = sc.InitializeBlock;
		break;
	case script_container::AtFinalize:
		blockIndex = sc.FinalizeBlock;
		break;
	case script_container::AtMainLoop:
		blockIndex = sc.MainLoopBlock;
		break;
	case script_container::AtBackGround:
		blockIndex = sc.BackGroundBlock;
		break;
	}
	if( blockIndex != invalidIndex )
	{
		++getScriptEnvironment( m.threads[ m.current_thread_index ] ).refCount;
		size_t calledEnv = fetchScriptEnvironment( blockIndex );
		script_environment & e = getScriptEnvironment( calledEnv );
		e.parentIndex = m.threads[ m.current_thread_index ];
		e.hasResult = 0;
		m.threads[ m.current_thread_index ] = calledEnv;
		while( !m.advance( *this ) );
	}

	currentRunningMachine = prevMachine;
}
void script_engine::parseScriptFromFile( std::string const & scriptPath )
{
	try
	{
		if( inventory::mappedMainScripts.find( scriptPath ) == inventory::mappedMainScripts.end() )
			parseScript( scriptPath );
	}
	catch( ... )
	{
	}
}
script_engine::~script_engine()
{
}

//script engine - miscellanious functions
void script_engine::raiseError( std::string const errMsg )
{
	errorMessage = errMsg;
	error = true;
	MessageBox( NULL, errorMessage.c_str(), "script_engine", NULL );
	throw;
}