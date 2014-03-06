#include <scriptengine.hpp>
#include <parser.hpp>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <Windows.h>
#include <ObjMgr.hpp>

//exception
eng_exception::eng_exception() : throw_reason( eng_error )
{
}
eng_exception::eng_exception( Reason const r ) : throw_reason( r )
{
}
eng_exception::eng_exception( Reason const r, std::string const & String ) : throw_reason( r ), Str( String )
{
}

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
		if( types[ i ].get_kind() == type_data::tk_array && types[ i ].get_element() == element )
			return types[ i ];
	}
	//else
	return *(types.insert( types.end(), type_data( type_data::tk_array, element ) ));
}


//script engine block-related functions
size_t script_engine::fetchBlock()
{
	size_t index = vecBlocks.size();
	vecBlocks.push_back( block() );
	return index;
}
block & script_engine::getBlock( size_t index )
{
	return vecBlocks[index];
}
void script_engine::registerScript( std::string const scriptName )
{
	mappedScripts[ scriptName ] = vecScripts.size();
	script_container new_cont;
	memset( &new_cont, -1, sizeof( new_cont ) );
	vecScripts.push_back( new_cont );
}
void script_engine::registerMainScript( std::string const scriptPath, std::string const scriptName )
{
	mappedMainScripts[ scriptPath ] = vecScripts.size();
	registerScript( scriptName );
}
void script_engine::registerInvalidMainScript( std::string const scriptPath )
{
	mappedMainScripts[ scriptPath ] = -1;
}
script_container * script_engine::getScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = mappedScripts.find( scriptName );
	if( it != mappedScripts.end() )
		return &(vecScripts[ it->second ]);
	return 0;
}
script_container & script_engine::getScript( size_t index )
{
	return vecScripts[ index ];
}
size_t script_engine::findScript( std::string const & scriptName )
{
	std::map< std::string, unsigned >::iterator it = mappedScripts.find( scriptName );
	if( it != mappedScripts.end() )
		return it->second;
	return -1;
}
size_t script_engine::findScriptFromFile( std::string const & scriptPath )
{
	std::map< std::string, unsigned >::iterator it = mappedMainScripts.find( scriptPath );
	if( it != mappedMainScripts.end() )
		return it->second;
	return -1;
}
size_t script_engine::findScriptDirectory( std::string const & scriptPath )
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
std::string const & script_engine::getCurrentScriptDirectory( size_t machineIdx ) const
{
	return vecScriptDirectories[ vecScripts[ vecContexts[ machineIdx ].current_script_index ].ScriptDirectory ];
}

