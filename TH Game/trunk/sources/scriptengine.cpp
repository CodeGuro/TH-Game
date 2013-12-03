#include <scriptengine.hpp>
#include <parser.hpp>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <Windows.h>

//exception
eng_exception::eng_exception() : throw_reason( eng_error )
{
}
eng_exception::eng_exception( Reason const r ) : throw_reason( r )
{
}

//script engine - public functions, called from the outside
script_engine::script_engine() : error( false ), currentRunningMachine( -1 )
{
}
void script_engine::cleanEngine()
{
	currentRunningMachine = -1;
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
	parseScript( scriptPath );
	if( error ) return;
	size_t scriptIdx = findScriptFromFile( getCurrentScriptPath() );
	if( !CheckValidIdx( scriptIdx ) ) return;
	currentRunningMachine = fetchScriptMachine();
	script_machine & machine = getScriptMachine( currentRunningMachine );
	machine.initialize( *this, scriptIdx );
}
bool script_engine::advance()
{
	unsigned u = 0;
	while( !error && u < inventory::vecMachines.size() )
	{
		try
		{
			for(; u < inventory::vecMachines.size(); ++u )
			{
				if( error ) return false;
				callSub( u, script_container::AtMainLoop );
			}
		}
		catch( eng_exception const & e )
		{
			switch( e.throw_reason )
			{
			case eng_exception::eng_error:
				error = true;
				break;
			}
		}
	}
	return true;
}

void script_engine::callSub( size_t machineIndex, script_container::sub AtSub )
{
	unsigned prevMachine = currentRunningMachine;
	currentRunningMachine = machineIndex;
	script_machine & m = getScriptMachine( machineIndex );
	assert( CheckValidIdx( m.current_thread_index ) && CheckValidIdx( m.current_script_index ) );
	size_t blockIndex = -1;
	script_container & sc = vecScripts[ m.current_script_index ];
	//initializing
	if( !m.threads.size() )
	{
		m.threads.push_back( fetchScriptEnvironment( getScript( m.current_script_index ).ScriptBlock ) );
		getScriptEnvironment( m.threads[ 0 ] ).parentIndex = -1;
		getScriptEnvironment( m.threads[ 0 ] ).hasResult = false;
		m.current_thread_index = 0;
		while( !getScriptMachine( machineIndex ).advance( *this ) );
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
	if( CheckValidIdx( blockIndex ) )
	{
		++getScriptEnvironment( m.threads[ m.current_thread_index ] ).refCount;
		size_t calledEnv = fetchScriptEnvironment( blockIndex );
		script_environment & e = getScriptEnvironment( calledEnv );
		e.parentIndex = m.threads[ m.current_thread_index ];
		e.hasResult = 0;
		m.threads[ m.current_thread_index ] = calledEnv;
		while( !getScriptMachine( machineIndex ).advance( *this ) );
	}

	currentRunningMachine = prevMachine;
}
void script_engine::raise_exception( eng_exception const & eng_except )
{
	throw eng_except;
}
script_engine::~script_engine()
{
}