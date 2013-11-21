#pragma once
#include <bytecode.hpp>
#include <map>
#include <Direct3DEngine.hpp>
#include <scriptmachine.hpp>

class script_type_manager
{
private:
	vector< type_data > types;
public:
	script_type_manager();
	type_data getRealType() const;
	type_data getBooleanType() const;
	type_data getCharacterType() const;
	type_data getStringType() const;
	type_data getObjectType() const;
	type_data getMiscType() const;
	type_data getArrayType() const;
	type_data getArrayType( size_t element ); //an array of some type
};

class inventory : protected script_type_manager, protected virtual Battery
{
	friend class script_engine;
private:
	vector< script_data > vecScriptData;
	vector< script_environment > vecScriptEnvironment;
	vector< block > vecBlocks;
	vector< script_machine > vecMachines;
	vector< size_t > vecScriptDataGarbage;
	vector< size_t > vecRoutinesGabage;
	vector< size_t > vecMachinesGarbage;
	vector< script_container > vecScripts;
	vector< std::string > vecScriptDirectories;
	std::map< std::string, size_t > mappedScripts;
	std::map< std::string, size_t > mappedMainScripts;
	std::map< std::string, size_t > mappedShotScripts;
	vector< script_queue > vecQueuedScripts;

protected:
	size_t fetchBlock();
	block & getBlock( size_t index );
	size_t fetchScriptData();
	size_t fetchScriptData( float real );
	size_t fetchScriptData( char character );
	size_t fetchScriptData( bool boolean );
	size_t fetchScriptData( std::string const & string );
	size_t fetchScriptData( size_t objParam );
	size_t fetchScriptData( D3DPRIMITIVETYPE primType );
	size_t fetchScriptData( BlendType blend );
	script_data & getScriptData( size_t index );
	void addRefScriptData( size_t index );
	void scriptDataAssign( size_t & dst, size_t src );
	void copyScriptData( size_t & dst, size_t & src );
	void uniqueizeScriptData( size_t & dst );
	float getRealScriptData( size_t index ) const;
	char getCharacterScriptData( size_t index ) const;
	bool getBooleanScriptData( size_t index ) const;
	unsigned getObjHandleScriptData( size_t index ) const;
	D3DPRIMITIVETYPE getPrimitiveTypeScriptData( size_t index ) const;
	BlendType getBlendModeScriptData( size_t index ) const;
	std::string getStringScriptData( size_t index );
	void releaseScriptData( size_t & index );
	size_t fetchScriptEnvironment( size_t blockIndex );
	script_environment & getScriptEnvironment( size_t index );
	void addRefScriptEnvironment( size_t index );
	void releaseScriptEnvironment( size_t & index );
	size_t fetchScriptMachine();
	script_machine & getScriptMachine( size_t index );
	void setQueueScriptMachine( script_queue const queue );
	void releaseScriptMachine( size_t & index );
	size_t getBlockFromScript( std::string const & filePath, std::string const & scriptName );
	void registerScript( std::string const scriptName );
	void registerMainScript( std::string const scriptPath, std::string const scriptName );
	void registerInvalidMainScript( std::string const scriptPath );
	script_container * getScript( std::string const & scriptName );
	script_container & getScript( size_t index );
	size_t findScript( std::string const & scriptName );
	size_t findScriptFromFile( std::string const & scriptPath );
	size_t findScriptDirectory( std::string const & scriptPath );
	std::string const & getCurrentScriptDirectory( size_t machineIdx ) const;
};
