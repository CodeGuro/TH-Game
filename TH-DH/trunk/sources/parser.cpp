#include <parser.hpp>
#include <scriptengine.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <math.h>

parser::lexer::lexer() : current( &character ), next( tk_end ), character( '\0' )
{
}
parser::lexer::lexer( char const * strstart ) : current( strstart ), line( 1 )
{
}
parser::lexer::lexer( char const * strstart, unsigned lineStart ) : current( strstart ), line( lineStart )
{
}
void parser::lexer::skip()
{

	while( *current == 10 || *current == 13 || *current == 9 || *current == 32 || 
			( current[0] == '/' && (current[1] == '/' || current[1] == '*') ) )
	{
		if( current[0] == '/' && current[1] == '/' )
		{
			do
			{
				++current;
			}while( !( current[0] == '\n' || current[0] == '\0' ) );
		}
		else if( current[0] == '/' && current[1] == '*' )
		{
			do
			{
				++current;
				if( *current == '\n' )
					++line;
				else if( *current == '\0' )
					break;
			}while( !( *(current-2) == '*' && *(current-1) == '/') );
		}
		else
		{
			if( *current == '\n' )
				++line;
			++current;
		}
	}
}
parser::token parser::lexer::advance()
{
	skip();
	switch( *current )
	{
	case '\0':
		next = tk_end;
		break;
	case '[':
		++current;
		next = tk_openbra;
		break;
	case ']':
		++current;
		next = tk_closebra;
	case '(':
		++current;
		next = tk_lparen;
		break;
	case ')':
		++current;
		next = tk_rparen;
		break;
	case '{':
		++current;
		next = tk_opencur;
		break;
	case '}':
		++current;
		next = tk_closecur;
	case '@':
		++current;
		next = tk_at;
		break;
	case ';':
		++current;
		next = tk_semicolon;
		break;
	case '~':
		++current;
		next = tk_tilde;
		break;
	case ',':
		++current;
		next = tk_comma;
		break;
	case '.':
		++current;
		next = tk_dot;
		break;
	case '#':
		++current;
		next = tk_sharp;
		break;
	case '*':
		{
			++current;
			next = tk_asterisk;
			if( *current == '=' )
			{
				++current;
				next = tk_multiply_assign;
			}
		}
		break;
	case '/':
		{
			++current;
			next = tk_slash;
			if( *current == '=' )
			{
				++current;
				next = tk_divide_assign;
			}
		}
		break;
	case '%':
		{
			++current;
			next = tk_percent;
			if( *current == '=' )
			{
				++current;
				next = tk_remainder_assign;
			}
		}
		break;
	case '^':
		{
			++current;
			next = tk_caret;
			if( *current == '=' )
			{
				++current;
				next = tk_power_assign;
			}
		}
		break;
	case '=':
		{
			++current;
			next = tk_assign;
			if( *current == '=' )
			{
				++current;
				next = tk_compare_equal;
			}
		}
		break;
	case '>':
		{
			++current;
			next = tk_compare_greater;
			if( *current == '=' )
			{
				++current;
				next = tk_compare_greaterequal;
			}
		}
		break;
	case '<':
		{
			++current;
			next = tk_compare_less;
			if( *current == '=' )
			{
				++current;
				next = tk_compare_lessequal;
			}
		}
		break;
	case '!':
		{
			++current;
			next = tk_not;
			if( *current == '=' )
			{
				++current;
				next = tk_compare_notequal;
			}
		}
		break;
	case '&':
		{
			++current;
			next = tk_ampersand;
			if( *current == '&' )
			{
				++current;
				next = tk_and;
			}
		}
		break;
	case '|':
		{
			++current;
			next = tk_vertical;
			if( *current == '|' )
			{
				++current;
				next = tk_or;
			}
		}
		break;
	case '+':
		{
			++current;
			next = tk_plus;
			if( *current == '=' )
			{
				++current;
				next = tk_add_assign;
			}
			else if( *current == '+' )
			{
				++current;
				next = tk_increment;
			}
		}
		break;
	case '-':
		{
			++current;
			next = tk_minus;
			if( *current == '=' )
			{
				++current;
				next = tk_subtract_assign;
			}
			else if( *current == '-' )
			{
				++current;
				next = tk_decrement;
			}
		}
		break;
	case '\'':
	case '\"':
		{
			std::string str;
			char q = *current;
			token tok = (( q == '\'' )? tk_character : tk_string );
			++current;
			while( *current != q )
			{
				if( *current == '\0' )
					return tk_end;
				str += *current;
				++current;
			}
			++current;
			word = str;
			if( str.size() > 1 && tok == tk_character )
				next = tk_invalid;
			else
				next = tok;
		}
		break;


	default:
		{
			if( isdigit( *current ) )
			{
				++current;
				next = tk_real;
				real = 0.0f;
				do
				{
					real = real * 10 + ( *current - '0' );
					++current;
				}while( isdigit( *current ) );
				if( current[0] == '.' && isdigit( current[1] ) )
				{
					++current;
					float tmp = 1.0f;
					while( isdigit( *current ) )
					{
						tmp = tmp / 10;
						real = real + ( tmp * ( *current - '0' ) );
						++current;
					}
				}

			}
			else if( isalpha( *current ) || *current == '_' )
			{
				next = tk_word;
				word = "";
				do
				{
					word += *current;
					current++;
				}while( isalpha( *current ) || *current == '_' || isdigit( *current ) );
				if( word == "if" )
					next = tk_IF;
				else if( word == "else" )
					next = tk_ELSE;
				else if( word == "while" )
					next = tk_WHILE;
				else if( word == "loop" )
					next = tk_LOOP;
				else if( word == "break" )
					next = tk_BREAK;
				else if( word == "let" )
					next = tk_LET;
				else if( word == "function" )
					next = tk_FUNCTION;
				else if( word == "task" )
					next = tk_TASK;
				else if( word == "return" )
					next = tk_RETURN;
				else if( word == "script_stage_main" )
					next = tk_SCRIPT_STAGE_MAIN;
				else if( word == "script_enemy" )
					next = tk_SCRIPT_ENEMY;
			}
			else
				next = tk_invalid;
		}
		break;
	}

	return next;
}
unsigned parser::lexer::getLine() const
{
	return line;
}
char parser::lexer::getCharacter() const
{
	return character;
}
float parser::lexer::getReal() const
{
	return real;
}
std::string parser::lexer::getString() const
{
	return word;
}
std::string parser::lexer::getWord() const
{
	return word;
}
parser::token parser::lexer::getToken() const
{
	return next;
}
const char * parser::lexer::getCurrent() const
{
	return current;
}

