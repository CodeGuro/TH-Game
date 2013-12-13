#include <scriptmachine.hpp>
#include <scriptengine.hpp>
#include <defstypedefs.hpp>
#include <assert.h>

void script_machine::initialize( script_engine & eng, size_t script_index )
{
	assert( !threads.size() );
	current_script_index = script_index;
	object_vector_index = eng.fetchObjectVector();
}
bool script_machine::advance( script_engine & eng )
{
	assert( threads.size() > current_thread_index );
	script_environment & env = eng.getScriptEnvironment( threads[ current_thread_index ] );
	if( env.codeIndex >= eng.getBlock( env.blockIndex ).vecCodes.size() )
	{
		size_t disposing = threads[ current_thread_index ];
		if( !CheckValidIdx( eng.getScriptEnvironment( disposing ).parentIndex ) )
			return true; //do not dispose initial environment
		script_environment & disposing_env = eng.getScriptEnvironment( threads[ current_thread_index ] );
		threads[ current_thread_index ] = disposing_env.parentIndex;

		if( disposing_env.hasResult )
		{
			eng.addRefScriptData( disposing_env.values[ 0 ] );
			eng.getScriptEnvironment( disposing_env.parentIndex ).stack.push_back( disposing_env.values[ 0 ] );
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
			script_environment * e;
			for( e = &env; e->blockIndex != current_code.blockIndex; e = &eng.getScriptEnvironment( e->parentIndex ) );
			if( e->values.size() <= current_code.variableIndex )
				e->values.resize( 4 + 2 * e->values.size(), -1 );
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
			env.stack.push_back( -1 );
			eng.scriptDataAssign( env.stack.back(), current_code.scriptDataIndex );
		}
		break;
	case vc_pushVar:
		{
			script_environment * e;
			for( e = &env; e->blockIndex != current_code.blockIndex; e = &eng.getScriptEnvironment( e->parentIndex ) );
			env.stack.push_back( -1 );
			eng.scriptDataAssign( env.stack.back(), e->values[ current_code.variableIndex ] );
		}
		break;
	case vc_duplicate:
		{
			eng.addRefScriptData( env.stack.back() );
			env.stack.push_back( env.stack.back() );
		}
		break;
	case vc_callFunction:
	case vc_callFunctionPush:
	case vc_callTask:
		{
			block & b = eng.getBlock( current_code.subIndex );
			if( b.nativeCallBack ) //always functional
			{
				env.stack.push_back( -1 );
				unsigned popCount = current_code.argc + (current_code.command != vc_callFunctionPush ? 1 : 0 );
				b.nativeCallBack( &eng, &env.stack[ env.stack.size() - ( 1 + current_code.argc ) ] );
				{
					script_machine & current_machine = eng.getScriptMachine( eng.currentRunningMachine );
					script_environment & env = eng.getScriptEnvironment( current_machine.threads[ current_machine.current_thread_index ] ); //in case of re-allocation
					for( unsigned u = 0; u < popCount; ++ u )
					{
						eng.releaseScriptData( env.stack.back() );
						env.stack.pop_back();
					}
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
					new_env.stack.push_back( *(current_env.stack.end() - 1 - u) );
				current_env.stack.erase( current_env.stack.end() - current_code.argc, current_env.stack.end() );
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
				if( !(b.kind == block::bk_function || b.kind == block::bk_task || b.kind == block::bk_sub) && CheckValidIdx( e->parentIndex ) )
					e = &eng.getScriptEnvironment( e->parentIndex );
				else
					BaseRoutine = true;
			}while( !BaseRoutine );
		}
		break;
	case vc_breakLoop:
		{
			bool BaseRoutine = false; //loops
			script_environment * e = &env;
			do
			{
				block const & b = eng.getBlock( e->blockIndex );
				e->codeIndex = b.vecCodes.size();
				e = &eng.getScriptEnvironment( e->parentIndex );
				if( b.kind == block::bk_loop )
				{
					BaseRoutine = true;
					for( unsigned u = 0; u < e->stack.size(); ++u )
						eng.releaseScriptData( e->stack[ u ] );
					e->stack.resize( 0 );
					do
						++(e->codeIndex);
					while( eng.getBlock( e->blockIndex ).vecCodes[ e->codeIndex - 1 ].command != vc_loopBack );
					assert( env.stack.size() == 0 );
					assert( e->stack.size() == 0 );
				}
			}while( !BaseRoutine );
		}
		break;
	case vc_loopIf:
		{
			if( eng.getRealScriptData( env.stack.back() ) != 0.f )
			{
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
			}
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			assert( env.stack.size() == 0 );
		}
		break;
	case vc_loopBack:
		env.codeIndex = current_code.loopBackIndex;
		break;
	case vc_yield:
		current_thread_index = ( current_thread_index ? current_thread_index : threads.size() ) - 1;
		break;
	case vc_checkIf:
		{
			float real = eng.getRealScriptData( env.stack.back() );
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			if( real != 0.f )
			{
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_caseNext );
				assert( env.stack.size() == 0 );
			}
		}
		break;
	case vc_loopAscent:
	case vc_loopDescent:
		{
			float real = eng.getRealScriptData( env.stack.back() );
			eng.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			bool proceed = ((current_code.command == vc_loopAscent) ? real > eng.getRealScriptData( env.stack.back() )
				: real < eng.getRealScriptData( env.stack.back() ) );
			if( !proceed )
			{
				eng.releaseScriptData( env.stack.back() );
				env.stack.pop_back();
				do
					++env.codeIndex;
				while( eng.getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
				assert( env.stack.size() == 0 );
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
	case vc_invalid:
	default:
		assert( false );
	}
	return false;
}
void script_machine::clean( script_engine & eng )
{
	if( !threads.size() )
		return;
	
	current_thread_index = threads.size() - 1;
	do
		eng.getScriptEnvironment( threads[ current_thread_index ] ).codeIndex = -1;
	while( !advance( eng ) );
	eng.releaseScriptEnvironment( threads[ current_thread_index ] );
	threads.pop_back();
	eng.releaseObjectVector( object_vector_index );
	current_script_index = -1;
	current_thread_index = -1;
	object_vector_index = -1;
	assert( !threads.size() );
}
size_t script_machine::getScriptIndex() const
{
	return current_script_index;
}
size_t script_machine::getObjectVectorIndex() const
{
	return object_vector_index;
}