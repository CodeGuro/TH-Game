#include <scriptmachine.hpp>
#include <scriptengine.hpp>
#include <defstypedefs.hpp>
#include <assert.h>

void script_machine::initialize( script_engine & eng, size_t script_index )
{
	assert( !threads.size() );
	current_script_index = script_index;
	current_thread_index = 0;
	threads.push_back( eng.fetchScriptEnvironment( eng.getScript( script_index ).ScriptBlock ) );
	eng.getScriptEnvironment( threads[ 0 ] ).parentIndex = invalidIndex;
	eng.getScriptEnvironment( threads[ 0 ] ).hasResult = false;
	while( !advance( eng ) );
}
bool script_machine::advance( script_engine & eng )
{
	assert( threads.size() > current_thread_index );
	script_environment & env = eng.getScriptEnvironment( threads[ current_thread_index ] );
	if( env.codeIndex >= eng.getBlock( env.blockIndex ).vecCodes.size() )
	{
		size_t disposing = threads[ current_thread_index ];
		if( eng.getScriptEnvironment( disposing ).parentIndex == invalidIndex )
			return true; //do not dispose initial environment
		script_environment & disposing_env = eng.getScriptEnvironment( threads[ current_thread_index ] );
		threads[ current_thread_index ] = disposing_env.parentIndex;

		if( disposing_env.hasResult )
		{
			eng.addRefScriptData( disposing_env.stack[ 0 ] );
			eng.getScriptEnvironment( disposing_env.parentIndex ).stack.push_back( disposing_env.stack[ 0 ] );
		}
		else if( eng.getBlock( disposing_env.blockIndex ).kind == block::bk_task )
			threads.erase( threads.begin() + current_thread_index-- );

		do
		{
			if( !(--eng.getScriptEnvironment( disposing ).refCount ) )
				break;
			
			size_t next = eng.getScriptEnvironment( disposing ).parentIndex;
			eng.releaseScriptEnvironment( disposing );
			disposing = next;
		}while( true );
		return false;
	}
	code const & current_code = eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex++ ];
	switch( current_code.command )
	{
	case vc_assign:
		{
			script_environment * e = &env;
			for( unsigned u = 0; u < current_code.variableLevel; ++u )
				e = &eng.getScriptEnvironment( e->parentIndex );
			if( e->stack.size() < 1 + current_code.variableIndex )
				e->stack.resize( 3 + 2 * e->stack.size(), invalidIndex );
			eng.scriptDataAssign( e->stack[ current_code.variableIndex ], env.stack[ env.stack.size() - 1 ] );
			eng.releaseScriptData( env.stack[ env.stack.size() - 1 ] );
			env.stack.pop_back();
		}
		break;
	case vc_overWrite:
		{
			script_environment * e = &env;
			for( unsigned u = 0; u < current_code.variableLevel; ++u )
				e = &eng.getScriptEnvironment( e->parentIndex );
			eng.uniqueizeScriptData( env.stack[ env.stack.size() - 1 ] );
			eng.copyScriptData( e->stack[ current_code.variableIndex ], env.stack[ env.stack.size() - 1 ] );
			eng.releaseScriptData( env.stack[ env.stack.size() - 1 ] );
			env.stack.pop_back();
		}
		break;
	case vc_pushVal:
		{
			env.stack.push_back( invalidIndex );
			eng.scriptDataAssign( env.stack[ env.stack.size() - 1 ], current_code.scriptDataIndex );
		}
		break;
	case vc_pushVar:
		{
			script_environment * e = &env;
			for( unsigned u = 0; u < current_code.variableLevel; ++u )
				e = &eng.getScriptEnvironment( e->parentIndex );
			env.stack.push_back( invalidIndex );
			eng.scriptDataAssign( env.stack[ env.stack.size() - 1 ], e->stack[ current_code.variableIndex ] );
		}
		break;
	case vc_callFunction:
	case vc_callFunctionPush:
	case vc_callTask:
		{
			block & b = eng.getBlock( current_code.subIndex );
			if( b.nativeCallBack ) //always functional
			{
				if( current_code.command == vc_callFunctionPush )
					env.stack.push_back( invalidIndex );
					b.nativeCallBack( &eng, &env.stack[ env.stack.size() - ( 1 + current_code.argc ) ] );
					for( unsigned u = 0; u < current_code.argc; ++ u )
					{
						eng.releaseScriptData( env.stack.back() );
						env.stack.pop_back();
					}
			}
			else
			{
				size_t envIdx = eng.fetchScriptEnvironment( current_code.subIndex );
				script_environment & current_env = eng.getScriptEnvironment( threads[ current_thread_index ] );
				script_environment & new_env = eng.getScriptEnvironment( envIdx );
				new_env.hasResult = ( current_code.command == vc_callFunctionPush );
				new_env.parentIndex = threads[ current_thread_index ];
				++current_env.refCount;
				for( unsigned u = 0; u < current_code.argc; ++ u )
				{
					new_env.stack.push_back( env.stack[ *(env.stack.end() - current_code.argc + u ) ] );
					eng.addRefScriptData( *(env.stack.end() - current_code.argc + u ) );
				}
				if( current_code.command != vc_callTask )
					threads[ current_thread_index ] = envIdx;
				else
					threads.insert( threads.begin() + ++current_thread_index, envIdx );
			}
		}
		break;
	default:
		assert( false );
	}
	return false;
}
void script_machine::clean( script_engine & eng )
{
	for( unsigned i = 0; i < threads.size(); ++i )
	{
		script_environment & env = eng.getScriptEnvironment( threads[i] );
		for( unsigned j = 0; j < env.stack.size(); ++j )
			eng.releaseScriptData( env.stack[j] );
		eng.releaseScriptEnvironment( threads[i] );
	}
}