//script engine - script data - related functions
size_t script_data_manager::fetchScriptData()
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
size_t script_data_manager::fetchScriptData( float real )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = real;
	data.type = type_mgr.getRealType();
	return index;
}
size_t script_data_manager::fetchScriptData( char character )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.character = character;
	data.type = type_mgr.getCharacterType();
	return index;
}
size_t script_data_manager::fetchScriptData( bool boolean )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.real = (float)boolean;
	data.type = type_mgr.getBooleanType();
	return index;
}
size_t script_data_manager::fetchScriptData( std::string const & string )
{
	size_t index = fetchScriptData();
	getScriptData( index ).type = type_mgr.getStringType();
	for( unsigned i = 0; i < string.size(); ++i )
		getScriptData( index ).vec.push_back( fetchScriptData( string[ i ] ) );
	return index;
}
size_t script_data_manager::fetchScriptData( ObjType typeobj, size_t machineIdx )
{
	if( !CheckValidIdx( machineIdx ) || !CheckValidIdx( eng->getScriptContext( machineIdx )->object_vector_index ) )
		return -1;
	//let objParam be the object type, 4 = bullet, 5 = effect
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	auto const objIdx = draw_mgr->CreateObject( typeobj );
	data.objIndex = objIdx;
	data.type = type_mgr.getObjectType();
	auto const objvector = eng->getScriptContext( machineIdx )->object_vector_index;
	draw_mgr->AddRefObjHandle( objIdx );
	eng->vvecObjects[ objvector ].push_back( objIdx );
	Object * obj = draw_mgr->GetObject( objIdx );
	return index;
}
size_t script_data_manager::fetchScriptData( D3DPRIMITIVETYPE primType )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.primitiveType = primType;
	getScriptData( index ).type = type_mgr.getMiscType();
	return index;
}
size_t script_data_manager::fetchScriptData( BlendType blend )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.blendMode = blend;
	data.type = type_mgr.getMiscType();
	return index;
}
size_t script_data_manager::fetchScriptData( ObjType typeobj )
{
	size_t index;
	script_data & data = getScriptData( index = fetchScriptData() );
	data.objtype = typeobj;
	data.type = type_mgr.getMiscType();
	return index;
}
script_data & script_data_manager::getScriptData( size_t index )
{
	return vecScriptData[ index ];
}
void script_data_manager::addRefScriptData( size_t index )
{
	if( CheckValidIdx( index ) )
		++getScriptData( index ).refCount;
}
void script_data_manager::releaseScriptData( size_t & index )
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
			if( dat.type.get_kind() == type_mgr.getObjectType().get_kind() )
				draw_mgr->ReleaseObjHandle( dat.objIndex );
		}
		index = -1;
	}
}
void script_data_manager::scriptDataAssign( size_t & dst, size_t src ) //index copy
{
	addRefScriptData( src );
	releaseScriptData( dst );
	dst = src;
}
void script_data_manager::copyScriptData( size_t & dst, size_t & src ) //contents copy, including vector
{
	if( !CheckValidIdx( dst ) )
		dst = fetchScriptData();
	script_data & destDat = getScriptData( dst );

	for( unsigned i = 0; i < destDat.vec.size(); ++i )
		releaseScriptData( destDat.vec[ i ] );
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
			draw_mgr->AddRefObjHandle( sourDat.objIndex );
			destDat.objIndex = sourDat.objIndex;
			break;
		case type_data::tk_array:
			{
				destDat.vec.resize( sourDat.vec.size() );
				for( unsigned i = 0; i < sourDat.vec.size(); ++i )
					(!CheckValidIdx( sourDat.vec[ i ] )) ? (destDat.vec[ i ] = -1) : (copyScriptData( ( destDat.vec[ i ] = fetchScriptData() ), sourDat.vec[ i ] ));
			}
			break;
		}
	}
}
void script_data_manager::uniqueizeScriptData( size_t & dst )
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
				draw_mgr->AddRefObjHandle( getScriptData( tmpDst ).objIndex );
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
std::string script_data_manager::getStringScriptData( size_t index )
{
	std::string result;
	if( CheckValidIdx( index ) )
	{
		script_data const & dat = getScriptData( index );
		switch( dat.type.get_kind() )
		{
			case type_data::tk_array:
			{
				if( dat.type.get_element() != type_mgr.getStringType().get_element() )
					result += "[ ";
				unsigned size = dat.vec.size();
				for( unsigned i = 0; i < size; ++i )
				{
					result += getStringScriptData( dat.vec[ i ] );
					if( getScriptData( dat.vec[ i ] ).type.kind != type_mgr.getCharacterType().kind && i + 1 < size )
						result += " , ";
				}
				if( dat.type.get_element() != type_mgr.getStringType().get_element()  )
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
float script_data_manager::getRealScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].real;
	return -1;
}
bool script_data_manager::getBooleanScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].real != 0;
	return true;

}
char script_data_manager::getCharacterScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].character;
	return -1;
}
unsigned script_data_manager::getObjHandleScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].objIndex;
	return -1;
}
D3DPRIMITIVETYPE script_data_manager::getPrimitiveTypeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].primitiveType;
	return (D3DPRIMITIVETYPE)-1;
}
BlendType script_data_manager::getBlendModeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].blendMode;
	return (BlendType)-1;
}
ObjType script_data_manager::getObjTypeScriptData( size_t index ) const
{
	if( CheckValidIdx( index ) )
		return vecScriptData[ index ].objtype;
	return (ObjType)-1;
}

//script engine - script environment - related functions
size_t script_engine::fetchScriptEnvironment( size_t blockIndex )
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
script_environment & script_engine::getScriptEnvironment( size_t index )
{
	return vecScriptEnvironment[ index ];
}
void script_engine::addRefScriptEnvironment( size_t index )
{
	if( CheckValidIdx( index ) )
		++getScriptEnvironment( index ).refCount;
}
void script_engine::releaseScriptEnvironment( size_t & index )
{
	if( CheckValidIdx( index ) )
	{
		script_environment & env = getScriptEnvironment( index );
		if( !( --env.refCount ) )
		{
			for( unsigned i = 0; i < env.stack.size(); ++i )
				scriptdata_mgr.releaseScriptData( env.stack[ i ] );
			for( unsigned u = 0; u < env.values.size(); ++u )
				scriptdata_mgr.releaseScriptData( env.values[ u ] );
			env.stack.resize( 0 );
			env.values.resize( 0 );
			vecRoutinesGabage.push_back( index );
		}
		index = -1;
	}
}

