#include <inventory.hpp>
#include <sstream>
#include <assert.h>
#include <scriptmachine.hpp>

//script type manager, script_engine::getScriptTypeManager
script_type_manager::script_type_manager()
{
	types.push_back( type_data( type_data::tk_real, -1 ) );
	types.push_back( type_data( type_data::tk_boolean, -1 ) );
	types.push_back( type_data( type_data::tk_char, -1 ) );
	types.push_back( type_data( type_data::tk_array, -1) );
	types.push_back( type_data( type_data::tk_array, 2 ) );
	types.push_back( type_data( type_data::tk_object, -1 ) );
	types.push_back( type_data( type_data::tk_misc, -1 ) );
}
type_data script_type_manager::getRealType() const
{
	return type_data( type_data::tk_real, -1 );
}
type_data script_type_manager::getBooleanType() const
{
	return type_data( type_data::tk_boolean, -1 );
}
type_data script_type_manager::getCharacterType() const
{
	return type_data( type_data::tk_char, -1 );
}
type_data script_type_manager::getStringType() const
{
	return type_data( type_data::tk_array, getCharacterType().get_kind() );
}
type_data script_type_manager::getObjectType() const
{
	return type_data( type_data::tk_object, -1 );
}
type_data script_type_manager::getMiscType() const
{
	return type_data( type_data::tk_misc, -1 );
}
type_data script_type_manager::getArrayType() const
{
	return type_data( type_data::tk_array, -1 );
}
type_data script_type_manager::getArrayType( size_t element )
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
size_t inventory::fetchBlock()
{
	size_t index = vecBlocks.size();
	vecBlocks.push_back( block() );
	return index;
}
block & inventory::getBlock( size_t index )
{
	return vecBlocks[index];
}
void inventory::registerScript( std::string const scriptName )
{
	mappedScripts[ scriptName ] = vecScripts.size();
	script_container new_cont;
	memset( &new_cont, -1, sizeof( new_cont ) );
	vecScripts.push_back( new_cont );
}
void inventory::registerMainScript( std::string const scriptPath, std::string const scriptName )
{
	mappedMainScripts[ scriptPath ] = vecScripts.size();
	registerScript( scriptName );
}
void inventory::registerInvalidMainScript( std::string const scriptPath )
{
	mappedMainScripts[ scriptPath ] = -1;
}
script_container * inventory::getScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = mappedScripts.find( scriptName );
	if( it != mappedScripts.end() )
		return &(vecScripts[ it->second ]);
	return 0;
}
script_container & inventory::getScript( size_t index )
{
	return vecScripts[ index ];
}
size_t inventory::findScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = mappedScripts.find( scriptName );
	if( it != mappedScripts.end() )
		return it->second;
	return -1;
}
size_t inventory::findScriptFromFile( std::string const & scriptPath )
{
	std::map< std::string, unsigned >::iterator it = mappedMainScripts.find( scriptPath );
	if( it != mappedMainScripts.end() )
		return it->second;
	return -1;
}
size_t inventory::findScriptDirectory( std::string const & scriptPath )
{
	size_t u;
	std::string path = scriptPath;
	do
		path.pop_back();
	while( !(path.back() == '\\' || path.back() =='/') );

	for( u = 0; u < vecScriptDirectories.size(); ++u )
	{
		if( vecScriptDirectories[ u ] == path )
			return u;
	}
	vecScriptDirectories.push_back( path );
	return u;
}
std::string const & inventory::getCurrentScriptDirectory( size_t machineIdx ) const
{
	return vecScriptDirectories[ vecScripts[ vecMachines[ machineIdx ].getScriptIndex() ].ScriptDirectory ];
}