block & parser::getBlock()
{
	return engine.getBlock( vecScope[ vecScope.size() - 1 ].blockIndex );
}
void parser::pushCode( code const & val )
{
	getBlock().vecCodes.push_back( val );
}
void parser::registerScript( std::string const & fullPath )
{
	if( scriptMgr.scriptUnits.find( fullPath ) != scriptMgr.scriptUnits.end() )
		raiseError( std::string() + "\"" + fullPath + "\" is already registered", error::er_parser );
	scriptMgr.scriptUnits[ fullPath ].finishParsed = false;
}
void parser::raiseError( std::string errmsg, error::errReason reason)
{
	error err;
	err.reason = reason;
	err.errmsg = errmsg;
	err.line = lexicon.getLine();
	err.pathDoc = scriptMgr.currentScriptPath;
	std::string::iterator it = scriptMgr.scriptString.begin();
	for( unsigned i = 1; i < lexicon.getLine() && it != scriptMgr.scriptString.end(); ++it )
	{
		if( *it == '\n' )
			++i;
	}

	for( unsigned i = 0; i < 5 && it != scriptMgr.scriptString.end(); ++it )
	{
		if( *it == '\n' )
			++i;
		err.fivelines += *it;
	}

	throw err;
}
void parser::mapScriptPaths( std::string const & pathStart )
{
	WIN32_FIND_DATA findDat;
	HANDLE hFile = FindFirstFile( ( pathStart + "\\*" ).c_str(), &findDat );
	do
	{
		std::string const fullPath = pathStart + "\\" + findDat.cFileName;
		if( findDat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( !( std::string( findDat.cFileName ) == "." || std::string( findDat.cFileName ) == ".." ) )
			{
				mapScriptPaths( fullPath );
			}
		}
		else
		{
			HANDLE _hFile = CreateFile( fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			char buff[512] = "#TouhouDanmaku";
			std::string scriptHeader = buff;
			DWORD readBytes;
			ReadFile( _hFile, buff, sizeof( buff ), &readBytes, NULL );
			CloseHandle( _hFile );
			if( std::string( buff ).find( scriptHeader ) != std::string::npos )
				registerScript( fullPath );
		}
	}while( FindNextFile( hFile, &findDat ) );
	FindClose( hFile );
}
parser::symbol * parser::search( std::string const & name )
{
	for( unsigned i = vecScope.size(); i--; )
	{
		if( vecScope[i].find( name ) != vecScope[i].end() )
			return &(vecScope[i][name]);
	}
	return NULL;
}
parser::parser( script_engine & eng ) : engine( eng )
{
	try
	{
		//map the scripts to individual units to be parsed
		char buff[512] = { 0 };
		GetCurrentDirectory( sizeof( buff ), buff );
		std::string const path = std::string( buff ) + "\\script";
		mapScriptPaths( path );

		for( std::map< std::string, scriptHandler::scriptObj >::iterator it = scriptMgr.scriptUnits.begin(); it != scriptMgr.scriptUnits.end(); ++it )
		{
			if( !(it->second.finishParsed) )
			{
				parseScript( it->first );
				it = scriptMgr.scriptUnits.begin();
			}
		}
	}
	catch( error const & err )
	{
		switch( err.reason )
		{
		case error::er_internal:
			std::cout << "INTERNAL PARSER ERROR";
			break;
		case error::er_parser:
			std::cout << "Parser error";
			break;
		case error::er_syntax:
			std::cout << "Syntax error" << std::endl << err.pathDoc << std::endl
				<< "line " << err.line << std::endl << err.errmsg << std::endl << err.fivelines << std::endl;
			break;
		}
	}
}
void parser::parseScript( std::string const & scriptPath )
{
	if( vecScope.size() )
		raiseError( "The scope has not been cleared for new parse", error::er_parser );
	vecScope.push_back( scope() );
	vecScope[0].blockIndex = invalidIndex;
	scriptMgr.currentScriptPath = scriptPath;
	scriptMgr.scriptString = std::string( (std::istreambuf_iterator< char >( std::ifstream( scriptPath ) )), std::istreambuf_iterator< char >() );
	lexicon = lexer( scriptMgr.scriptString.c_str() );
	importNativeSymbols();
	scanCurrentScope( block::bk_normal, vector< std::string >() );
	parseBlock( block::bk_normal, vector< std::string >() );
	vecScope.pop_back();
	scriptMgr.scriptUnits[ scriptPath ].finishParsed = true;
}
/*incomplete*/void parser::parseBlock( block::block_kind kind, vector< std::string > const args )
{
}
/*incomplete*/void parser::parsePreProcess( void )
{
	while( lexicon.advance() == tk_sharp )
	{
		if( lexicon.advance() == tk_word )
		{
			if( lexicon.getWord() == "TouhouDanmaku" )
			{
			}
			else if( lexicon.getWord() == "include" )
			{
				if( lexicon.advance() != tk_string )
					raiseError( "\"[string]\" expected", error::er_syntax );
				lexer tmp = lexicon;
				std::string str = lexicon.getString();
				std::string path;
				std::string::iterator it = str.begin();
				if( *it == '.' )
				{
					std::string::iterator it2 = scriptMgr.currentScriptPath.end() - 1;
					do
					{
						while( *(--it2) != '\\' )
							--it2;
					}while( *(++it) == '.' );
					path = std::string( scriptMgr.currentScriptPath.begin(), it2 ) + std::string( it, str.end() );
				}
				else if( str.size() > 7 && std::string( it, it + 7 ) == "script\\" )
				{
					char * buff = new char[ 512 ]();
					GetCurrentDirectory( 512, buff );
					path = std::string( buff ) + "\\" + str;
					delete buff;
				}
				else
					path = str;
				std::string pathStr = std::string( std::istreambuf_iterator< char >( std::ifstream( path ) ), std::istreambuf_iterator< char >() );
				if( !pathStr.size() )
					raiseError( "File does not exist or is an invalid document", error::er_syntax );

				//save the current workspace
				lexer lexSave = lexicon;
				unsigned lexCurrSave = lexicon.getCurrent() - scriptMgr.currentScriptPath.c_str();
				std::string currentPath = scriptMgr.currentScriptPath;
				std::string currentScriptStr = scriptMgr.scriptString;

			}
		}

	}
}
/*incomplete*/void parser::scanCurrentScope( block::block_kind kind, vector< std::string > const args )
{
}

struct native_function
{
	char const * name;
	void (*nCallBack)( script_engine * eng, size_t * argv );
	unsigned argc;
};

//native symbols defined here
struct natives
{
private:
	friend class parser;
	static void _add( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real + eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _subtract( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real - eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _multiply( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real * eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _divide( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real / eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _power( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( pow( eng->getScriptData( argv[0] ).real, eng->getScriptData( argv[1] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _absolute( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( abs( eng->getScriptData( argv[0] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _roof( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( ceil( eng->getScriptData( argv[0] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _floor( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( floor( eng->getScriptData( argv[0] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _index( script_engine * eng, size_t * argv )
	{
		eng->scriptDataAssign( argv[0], eng->getScriptData( argv[0] ).vec[ (unsigned)eng->getScriptData( argv[1] ).real ] );
	}
	static void _uniqueize( script_engine * eng, size_t * argv )
	{
		eng->uniqueizeScriptData( argv[0] );
	}
	static void _rand( script_engine * eng, size_t * argv )
	{
		float domain = eng->getScriptData( argv[1] ).real - eng->getScriptData( argv[0] ).real;
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real + fmod( (float)rand() + 1.f, domain ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _rand_int( script_engine * eng, size_t * argv )
	{
		float domain = floor( eng->getScriptData( argv[1] ).real - eng->getScriptData( argv[0] ).real );
		size_t tmp = eng->fetchScriptData( floor( eng->getScriptData( argv[0] ).real ) + fmod( (float)rand() + 1.f, domain ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
};

void parser::importNativeSymbols()
{
	if( vecScope.size() != 1 )
		raiseError( "Natives can be imported only in the file scope", error::er_parser );
	native_function funcs[] =
	{
		{ "add", &natives::_add, 2 },
		{ "subtract", &natives::_subtract, 2 },
		{ "multiply", &natives::_multiply, 2 },
		{ "divide", &natives::_divide, 2 },
		{ "power", &natives::_power, 2 },
		{ "absolute", &natives::_absolute, 1 },
		{ "roof", &natives::_roof, 1 },
		{ "floor", &natives::_floor, 1 },
		{ "index", &natives::_index, 1 },
		{ "uniqueize", &natives::_uniqueize, 1 },
		{ "rand", &natives::_rand, 2 },
		{ "rand_int", &natives::_rand_int, 2 }
	};
	for( unsigned i = 0; i <  sizeof( funcs ) / sizeof( native_function ); ++i )
	{
		symbol * s = search( funcs[i].name );
		if( s )
			raiseError( std::string() + "\"" + funcs[i].name +"\" "+ "has already been defined" , error::er_syntax );
		block & b = engine.getBlock( engine.fetchBlock() );
		b.hasResult = true;
		b.kind = block::bk_function;
		b.name = funcs[i].name;
		b.argc = funcs[i].argc;
		b.nativeCallBack = funcs[i].nCallBack;
		symbol sym;
		sym.blockIndex = invalidIndex;
		sym.id = invalidIndex;
		sym.level = 0;
		vecScope[0][ funcs[i].name ] = sym;
	}
}