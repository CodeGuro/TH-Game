#include <scriptmachine.hpp>
#include <scriptengine.hpp>
#include <defstypedefs.hpp>
#include <assert.h>

void script_machine::initialize( script_engine & eng, size_t script_index )
{
	assert( !threads.size() );
	current_script_index = script_index;
	threads.push_back( eng.fetchScriptEnvironment( eng.getScript( script_index ).ScriptBlock ) );
	eng.getScriptEnvironment( threads[ 0 ] ).parentIndex = invalidIndex;
	while( advance( eng ) );
}
bool script_machine::advance( script_engine & eng )
{
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