//script engine - script data - related functions
size_t inventory::fetchScriptData()
{
	size_t index;
	if( vecScriptDataGarbage.size() )
	{
		index = vecScriptDataGarbage.back();
		vecScriptDataGarbage.pop_back();
	}
	else
	{
		index = vecScriptData.size();
		vecScriptData.resize( 1 + index );
	}
	getScriptData( index ).refCount = 1;
	return index;
}
size_t inventory::fetchScriptData( float real )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = real;
	data.type = getRealType();
	return index;
}
size_t inventory::fetchScriptData( char character )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.character = character;
	data.type = getCharacterType();
	return index;
}
size_t inventory::fetchScriptData( bool boolean )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = (float)boolean;
	data.type = getBooleanType();
	return index;
}
size_t inventory::fetchScriptData( std::string const & string )
{
	size_t index = fetchScriptData();
	getScriptData( index ).type = getStringType();
	for( unsigned i = 0; i < string.size(); ++i )
		getScriptData( index ).vec.push_back( fetchScriptData( string[i] ) );
	return index;
}
size_t inventory::fetchScriptData( ObjType typeobj, size_t machineIdx )
{
	if( !CheckValidIdx( machineIdx ) || !CheckValidIdx( getScriptMachine( machineIdx ).getObjectVectorIndex() ) )
		return -1;
	//let objParam be the object type, 4 = bullet, 5 = effect
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	auto const objIdx = CreateObject( typeobj );
	data.objIndex = objIdx;
	data.type = getObjectType();
	auto const objvector = getScriptMachine( machineIdx ).getObjectVectorIndex();
	AddRefObjHandle( objIdx );
	vvecObjects[ objvector ].push_back( objIdx );
	Object * obj = GetObject( objIdx );
	return index;
}
size_t inventory::fetchScriptData( D3DPRIMITIVETYPE primType )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.primitiveType = primType;
	getScriptData( index ).type = getMiscType();
	return index;
}
size_t inventory::fetchScriptData( BlendType blend )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.blendMode = blend;
	data.type = getMiscType();
	return index;
}
size_t inventory::fetchScriptData( ObjType typeobj )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.objtype = typeobj;
	data.type = getMiscType();
	return index;
}
script_data & inventory::getScriptData( size_t index )
{
	return vecScriptData[ index ];
}
void inventory::addRefScriptData( size_t index )
{
	if( CheckValidIdx( index ) )
		++getScriptData( index ).refCount;
}
void inventory::releaseScriptData( size_t & index )
{
	if( CheckValidIdx( index ) )
	{
		script_data & dat = getScriptData( index );
		if( !(--dat.refCount) )
		{
			for( unsigned i = 0; i < dat.vec.size(); ++i )
				releaseScriptData( dat.vec[ i ] );
			dat.vec.resize( 0 );
			vecScriptDataGarbage.push_back( index );
			if( dat.type.get_kind() == getObjectType().get_kind() )
				ReleaseObjHandle( dat.objIndex );
		}
		index = -1;
	}
}
void inventory::scriptDataAssign( size_t & dst, size_t src ) //index copy
{
	addRefScriptData( src );
	releaseScriptData( dst );
	dst = src;
}
void inventory::copyScriptData( size_t & dst, size_t & src ) //contents copy, including vector
{
	if( !CheckValidIdx( dst ) )
		dst = fetchScriptData();
	script_data & destDat = getScriptData( dst );

	for( unsigned i = 0; i < destDat.vec.size(); ++i )
		releaseScriptData( destDat.vec[i] );
	destDat.vec.resize( 0 );

	if( CheckValidIdx( src ) )
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
			AddRefObjHandle( sourDat.objIndex );
			destDat.objIndex = sourDat.objIndex;
			break;
		case type_data::tk_array:
			{
				destDat.vec.resize( sourDat.vec.size() );
				for( unsigned i = 0; i < sourDat.vec.size(); ++i )
					(!CheckValidIdx( sourDat.vec[i] )) ? (destDat.vec[i] = -1) : (copyScriptData( ( destDat.vec[i] = fetchScriptData() ), sourDat.vec[i] ));
			}
			break;
		}
	}
}
void inventory::uniqueizeScriptData( size_t & dst )
{
	if( CheckValidIdx( dst ) )
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
				AddRefObjHandle( getScriptData( tmpDst ).objIndex );
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
std::string inventory::getStringScriptData( size_t index )
{
	std::string result;
	if( CheckValidIdx( index ) )
	{
		script_data const & dat = getScriptData( index );
		switch( dat.type.get_kind() )
		{
			case type_data::tk_array:
			{
				if( dat.type.get_element() != getStringType().get_element() )
					result += "[ ";
				unsigned size = dat.vec.size();
				for( unsigned i = 0; i < size; ++i )
				{
					result += getStringScriptData( dat.vec[ i ] );
					if( getScriptData( dat.vec[ i ] ).type.kind != getCharacterType().kind && i + 1 < size )
						result += " , ";
				}
				if( dat.type.get_element() != getStringType().get_element()  )
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
float inventory::getRealScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].real;
	return -1;
}
bool inventory::getBooleanScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].real != 0;
	return true;

}
char inventory::getCharacterScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].character;
	return -1;
}
unsigned inventory::getObjHandleScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].objIndex;
	return -1;
}
D3DPRIMITIVETYPE inventory::getPrimitiveTypeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].primitiveType;
	return (D3DPRIMITIVETYPE)-1;
}
BlendType inventory::getBlendModeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].blendMode;
	return (BlendType)-1;
}
ObjType inventory::getObjTypeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].objtype;
	return (ObjType)-1;
}

