#include <scriptengine.hpp>
#include <parser.hpp>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <Windows.h>

//script type manager, script_engine::getScriptTypeManager
script_engine::script_type_manager::script_type_manager()
{
	types.push_back( type_data( type_data::tk_real, invalidIndex ) );
	types.push_back( type_data( type_data::tk_boolean, invalidIndex ) );
	types.push_back( type_data( type_data::tk_char, invalidIndex ) );
	types.push_back( type_data( type_data::tk_array, invalidIndex) );
	types.push_back( type_data( type_data::tk_array, (size_t)2 ) );
	types.push_back( type_data( type_data::tk_object, invalidIndex ) );
}
type_data script_engine::script_type_manager::getRealType() const
{
	return type_data( type_data::tk_real, invalidIndex );
}
type_data script_engine::script_type_manager::getBooleanType() const
{
	return type_data( type_data::tk_boolean, invalidIndex );
}
type_data script_engine::script_type_manager::getCharacterType() const
{
	return type_data( type_data::tk_char, invalidIndex );
}
type_data script_engine::script_type_manager::getStringType() const
{
	return type_data( type_data::tk_array, getCharacterType().get_kind() );
}
type_data script_engine::script_type_manager::getObjectType() const
{
	return type_data( type_data::tk_object, invalidIndex );
}
type_data script_engine::script_type_manager::getArrayType() const
{
	return type_data( type_data::tk_array, invalidIndex );
}
type_data script_engine::script_type_manager::getArrayType( size_t element )
{
	for( unsigned i = 0; i < types.size(); ++i )
	{
		if( types[i].get_kind() == type_data::tk_array && types[i].get_element() == element )
			return types[i];
	}
	//else
	return *(types.insert( types.end(), type_data( type_data::tk_array, element ) ));
}

//script engine block-related functions
size_t script_engine::fetchBlock()
{
	size_t index = battery.vecBlocks.size();
	battery.vecBlocks.push_back( block() );
	return index;
}
block & script_engine::getBlock( size_t index )
{
	return battery.vecBlocks[index];
}
void script_engine::registerScript( std::string const scriptName )
{
	battery.mappedScripts[ scriptName ] = battery.vecScripts.size();
	script_container new_cont;
	memset( &new_cont, invalidIndex, sizeof( new_cont) );
	battery.vecScripts.push_back( new_cont );
}
void script_engine::registerMainScript( std::string const scriptPath, std::string const scriptName )
{
	battery.mappedMainScripts[ scriptPath ] = battery.vecScripts.size();
	registerScript( scriptName );
}
void script_engine::registerInvalidMainScript( std::string const scriptPath )
{
	battery.mappedMainScripts[ scriptPath ] = invalidIndex;
}
script_container * script_engine::getScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = battery.mappedScripts.find( scriptName );
	if( it != battery.mappedScripts.end() )
		return &(battery.vecScripts[ it->second ]);
	return 0;
}
script_container & script_engine::getScript( size_t index )
{
	return battery.vecScripts[ index ];
}
size_t script_engine::findScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = battery.mappedScripts.find( scriptName );
	if( it != battery.mappedScripts.end() )
		return it->second;
	return invalidIndex;
}
size_t script_engine::findScriptFromFile( std::string const & scriptPath )
{
	std::map< std::string, unsigned >::iterator it = battery.mappedMainScripts.find( scriptPath );
	if( it != battery.mappedMainScripts.end() )
		return it->second;
	return invalidIndex;
}

