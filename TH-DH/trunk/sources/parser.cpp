#include <parser.hpp>
#include <scriptengine.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>

parser::lexer::lexer() : current( &character ), next( tk_end ), character( '\0' )
{
}
parser::lexer::lexer( char const * strstart ) : current( strstart ), line( 1 )
{
}
void parser::lexer::skip()
{

	while( *current == 10 || *current == 13 || *current == 9 || *current == 32 || 
			( current[0] == '/' && (current[1] == '/' || current[1] == '*') ) )
	{
		if( current[1] == '/' )
			while( *current++ != '\n' );
		else if( current[1] == '*' )
		{
			current += 2;
			while( !( current[0] == '*' && current[1] == '/' ) )
			{
				if( *current == '\n' )
					++line;
				++current;
			}
			current +=2;
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
			else if( isalpha( *current ) && *current == '_' )
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
parser::token parser::lexer::getToken() const
{
	return next;
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
	std::string::const_iterator it = errmsg.begin();
	for( unsigned i = 1; i < lexicon.getLine() && it != errmsg.end(); ++it )
	{
		if( *it == '\n' )
			++i;
	}

	for( unsigned i = 0; i < 5 && it != errmsg.end(); ++it )
		err.fivelines += *it;

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
parser::parser( script_engine & eng ) : engine( eng )
{
	try
	{
		char buff[512] = { 0 };
		GetCurrentDirectory( sizeof( buff ), buff );
		std::string const path = std::string( buff ) + "\\script";
		mapScriptPaths( path );
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