//script engine - script machine - related functions
size_t script_engine::fetchScriptContext()
{
	size_t index;
	index = vecContexts.size();
	vecContexts.resize( 1 + index );
	return index;
}
script_engine::script_context * script_engine::getScriptContext( size_t index )
{
	return &vecContexts[ index ];
}
void script_engine::releaseScriptContext( size_t & index )
{
	vecContexts.erase( vecContexts.begin() + index );
	index = -1;
}
size_t script_engine::fetchObjectVector()
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
void script_engine::releaseObjectVector( size_t & index )
{
	if( !CheckValidIdx( index ) )
		return;
	auto & objvec = vvecObjects[ index ];
	unsigned s = objvec.size();
	for( unsigned u = 0; u < s; ++u )
	{
		get_drawmgr()->ReleaseObject( objvec[ u ] );
		get_drawmgr()->ReleaseObjHandle( objvec[ u ] );
	}
	objvec.resize( 0 );
	vvecObjectsGarbage.push_back( index );
	index = -1;
}
void script_engine::latchScriptObjectToMachine( size_t index, size_t machineIdx )
{
	size_t objHandle = scriptdata_mgr.getObjHandleScriptData( index );
	if( !CheckValidIdx( objHandle )  || !CheckValidIdx( machineIdx ) )
		return;
	script_context & machine = *getScriptContext( machineIdx );
	size_t objvec = machine.object_vector_index;
	if( !CheckValidIdx( objvec ) )
		return;
	auto & vec = vvecObjects[ objvec ];
	for( unsigned u = 0; u < vec.size(); ++u )
		if( vec[ u ] == objHandle )
		{
			machine.script_object = u;
			break;
		}
}

//script engine - other
Object * script_engine::getObjFromScriptVector( size_t objvector, size_t Idx )
{
	if( CheckValidIdx( objvector ) && CheckValidIdx( Idx ) )
		return get_drawmgr()->GetObject( vvecObjects[ objvector ][ Idx ] );
	return NULL;
}
unsigned script_engine::getContextCount() const
{
	return vecContexts.size();
}

//script engine - public functions, called from the outside
script_engine::script_engine( Direct3DEngine * draw_mgr ) : scriptdata_mgr( draw_mgr, this ), draw_mgr( draw_mgr ), error( false ), finished( false ), currentRunningMachine( -1 )
{
}
void script_engine::cleanEngine()
{
	for( unsigned u = 0; u < vecContexts.size(); ++u )
		clean_script_context( u );
	*this = script_engine( get_drawmgr() );

	currentRunningMachine = -1;
	error = false;
	errorMessage.clear();
}
bool script_engine::start()
{
	try
	{
		//map the scripts to individual units to be parsed
		char buff[ 1024 ] = { 0 };
		GetCurrentDirectory( sizeof( buff ), buff );
		std::string const path = std::string( buff ) + "\\script";
		parser script_parser( this );
		std::string scriptPath;
		OPENFILENAMEA ofn ={ 0 };
		char buff2[ 1024 ] ={ 0 };
		ofn.lStructSize = sizeof( OPENFILENAMEA );
		ofn.lpstrFilter = "All Files\0*.*\0\0";
		ofn.lpstrFile = buff2;
		ofn.nMaxFile = sizeof( buff2 );
		ofn.lpstrTitle = "Open script...";
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST ;
		GetOpenFileNameA( &ofn );
		SetCurrentDirectory( buff );
		scriptPath= ofn.lpstrFile;
		if( !scriptPath.size() )
		{
			MessageBox( NULL, "Script not chosen, terminating...", "Engine Error", MB_TASKMODAL );
			return false;
		}
		script_parser.parseScript( scriptPath );
		size_t script_index = findScriptFromFile( script_parser.getCurrentScriptPath() );
		if( !CheckValidIdx( script_index ) ) raise_exception( eng_exception( eng_exception::eng_error ) );

		script_context * context = getScriptContext( fetchScriptContext() );
		context->current_script_index = script_index;
		context->object_vector_index = fetchObjectVector();
		context->script_object = -1;
	}
	catch( eng_exception const & error )
	{
		if( error.throw_reason == eng_exception::eng_error )
		{
			if( error.Str.size() )
				MessageBox( NULL, error.Str.c_str(), "Engine Error", MB_TASKMODAL );
			cleanEngine();
			return start();
		}
	}
	return true;
}
bool script_engine::run()
{
	currentRunningMachine = 0;
	get_drawmgr()->UpdateObjectCollisions();
	while( currentRunningMachine < vecContexts.size() )
	{
		try
		{
			for(; currentRunningMachine < vecContexts.size(); ++currentRunningMachine )
			{
				if( error ) return false;
				script_context const & machine = *getScriptContext( currentRunningMachine );
				Object * current = getObjFromScriptVector( machine.object_vector_index, machine.script_object );
				if( current )
				{
					if( current->FlagCollision( -1 ) )
						callSub( currentRunningMachine, script_container::AtHit );
				}
				callSub( currentRunningMachine, script_container::AtMainLoop );
			}
		}
		catch( eng_exception const & e )
		{
			switch( e.throw_reason )
			{
			case eng_exception::eng_error:
				{
					cleanEngine();
					start();
				}
				break;
			}
		}
	}
	return true;
}
Direct3DEngine * script_engine::get_drawmgr()
{
	return draw_mgr;
}

