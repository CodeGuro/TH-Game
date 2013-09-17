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
			unsigned refs = eng.getScriptEnvironment( disposing ).refCount;
			size_t next = eng.getScriptEnvironment( disposing ).parentIndex;
			eng.releaseScriptEnvironment( disposing );
			if( refs > 1 )
				break;
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
			if( e->values.size() <= current_code.variableIndex )
				e->values.resize( 4 + 2 * e->values.size(), invalidIndex );
			eng.scriptDataAssign( e->values[ current_code.variableIndex ], env.stack.back() );
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
		}
		break;
	case vc_overWrite:
		{
			eng.copyScriptData( env.stack[ env.stack.size() - 2 ], env.stack.back() );
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
		}
		break;
	case vc_pushVal:
		{
			env.stack.push_back( invalidIndex );
			eng.scriptDataAssign( env.stack.back(), current_code.scriptDataIndex );
		}
		break;
	case vc_pushVar:
		{
			script_environment * e = &env;
			for( unsigned u = 0; u < current_code.variableLevel; ++u )
				e = &eng.getScriptEnvironment( e->parentIndex );
			env.stack.push_back( invalidIndex );
			eng.scriptDataAssign( env.stack.back(), e->values[ current_code.variableIndex ] );
		}
		break;
	case vc_callFunction:
	case vc_callFunctionPush:
	case vc_callTask:
		{
			block & b = eng.getBlock( current_code.subIndex );
			if( b.nativeCallBack ) //always functional
			{
				env.stack.push_back( invalidIndex );
				b.nativeCallBack( &eng, &env.stack[ env.stack.size() - ( 1 + current_code.argc ) ] );
				unsigned popCount = current_code.argc + (current_code.command != vc_callFunctionPush ? 1 : 0 );
				for( unsigned u = 0; u < popCount; ++ u )
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
					new_env.stack.push_back( env.stack[ *(env.stack.end() - 1 - u) ] );
					eng.addRefScriptData( *(env.stack.end() - 1 - u) );
				}
				if( current_code.command != vc_callTask )
					threads[ current_thread_index ] = envIdx;
				else
					threads.insert( threads.begin() + ++current_thread_index, envIdx );
			}
		}
		break;
	case vc_breakRoutine:
		{
			bool BaseRoutine = false; //task/function returns
			script_environment * e = &env;
			do
			{
				block const & b = eng.getBlock( e->blockIndex );
				e->codeIndex = b.vecCodes.size();
				if( !(b.kind == block::bk_function || b.kind == block::bk_task || b.kind == block::bk_sub) && e->parentIndex != invalidIndex )
					e = &eng.getScriptEnvironment( e->parentIndex );
				else
					BaseRoutine = true;
			}while( !BaseRoutine );
		}
		break;
	case vc_loopIfDecr:
		{
			float real = eng.getRealScriptData( env.stack.back() );
			if( real <= 0 )
			{
				eng.releaseScriptData( env.stack.back() );
				env.stack.pop_back();
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
			}
			else
			{
				eng.releaseScriptData( env.stack.back() );
				env.stack.back() = eng.fetchScriptData( real - 1 );
			}
		}
		break;
	case vc_loopIf:
		{
			if( eng.getRealScriptData( env.stack.back() ) <= 0 )
			{
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
			}
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
		}
		break;
	case vc_loopBack:
		env.codeIndex = current_code.loopBackIndex;
		break;
	case vc_yield:
		current_thread_index = ( current_thread_index ? threads.size() : current_thread_index ) - 1;
		break;
	case vc_checkIf:
		{
			float real = eng.getRealScriptData( env.stack.back() );
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			if( real <= 0 )
			{
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_caseNext );
			}
		}
		break;
	case vc_caseBegin:
	case vc_caseNext:
	case vc_caseEnd:
		break;
	case vc_gotoEnd:
			for( block const & b = eng.getBlock( env.blockIndex );
				b.vecCodes[ env.codeIndex - 1 ].command != vc_caseEnd;
				++env.codeIndex );
		break;
	case vc_dup:
		env.stack.push_back( env.stack.back() );
		eng.addRefScriptData( env.stack.back() );
		break;
	case vc_invalid:
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