//script engine - script data - related functions
size_t script_engine::fetchScriptData()
{
	size_t index;
	if( battery.vecScriptDataGarbage.size() )
	{
		index = battery.vecScriptDataGarbage.back();
		battery.vecScriptDataGarbage.pop_back();
	}
	else
	{
		index = battery.vecScriptData.size();
		battery.vecScriptData.resize( 1 + index );
	}
	getScriptData( index ).refCount = 1;
	return index;
}
size_t script_engine::fetchScriptData( float real )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = real;
	data.type = typeManager.getRealType();
	return index;
}
size_t script_engine::fetchScriptData( char character )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.character = character;
	data.type = typeManager.getCharacterType();
	return index;
}
size_t script_engine::fetchScriptData( bool boolean )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = (float)boolean;
	data.type = typeManager.getBooleanType();
	return index;
}
size_t script_engine::fetchScriptData( std::string const & string )
{
	size_t index = fetchScriptData();
	getScriptData( index ).type = typeManager.getStringType();
	for( unsigned i = 0; i < string.size(); ++i )
		getScriptData( index ).vec.push_back( fetchScriptData( string[i] ) );
	return index;
}
script_data & script_engine::getScriptData( size_t index )
{
	return battery.vecScriptData[ index ];
}
void script_engine::addRefScriptData( size_t index ) //interface function
{
	if( index != invalidIndex )
		++getScriptData( index ).refCount;
}
void script_engine::releaseScriptData( size_t & index ) //interface function
{
	if( index != invalidIndex )
	{
		script_data & dat = getScriptData( index );
		if( !(--dat.refCount) )
		{
			for( unsigned i = 0; i < dat.vec.size(); ++i )
				releaseScriptData( dat.vec[i] );
			dat.vec.resize( 0 );
			battery.vecScriptDataGarbage.push_back( index );
		}
		index = invalidIndex;
	}
}
void script_engine::scriptDataAssign( size_t & dst, size_t src ) //index copy
{
	addRefScriptData( src );
	releaseScriptData( dst );
	dst = src;
}
void script_engine::copyScriptData( size_t & dst, size_t & src ) //contents copy, including vector
{
	if( dst == invalidIndex )
		dst = fetchScriptData();
	script_data & destDat = getScriptData( dst );

	for( unsigned i = 0; i < destDat.vec.size(); ++i )
		releaseScriptData( destDat.vec[i] );
	destDat.vec.resize( 0 );

	if( src != invalidIndex )
	{
		script_data & sourDat = getScriptData( src );
		switch( ( destDat.type = sourDat.type ).get_kind() )
		{
		case type_data::tk_real:
		case type_data::tk_boolean:
			destDat.real = sourDat.real;
			break;
		case type_data::tk_char:
			destDat.character = sourDat.character;
			break;
		case type_data::tk_object:
			destDat.objIndex = sourDat.objIndex;
			break;
		case type_data::tk_array:
			{
				destDat.vec.resize( sourDat.vec.size() );
				for( unsigned i = 0; i < sourDat.vec.size(); ++i )
					(sourDat.vec[i] == invalidIndex) ? (destDat.vec[i] = invalidIndex) : (copyScriptData( ( destDat.vec[i] = fetchScriptData() ), sourDat.vec[i] ));
			}
			break;
		}
	}
}
void script_engine::uniqueizeScriptData( size_t & dst )
{
	if( dst != invalidIndex )
	{
		if( getScriptData( dst ).refCount > 1 )
		{
			size_t tmpDst = dst;
			releaseScriptData( dst ); //dst has more than 1 ref count, so we'll continue using
			size_t uni = fetchScriptData();
			switch( ( getScriptData( uni ).type = getScriptData( tmpDst ).type ).get_kind() )
			{
			case type_data::tk_real:
			case type_data::tk_boolean:
				getScriptData( uni ).real = getScriptData( tmpDst ).real;
				break;
			case type_data::tk_char:
				getScriptData( uni ).character = getScriptData( tmpDst ).character;
				break;
			case type_data::tk_object:
				getScriptData( uni ).objIndex = getScriptData( tmpDst ).objIndex;
				break;
			case type_data::tk_array:
				{
					for( unsigned i = 0; i < getScriptData( tmpDst ).vec.size(); ++i )
					{
						addRefScriptData( getScriptData( tmpDst ).vec[ i ] );
						getScriptData( uni ).vec.push_back( getScriptData( tmpDst ).vec[ i ] );
						size_t buff = getScriptData( uni ).vec[ i ];
						uniqueizeScriptData( buff );
						getScriptData( uni ).vec[ i ] = buff;
					}
				}
				break;
			}
			dst = uni;
		}
	}
	else
		getScriptData( dst = fetchScriptData() ).type.kind = type_data::tk_invalid;
}
std::string script_engine::getStringScriptData( size_t index )
{
	std::string result;
	if( index != invalidIndex )
	{
		script_data const & dat = getScriptData( index );
		switch( dat.type.get_kind() )
		{
			case type_data::tk_array:
			{
				if( dat.type.get_element() != typeManager.getStringType().get_element() )
					result += "[ ";
				unsigned size = dat.vec.size();
				for( unsigned i = 0; i < size; ++i )
				{
					result += getStringScriptData( dat.vec[ i ] );
					if( getScriptData( dat.vec[ i ] ).type.kind != typeManager.getCharacterType().kind && i + 1 < size )
						result += " , ";
				}
				if( dat.type.get_element() != typeManager.getStringType().get_element()  )
					result += " ]";
			}
			break;
			case type_data::tk_boolean:
				result = ( getBooleanScriptData( index ) ? "TRUE" : "FALSE" );
			break;
			case type_data::tk_char:
				result = getCharacterScriptData( index );
				break;
			case type_data::tk_real:
				{
					std::stringstream ss;
					ss << getRealScriptData( index );
					result = ss.str();
				}
				break;
			case type_data::tk_object:
				result = "(OBJECT TYPE)";
				break;
			case type_data::tk_invalid:
			default:
				assert( 0 );
		}
		return result;
	}
	else result = "(INVALID SCRIPT DATA INDEX)";
	return result;
}
float script_engine::getRealScriptData( size_t index ) const
{
	if( index != invalidIndex )
		return battery.vecScriptData[ index ].real;
	return -1;
}
bool script_engine::getBooleanScriptData( size_t index ) const
{
	if( index != invalidIndex )
		return battery.vecScriptData[ index ].real != 0;
	return true;

}
char script_engine::getCharacterScriptData( size_t index ) const
{
	if( index != invalidIndex )
		return battery.vecScriptData[ index ].character;
	return -1;
}