//script engine - script environment - related functions
size_t inventory::fetchScriptEnvironment( size_t blockIndex )
{
	size_t index;
	if( vecRoutinesGabage.size() )
	{
		index = vecRoutinesGabage.back();
		vecRoutinesGabage.pop_back();
	}
	else
	{
		index = vecScriptEnvironment.size();
		vecScriptEnvironment.resize( 1 + index );
	}
	script_environment & env = getScriptEnvironment( index );
	env.blockIndex = blockIndex;
	env.codeIndex = 0;
	env.refCount = 1;
	return index;
}
script_environment & inventory::getScriptEnvironment( size_t index )
{
	return vecScriptEnvironment[ index ];
}
void inventory::addRefScriptEnvironment( size_t index )
{
	if( CheckValidIdx( index ) )
		++getScriptEnvironment( index ).refCount;
}
void inventory::releaseScriptEnvironment( size_t & index )
{
	if( CheckValidIdx( index ) )
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
			vecRoutinesGabage.push_back( index );
		}
		index = -1;
	}
}

//script engine - script machine - related functions
size_t inventory::fetchScriptMachine()
{
	size_t index;
	index = vecMachines.size();
	vecMachines.resize( 1 + index );
	return index;
}
script_machine & inventory::getScriptMachine( size_t index )
{
	return vecMachines[ index ];
}
void inventory::releaseScriptMachine( size_t & index )
{
	vecMachines.erase( vecMachines.begin() + index );
	index = -1;
}
size_t inventory::fetchObjectVector()
{
	size_t res;
	if( vvecObjectsGarbage.size() )
	{
		res = vvecObjectsGarbage.back();
		vvecObjectsGarbage.pop_back();
	}
	else
	{
		res = vvecObjects.size();
		vvecObjects.resize( 1 + res );
	}
	return res;
}
void inventory::releaseObjectVector( size_t & index )
{
	if( !CheckValidIdx( index ) )
		return;
	auto & objvec = vvecObjects[ index ];
	unsigned s = objvec.size();
	for( unsigned u = 0; u < s; ++u )
	{
		ReleaseObject( objvec[ u ] );
		ReleaseObjHandle( objvec[ u ] );
	}
	objvec.resize( 0 );
	vvecObjectsGarbage.push_back( index );
	index = -1;
}
void inventory::latchScriptObjectToMachine( size_t index, size_t machineIdx )
{
	size_t objHandle = getObjHandleScriptData( index );
	if( !CheckValidIdx( objHandle )  || !CheckValidIdx( machineIdx ) )
		return;
	script_machine & machine = getScriptMachine( machineIdx );
	size_t objvec = machine.getObjectVectorIndex();
	if( !CheckValidIdx( objvec ) )
		return;
	auto & vec = vvecObjects[ objvec ];
	for( unsigned u = 0; u < vec.size(); ++u )
		if( vec[ u ] == objHandle )
		{
			machine.latchObject( u );
			break;
		}
}

//script engine - other
void inventory::cleanInventory( class script_engine & eng )
{
	for( unsigned u = 0; u < vecMachines.size(); ++u )
		vecMachines[ u ].clean( eng );
	inventory_mem * this_inv = this;
	*this_inv = inventory_mem();
}
Object * inventory::getObjFromScriptVector( size_t objvector, size_t Idx )
{
	if( CheckValidIdx( objvector ) && CheckValidIdx( Idx ) )
		return Battery::GetObject( vvecObjects[ objvector ][ Idx ] );
	return NULL;
}
unsigned inventory::getMachineCount() const
{
	return vecMachines.size();
}