void script_engine::callSub( size_t machineIndex, script_container::sub AtSub )
{
	unsigned prevMachine = currentRunningMachine;
	currentRunningMachine = machineIndex;
	script_context & m = *getScriptContext( machineIndex );
	assert( CheckValidIdx( m.current_thread_index ) && CheckValidIdx( m.current_script_index ) );
	size_t blockIndex = -1;
	script_container & sc = getScript( m.current_script_index );
	//initializing
	if( !m.threads.size() )
	{
		m.threads.push_back( fetchScriptEnvironment( getScript( m.current_script_index ).ScriptBlock ) );
		getScriptEnvironment( m.threads[ 0 ] ).parentIndex = -1;
		getScriptEnvironment( m.threads[ 0 ] ).hasResult = false;
		m.current_thread_index = 0;
		while( !advance() );
		callSub( currentRunningMachine, script_container::AtInitialize );
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
		{
			//check to see if latched object (if there is one) is valid
			if( CheckValidIdx( getScriptContext( machineIndex )->script_object ) && 
				CheckValidIdx( getScriptContext( machineIndex )->object_vector_index  ) )
			{
				Object * obj = getObjFromScriptVector( getScriptContext( machineIndex )->object_vector_index, getScriptContext( machineIndex )->script_object ); // GetObject( vvecObjects[ getScriptMachine( machineIndex ).getObjectVectorIndex() ][ getScriptMachine( machineIndex ).getLatchedObject() ] );
				if( !obj )
				{
					//latched object has been deleted, terminate machine
					callSub( machineIndex, script_container::AtFinalize );
					clean_script_context( machineIndex );
					releaseScriptContext( machineIndex );
					raise_exception( eng_exception( eng_exception::finalizing_machine ) );
					break;
				}
			}
			blockIndex = sc.MainLoopBlock;
		}
		break;
	case script_container::AtBackGround:
		blockIndex = sc.BackGroundBlock;
		break;
	case script_container::AtHit:
		blockIndex = sc.HitBlock;
	}
	if( CheckValidIdx( blockIndex ) )
	{
		++getScriptEnvironment( m.threads[ m.current_thread_index ] ).refCount;
		size_t calledEnv = fetchScriptEnvironment( blockIndex );
		script_environment & e = getScriptEnvironment( calledEnv );
		e.parentIndex = m.threads[ m.current_thread_index ];
		e.hasResult = 0;
		m.threads[ m.current_thread_index ] = calledEnv;
		while( !advance() );
	}

	currentRunningMachine = prevMachine;
}
void script_engine::raise_exception( eng_exception const & eng_except )
{
	throw eng_except;
}
bool script_engine::IsFinished()
{
	return finished;
}