//script engine - script environment - related functions
size_t script_engine::fetchScriptEnvironment( size_t blockIndex )
{
	size_t index;
	if( battery.vecRoutinesGabage.size() )
	{
		index = battery.vecRoutinesGabage.back();
		battery.vecRoutinesGabage.pop_back();
	}
	else
	{
		index = battery.vecScriptEnvironment.size();
		battery.vecScriptEnvironment.resize( 1 + index );
	}
	script_environment & env = getScriptEnvironment( index );
	env.blockIndex = blockIndex;
	env.codeIndex = 0;
	env.refCount = 1;
	return index;
}
script_environment & script_engine::getScriptEnvironment( size_t index )
{
	return battery.vecScriptEnvironment[ index ];
}
void script_engine::addRefScriptEnvironment( size_t index )
{
	if( index != invalidIndex )
		++getScriptEnvironment( index ).refCount;
}
void script_engine::releaseScriptEnvironment( size_t & index )
{
	if( index != invalidIndex )
	{
		script_environment & env = getScriptEnvironment( index );
		if( !( --env.refCount ) )
		{
			for( unsigned i = 0; i < env.stack.size(); ++i )
				releaseScriptData( env.stack[i] );
			for( unsigned u = 0; u < env.values.size(); ++u )
				releaseScriptData( env.values[ u ] );
			env.stack.resize( 0 );
			env.values.resize( 0 );
			battery.vecRoutinesGabage.push_back( index );
		}
		index = invalidIndex;
	}
}

//script engine - script machine - related functions
size_t script_engine::fetchScriptMachine()
{
	size_t index;
	if( battery.vecMachinesGarbage.size() )
	{
		index = battery.vecMachinesGarbage.back();
		battery.vecMachinesGarbage.pop_back();
	}
	else
	{
		index = battery.vecMachines.size();
		battery.vecMachines.resize( 1 + index );
	}
	return index;
}
script_machine & script_engine::getScriptMachine( size_t index )
{
	return battery.vecMachines[ index ];
}
void script_engine::releaseScriptMachine( size_t & index )
{
	if( index != invalidIndex )
		battery.vecMachinesGarbage.push_back( index );
	index = invalidIndex;
}
void script_engine::callSub( size_t machineIndex, script_container::sub AtSub )
{
	unsigned prevMachine = currentRunningMachine;
	script_machine & m = getScriptMachine( machineIndex );
	assert( m.current_thread_index != invalidIndex && m.current_script_index != invalidIndex );
	size_t blockIndex = invalidIndex;
	script_container & sc = battery.vecScripts[ m.current_script_index ];
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
void script_engine::setQueueScriptMachine( script_queue const queue )
{
	battery.vecQueuedScripts.push_back( queue );
}

//script engine - public functions, called from the outside
script_engine::script_engine() : error( false ), scriptParser( *this ), currentRunningMachine( invalidIndex )
{
}
void script_engine::cleanEngine()
{
	currentRunningMachine = invalidIndex;
	typeManager = script_type_manager();
	battery = inventory();
}
void script_engine::start()
{
	//map the scripts to individual units to be parsed
	char buff[512] = { 0 };
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
	scriptPath= ofn.lpstrFile;
	if( !scriptPath.size() )
	{
		MessageBox( NULL, "File not selected", "script_engine::start", NULL );
		return;
	}
	parseScriptFromFile( scriptPath );
	if( error ) return;
	size_t scriptIdx = findScriptFromFile( scriptParser.getCurrentScriptPath() );
	if( scriptIdx == invalidIndex ) return;
	currentRunningMachine = fetchScriptMachine();
	script_machine & machine = getScriptMachine( currentRunningMachine );
	machine.initialize( *this, scriptIdx );
}
bool script_engine::advance()
{
	unsigned saved = currentRunningMachine;
	for( unsigned u = 0; u < battery.vecMachines.size(); ++u )
	{
		if( error ) return false;
		currentRunningMachine = u;
		if( battery.vecMachines[ u ].isOperable() )
			callSub( u, script_container::AtMainLoop );
		while( battery.vecQueuedScripts.size() )
		{
			switch( battery.vecQueuedScripts.front().queueType )
			{
			case script_queue::Initialization:
				getScriptMachine( fetchScriptMachine() ).initialize( *this, battery.vecQueuedScripts.front().index ); //script index
				break;
			case script_queue::TerminationMark: //machine index
				break;
			case script_queue::Termination: //machine index
				break;
			}
			battery.vecQueuedScripts.erase( battery.vecQueuedScripts.begin() );
		}
	}
	currentRunningMachine = saved;
	return true;
}
void script_engine::parseScriptFromFile( std::string const & scriptPath )
{
	try
	{
		if( battery.mappedMainScripts.find( scriptPath ) == battery.mappedMainScripts.end() )
			scriptParser.parseScript( scriptPath );
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