#include <scriptmachine.hpp>
#include <scriptengine.hpp>
#include <defstypedefs.hpp>

void script_machine::clean( script_engine & eng )
{
	for( unsigned i = 0; i < threads.size(); ++i )
	{
		script_environment & env = eng.getScriptEnvironment( threads[i] );
		for( unsigned j = 0; j < env.stack.size(); ++j )
			if( env.stack[j] != invalidIndex )
				eng.releaseScriptData( env.stack[j] );

		eng.disposeScriptEnvironment( threads[i] );
		threads[i] = invalidIndex;
	}
}