void script_engine::initialize_script_context( size_t script_index, size_t context_index )
{
	script_context * context = getScriptContext( context_index );
	context->current_script_index = script_index;
	context->object_vector_index = fetchObjectVector();
	context->script_object = -1;
	callSub( context_index, script_container::AtInitialize );
}
bool script_engine::advance()
{
	script_context * current_context = getScriptContext( currentRunningMachine );
	assert( current_context->threads.size() > current_context->current_thread_index );
	script_environment & env = getScriptEnvironment( current_context->threads[ current_context->current_thread_index ] );
	if( env.codeIndex >= getBlock( env.blockIndex ).vecCodes.size() )
	{
		size_t disposing = current_context->threads[ current_context->current_thread_index ];
		if( !CheckValidIdx( getScriptEnvironment( disposing ).parentIndex ) )
			return true; //do not dispose initial environment
		script_environment & disposing_env = getScriptEnvironment( current_context->threads[ current_context->current_thread_index ] );
		current_context->threads[ current_context->current_thread_index ] = disposing_env.parentIndex;

		if( disposing_env.hasResult )
		{
			scriptdata_mgr.addRefScriptData( disposing_env.values[ 0 ] );
			getScriptEnvironment( disposing_env.parentIndex ).stack.push_back( disposing_env.values[ 0 ] );
		}
		else if( getBlock( disposing_env.blockIndex ).kind == block::bk_task )
			current_context->threads.erase( current_context->threads.begin() + current_context->current_thread_index-- );

		do
		{
			unsigned refs = getScriptEnvironment( disposing ).refCount;
			size_t next = getScriptEnvironment( disposing ).parentIndex;
			releaseScriptEnvironment( disposing );
			if( refs > 1 )
				break;
			disposing = next;
		}while( true );
		return false;
	}
	code const & current_code = getBlock( env.blockIndex ).vecCodes[ env.codeIndex++ ];
	switch( current_code.command )
	{
	case vc_assign:
		{
			script_environment * e;
			for( e = &env; e->blockIndex != current_code.blockIndex; e = &getScriptEnvironment( e->parentIndex ) );
			if( e->values.size() <= current_code.variableIndex )
				e->values.resize( 4 + 2 * e->values.size(), -1 );
			scriptdata_mgr.scriptDataAssign( e->values[ current_code.variableIndex ], env.stack.back() );
			scriptdata_mgr.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
		}
		break;
	case vc_overWrite:
		{
			scriptdata_mgr.copyScriptData( env.stack[ env.stack.size() - 2 ], env.stack.back() );
			scriptdata_mgr.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
		}
		break;
	case vc_pushVal:
		{
			env.stack.push_back( -1 );
			scriptdata_mgr.scriptDataAssign( env.stack.back(), current_code.scriptDataIndex );
		}
		break;
	case vc_pushVar:
		{
			script_environment * e;
			for( e = &env; e->blockIndex != current_code.blockIndex; e = &getScriptEnvironment( e->parentIndex ) );
			env.stack.push_back( -1 );
			scriptdata_mgr.scriptDataAssign( env.stack.back(), e->values[ current_code.variableIndex ] );
		}
		break;
	case vc_duplicate:
		{
			scriptdata_mgr.addRefScriptData( env.stack.back() );
			env.stack.push_back( env.stack.back() );
		}
		break;
	case vc_callFunction:
	case vc_callFunctionPush:
	case vc_callTask:
		{
			block & b = getBlock( current_code.subIndex );
			if( b.nativeCallBack ) //always functional
			{
				env.stack.push_back( -1 );
				unsigned popCount = current_code.argc + (current_code.command != vc_callFunctionPush ? 1 : 0 );
				b.nativeCallBack( this, &env.stack[ env.stack.size() - ( 1 + current_code.argc ) ] );
				{
					script_context & current_machine = *getScriptContext( currentRunningMachine );
					script_environment & env = getScriptEnvironment( current_machine.threads[ current_machine.current_thread_index ] ); //in case of re-allocation
					for( unsigned u = 0; u < popCount; ++ u )
					{
						scriptdata_mgr.releaseScriptData( env.stack.back() );
						env.stack.pop_back();
					}
				}
			}
			else
			{
				size_t envIdx = fetchScriptEnvironment( current_code.subIndex );
				script_environment & current_env = getScriptEnvironment( current_context->threads[ current_context->current_thread_index ] );
				script_environment & new_env = getScriptEnvironment( envIdx );
				new_env.hasResult = ( current_code.command == vc_callFunctionPush );
				new_env.parentIndex = current_context->threads[ current_context->current_thread_index ];
				++current_env.refCount;
				for( unsigned u = 0; u < current_code.argc; ++ u )
					new_env.stack.push_back( *(current_env.stack.end() - 1 - u) );
				current_env.stack.erase( current_env.stack.end() - current_code.argc, current_env.stack.end() );
				if( current_code.command != vc_callTask )
					current_context->threads[ current_context->current_thread_index ] = envIdx;
				else
					current_context->threads.insert( current_context->threads.begin() + ++current_context->current_thread_index, envIdx );
			}
		}
		break;
	case vc_breakRoutine:
		{
			bool BaseRoutine = false; //task/function returns
			script_environment * e = &env;
			do
			{
				block const & b = getBlock( e->blockIndex );
				e->codeIndex = b.vecCodes.size();
				if( !(b.kind == block::bk_function || b.kind == block::bk_task || b.kind == block::bk_sub) && CheckValidIdx( e->parentIndex ) )
					e = &getScriptEnvironment( e->parentIndex );
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
				block const & b = getBlock( e->blockIndex );
				e->codeIndex = b.vecCodes.size();
				e = &getScriptEnvironment( e->parentIndex );
				if( b.kind == block::bk_loop )
				{
					BaseRoutine = true;
					for( unsigned u = 0; u < e->stack.size(); ++u )
						scriptdata_mgr.releaseScriptData( e->stack[ u ] );
					e->stack.resize( 0 );
					do
						++(e->codeIndex);
					while( getBlock( e->blockIndex ).vecCodes[ e->codeIndex - 1 ].command != vc_loopBack );
					assert( env.stack.size() == 0 );
					assert( e->stack.size() == 0 );
				}
			}while( !BaseRoutine );
		}
		break;
	case vc_loopIf:
		{
			if( scriptdata_mgr.getRealScriptData( env.stack.back() ) == 0.f )
			{
				do
					++env.codeIndex;
				while( getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
			}
			scriptdata_mgr.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			assert( env.stack.size() == 0 );
		}
		break;
	case vc_loopBack:
		env.codeIndex = current_code.loopBackIndex;
		break;
	case vc_yield:
		current_context->current_thread_index = ( current_context->current_thread_index ? current_context->current_thread_index : current_context->threads.size() ) - 1;
		break;
	case vc_checkIf:
		{
			float real = scriptdata_mgr.getRealScriptData( env.stack.back() );
			scriptdata_mgr.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			if( real == 0.f )
			{
				do
					++env.codeIndex;
				while( getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_caseNext );
				assert( env.stack.size() == 0 );
			}
		}
		break;
	case vc_loopAscent:
	case vc_loopDescent:
		{
			float real = scriptdata_mgr.getRealScriptData( env.stack.back() );
			scriptdata_mgr.releaseScriptData( env.stack.back() );
			env.stack.pop_back();
			bool proceed = ((current_code.command == vc_loopAscent) ? real > scriptdata_mgr.getRealScriptData( env.stack.back() )
				: real < scriptdata_mgr.getRealScriptData( env.stack.back() ) );
			if( !proceed )
			{
				scriptdata_mgr.releaseScriptData( env.stack.back() );
				env.stack.pop_back();
				do
					++env.codeIndex;
				while( getBlock( env.blockIndex ).vecCodes[ env.codeIndex - 1 ].command != vc_loopBack );
				assert( env.stack.size() == 0 );
			}
		}
		break;
	case vc_caseBegin:
	case vc_caseNext:
	case vc_caseEnd:
		break;
	case vc_gotoEnd:
			for( block const & b = getBlock( env.blockIndex );
				b.vecCodes[ env.codeIndex - 1 ].command != vc_caseEnd;
				++env.codeIndex );
		break;
	case vc_invalid:
	default:
		assert( false );
	}
	return false;
}
void script_engine::clean_script_context( size_t context_index )
{
	currentRunningMachine = context_index;
	script_context * current_context = this->getScriptContext( context_index );
	if( !current_context->threads.size() )
		return;
	
	current_context->current_thread_index = current_context->threads.size() - 1;
	do
		getScriptEnvironment( current_context->threads[ current_context->current_thread_index ] ).codeIndex = -1;
	while( !advance() );
	releaseScriptEnvironment( current_context->threads[ current_context->current_thread_index ] );
	current_context->threads.pop_back();
	releaseObjectVector( current_context->object_vector_index );
	current_context->current_script_index = -1;
	current_context->current_thread_index = -1;
	current_context->object_vector_index = -1;
	current_context->script_object = -1;
	assert( !current_context->threads.size() );
}