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