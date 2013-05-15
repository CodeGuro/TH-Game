#include <scriptengine.hpp>

script_engine::script_type_manager::script_type_manager()
{
	types.push_back( type_data( type_data::tk_real, invalidIndex ) );
	types.push_back( type_data( type_data::tk_boolean, invalidIndex ) );
	types.push_back( type_data( type_data::tk_char, invalidIndex ) );
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

script_engine::script_engine() : currentRunningMachine( invalidIndex )
{
}
size_t script_engine::fetchBlock()
{
	return battery.vecBlocks.insert( battery.vecBlocks.end(), block() ) - battery.vecBlocks.begin();
}
block & script_engine::getBlock( size_t index )
{
	return battery.vecBlocks[index];
}

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
		battery.vecScriptData.reserve( 1 );
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
void script_engine::addRefScriptData( size_t & index )
{
	if( index != invalidIndex )
		++getScriptData( index ).refCount;
}
void script_engine::releaseScriptData( size_t & index )
{
	if( index != invalidIndex )
	{
		script_data & dat = getScriptData( index );
		if( !(--dat.refCount) )
		{
			for( unsigned i = 0; i < dat.vec.size(); ++i )
				releaseScriptData( dat.vec[i] );
			disposeScriptData( index );
			index = invalidIndex;
		}
	}
}
void script_engine::scriptDataAssign( size_t & dst, size_t & src ) //index copy
{
	addRefScriptData( src );
	releaseScriptData( dst );
	dst = src;
}
void script_engine::copyScriptData( size_t & dst, size_t & src ) //contents copy
{
	if( dst != invalidIndex )
	{
		script_data destDat = getScriptData( dst );
		if( destDat.vec.size() )
		{
			for( unsigned i = 0; i < destDat.vec.size(); ++i )
			{
				
			}
		}
	}
}
void script_engine::uniqueize( size_t & dst )
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
						uniqueize( destDat.vec[i] );
				}
				break;
			}
		}
	}
}
void script_engine::disposeScriptData( size_t index )
{
	battery.vecScriptDataGarbage.push_back( index );
}

size_t script_engine::fetchScriptEnvironment( size_t blockIndex)
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
		battery.vecScriptData.reserve( 1 );
	}
	getScriptEnvironment( index ).blockIndex = blockIndex;
	getScriptEnvironment( index ).codeIndex = 0;
	return index;
}
script_environment & script_engine::getScriptEnvironment( size_t index )
{
	return battery.vecRoutines[ index ];
}
void script_engine::disposeScriptEnvironment( size_t index )
{
	battery.vecRoutinesGabage.push_back( index );
}

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
		battery.vecMachines.reserve( 1 );
	}
	return index;
}
script_machine & script_engine::getScriptMachine( size_t index )
{
	return battery.vecMachines[ index ];
}
void script_engine::disposeScriptMachine( size_t index )
{
	battery.vecMachinesGarbage.push_back( index );
}