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
		return index;
	}
	index = battery.vecScriptData.size();
	battery.vecScriptData.reserve( 1 );
	return index;
}
size_t script_engine::fetchScriptData( float real )
{
	size_t index = fetchScriptData();
	battery.vecScriptData[ index ].real = real;
	battery.vecScriptData[ index ].refCount = 1;
	battery.vecScriptData[ index ].type = typeManager.getRealType();
	return index;
}
size_t script_engine::fetchScriptData( char character )
{
	size_t index = fetchScriptData();
	battery.vecScriptData[ index ].character = character;
	battery.vecScriptData[ index ].refCount = 1;
	battery.vecScriptData[ index ].type = typeManager.getCharacterType();
	return index;
}
size_t script_engine::fetchScriptData( bool boolean )
{
	size_t index = fetchScriptData();
	battery.vecScriptData[ index ].real = (float)boolean;
	battery.vecScriptData[ index ].refCount = 1;
	battery.vecScriptData[ index ].type = typeManager.getBooleanType();
	return index;
}
size_t script_engine::fetchScriptData( std::string const & string )
{
	size_t index = fetchScriptData();
	battery.vecScriptData[ index ].refCount = 1;
	battery.vecScriptData[ index ].type = typeManager.getStringType();
	for( unsigned i = 0; i < string.size(); ++i )
		battery.vecScriptData[ index ].vec.push_back( fetchScriptData( string[i] ) );
	return index;
}