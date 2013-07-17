#include <scriptengine.hpp>
#include <parser.hpp>
#include <assert.h>

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
	battery.mappedScriptBlocks[ scriptName ] = script_container();
}
script_container * script_engine::getScript( std::string const & scriptName )
{
	std::map< std::string, script_container >::iterator it = battery.mappedScriptBlocks.find( scriptName );
	if( it != battery.mappedScriptBlocks.end() )
		return &(it->second);
	return 0;
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
void script_engine::addRefScriptData( size_t & index ) //interface function
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
			battery.vecScriptDataGarbage.push_back( index );
		}
		index = invalidIndex;
	}
}
void script_engine::scriptDataAssign( size_t & dst, size_t & src ) //index copy
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
		script_data & destDat = getScriptData( dst );

		if( destDat.refCount > 1 )
		{
			releaseScriptData( dst );
			dst = fetchScriptData(); //unique
			script_data & uniqDat = getScriptData( dst );
			switch( (uniqDat.type = destDat.type).get_kind() )
			{
			case type_data::tk_real:
			case type_data::tk_boolean:
				uniqDat.real = destDat.real;
				break;
			case type_data::tk_char:
				uniqDat.character = destDat.character;
				break;
			case type_data::tk_object:
				uniqDat.objIndex = destDat.objIndex;
				break;
			case type_data::tk_array:
				{
					for( unsigned i = 0; i < destDat.vec.size(); ++i )
						uniqueizeScriptData( destDat.vec[i] );
				}
				break;
			}
		}
	}
	else
		getScriptData( dst = fetchScriptData() ).type.kind = type_data::tk_invalid;
}
std::string script_engine::getStringScriptData( size_t index )
{
	script_data const & dat = getScriptData( index );
	unsigned size = dat.vec.size();
	std::string result;
	for( unsigned i = 0; i < size; ++i)
		result += getScriptData( dat.vec[ i ] ).character;
	return result;
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
		index = battery.vecScriptData.size();
		battery.vecScriptData.resize( 1 + index );
	}
	script_environment & env = getScriptEnvironment( index );
	env.blockIndex = blockIndex;
	env.codeIndex = 0;
	env.refCount = 1;
	return index;
}
script_environment & script_engine::getScriptEnvironment( size_t index )
{
	return battery.vecRoutines[ index ];
}
void script_engine::addRefScriptEnvironment( size_t & index )
{
	if( index != invalidIndex )
		++getScriptEnvironment( index ).refCount;
}
void script_engine::releaseScriptEnvironment( size_t & index )
{
	if( index != invalidIndex )
	{
		script_environment & env = getScriptEnvironment( index );
		if( !(--env.refCount) )
		{
			for( unsigned i = 0; i < env.stack.size(); ++i )
				releaseScriptData( env.stack[i] );
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
script_engine::inventory::script_machine_container & script_engine::getScriptMachine( size_t index )
{
	return battery.vecMachines[ index ];
}
void script_engine::releaseScriptMachine( size_t & index )
{
	if( index != invalidIndex )
		battery.vecMachinesGarbage.push_back( index );
	index = invalidIndex;
}

//script engine - public functions, called from the outside
script_engine::script_engine() : currentRunningMachine( invalidIndex )
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
	parser p(*this);
}
script_engine::~script_engine()
{
}