#include <scriptengine.hpp>
#include <parser.hpp>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <Windows.h>
#include <ObjMgr.hpp>

//exception
eng_exception::eng_exception() : throw_reason( eng_error )
{
}
eng_exception::eng_exception( Reason const r ) : throw_reason( r )
{
}
eng_exception::eng_exception( Reason const r, std::string const & String ) : throw_reason( r ), Str( String )
{
}

//script engine - public functions, called from the outside
script_engine::script_engine() : error( false ), finished( false ), currentRunningMachine( -1 )
{
}
void script_engine::cleanEngine()
{
	currentRunningMachine = -1;
	error = false;
	errorMessage.clear();
	cleanInventory( *this );
}
bool script_engine::start()
{
	try
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
			MessageBox( NULL, "Script not chosen, terminating...", "Engine Error", MB_TASKMODAL );
			return false;
		}
		parseScript( scriptPath );
		size_t scriptIdx = findScriptFromFile( getCurrentScriptPath() );
		if( !CheckValidIdx( scriptIdx ) ) raise_exception( eng_exception( eng_exception::eng_error ) );
		currentRunningMachine = fetchScriptMachine();
		script_machine & machine = getScriptMachine( currentRunningMachine );
		machine.initialize( *this, scriptIdx );
	}
	catch( eng_exception const & error )
	{
		if( error.throw_reason == eng_exception::eng_error )
		{
			if( error.Str.size() )
				MessageBox( NULL, error.Str.c_str(), "Engine Error", MB_TASKMODAL );
			cleanEngine();
			return start();
		}
	}
	return true;
}
bool script_engine::advance()
{
	currentRunningMachine = 0;
	UpdateObjectCollisions();
	while( currentRunningMachine < getMachineCount() )
	{
		try
		{
			for(; currentRunningMachine < getMachineCount(); ++currentRunningMachine )
			{
				if( error ) return false;
				script_machine const & machine = getScriptMachine( currentRunningMachine );
				Object * current = getObjFromScriptVector( machine.object_vector_index, machine.script_object );
				if( current )
				{
					if( current->FlagCollision( -1 ) )
						callSub( currentRunningMachine, script_container::AtHit );
				}
				callSub( currentRunningMachine, script_container::AtMainLoop );
			}
		}
		catch( eng_exception const & e )
		{
			switch( e.throw_reason )
			{
			case eng_exception::eng_error:
				{
					cleanEngine();
					start();
				}
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
	script_container & sc = getScript( m.current_script_index );
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
		{
			//check to see if latched object (if there is one) is valid
			if( CheckValidIdx( getScriptMachine( machineIndex ).getLatchedObject() ) && 
				CheckValidIdx( getScriptMachine( machineIndex ).getObjectVectorIndex() ) )
			{
				Object * obj = getObjFromScriptVector( getScriptMachine( machineIndex ).getObjectVectorIndex(), getScriptMachine( machineIndex ).getLatchedObject() ); // GetObject( vvecObjects[ getScriptMachine( machineIndex ).getObjectVectorIndex() ][ getScriptMachine( machineIndex ).getLatchedObject() ] );
				if( !obj )
				{
					//latched object has been deleted, terminate machine
					callSub( machineIndex, script_container::AtFinalize );
					getScriptMachine( machineIndex ).clean( *this );
					releaseScriptMachine( machineIndex );
					raise_exception( eng_exception( eng_exception::finalizing_machine ) );
					break;
				}
			}
			blockIndex = sc.MainLoopBlock;
		}
		break;
	case script_container::AtBackGround:
		blockIndex = sc.BackGroundBlock;
		break;
	case script_container::AtHit:
		blockIndex = sc.HitBlock;
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
bool script_engine::IsFinished()
{
	return finished;
}