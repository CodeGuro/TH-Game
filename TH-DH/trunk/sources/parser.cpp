#include <parser.hpp>
#include <scriptengine.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <math.h>
#include <sstream>
#define CSTRFUNCRESULT "(RESULT)"

parser::lexer::lexer() : current( &character ), next( tk_end ), character( '\0' )
{
}
parser::lexer::lexer( char const * strstart ) : current( strstart ), line( 1 )
{
	advance();
}
parser::lexer::lexer( char const * strstart, unsigned lineStart ) : current( strstart ), line( lineStart )
{
	advance();
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
		break;
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
		break;
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
				else if( word == "script_main" )
					next = tk_SCRIPT_MAIN;
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

void parser::parseExpression()
{
	parseLogic();
}
void parser::parseLogic()
{
	parseComparison();
	while( lexicon.getToken() == tk_and || lexicon.getToken() == tk_or )
	{
		token tok = lexicon.getToken();
		lexicon.advance();
		parseComparison();
		std::string operation = ( tok == tk_and ? "logicAnd" : "logicOr" );
		writeOperation( operation );
	}
}
void parser::parseComparison()
{
	parseSum();
	std::string operation;
	switch( lexicon.getToken() )
	{
	case tk_assign:
		raiseError( "\"=\" is not a comparison operator, did you mean \"==\"?", error::er_syntax );
		break;
	case tk_compare_equal:
		operation = "compareEqual";
		break;
	case tk_compare_notequal:
		operation = "compareNotEqual";
		break;
	case tk_compare_greater:
		operation = "compareGreater";
		break;
	case tk_compare_greaterequal:
		operation = "compareGreaterEqual";
		break;
	case tk_compare_less:
		operation = "compareLess";
		break;
	case tk_compare_lessequal:
		operation = "compareLessEqual";
		break;
	}
	if( operation.size() )
	{
		lexicon.advance();
		writeOperation( operation );
	}
}
void parser::parseSum()
{
	parseProduct();
	while( lexicon.getToken() == tk_plus || lexicon.getToken() == tk_minus || lexicon.getToken() == tk_tilde )
	{
		token tok = lexicon.getToken();
		lexicon.advance();
		parseProduct();
		std::string operation = ( tok == tk_plus ? "add" : ( tok == tk_minus ? "subtract" : "concatenate" ) );
		writeOperation( operation );
	}
}
void parser::parseProduct()
{
	parsePrefix();
	while( lexicon.getToken() == tk_asterisk || lexicon.getToken() == tk_slash )
	{
		token tok = lexicon.getToken();
		lexicon.advance();
		parsePrefix();
		std::string operation = ( tok == tk_asterisk ? "multiply" : "divide" );
		writeOperation( operation );
	}
}
void parser::parsePrefix()
{
	std::string operation;
	switch( lexicon.getToken() )
	{
	case tk_plus:
		lexicon.advance();
		parsePrefix();
		break;
	case tk_minus:
		lexicon.advance();
		operation = "negative";
		parsePrefix();
		break;
	case tk_not:
		lexicon.advance();
		operation = "not";
		parsePrefix();
		break;
	}
	operation.size() ? writeOperation( operation ) : parseSuffix();
}
void parser::parseSuffix()
{
	parseClause();
	if( lexicon.getToken() == tk_caret )
	{
		lexicon.advance();
		parseSuffix();
		writeOperation( std::string( "power" ) );
	}
}
void parser::parseClause()
{
	switch( lexicon.getToken() )
	{
	case tk_real:
		pushCode( code::dat( vc_pushVal, engine.fetchScriptData( lexicon.getReal() ) ) );
		lexicon.advance();
		break;
	case tk_character:
		pushCode( code::dat( vc_pushVal, engine.fetchScriptData( lexicon.getCharacter() ) ) );
		lexicon.advance();
		break;
	case tk_string:
		pushCode( code::dat( vc_pushVal, engine.fetchScriptData( lexicon.getString() ) ) );
		writeOperation( "uniqueize" );
		lexicon.advance();
		break;
	case tk_word:
		{
			symbol * sym = search( lexicon.getWord() );
			if( !sym )
				raiseError( lexicon.getWord(), error::er_symbol );
			lexicon.advance();
			if( sym->blockIndex != invalidIndex )
			{
				int argc;
				if( (argc = parseArguments()) != engine.getBlock( sym->blockIndex ).argc )
					raiseError( "wrong number of arguments", error::er_syntax );
				pushCode( code::subArg( vc_callFunctionPush, sym->blockIndex, argc ) );
			}
			else
				pushCode( code::varLev( vc_pushVar, sym->id, sym->level - vecScope.size() ) );

		}
		break;
	case tk_lparen:
		{
			parseParentheses();
		}
		break;
	case tk_openbra:
		{
			size_t scriptDataIdx = engine.fetchScriptData();
			engine.getScriptData( scriptDataIdx ).type = engine.typeManager.getArrayType();
			pushCode( code::dat( vc_pushVal, scriptDataIdx ) );
			writeOperation( "uniqueize" );
			do
			{
				lexicon.advance();
				parseExpression();
				writeOperation( "appendArray" );
			}while( lexicon.getToken() == tk_comma );
			if( lexicon.getToken() != tk_closebra )
				raiseError( "\"]\" expected", error::er_syntax );
			writeOperation( "uniqueize" );
			lexicon.advance();
		}
		break;
	default:
		raiseError( "invalid clause in expression", error::er_syntax );
	}
}
void parser::parseParentheses()
{
	if( lexicon.getToken() != tk_lparen )
		raiseError( "\"(\" expected" , error::er_syntax );
	lexicon.advance();
	parseExpression();
	if( lexicon.getToken() != tk_rparen )
		raiseError( "\")\" expected", error::er_syntax );
	lexicon.advance();
}
unsigned parser::parseArguments()
{
	unsigned argc = 0;
	if( lexicon.getToken() == tk_lparen )
	{
		do
		{
			if( lexicon.advance() == tk_rparen )
			{
				lexicon.advance();
				break;
			}
			parseExpression();
			++argc;
		}while( !(lexicon.getToken() == tk_comma || lexicon.getToken() == tk_rparen) );
	}
	return argc;
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
parser::symbol * parser::searchResult()
{
	std::string str = CSTRFUNCRESULT;
	return search( str );
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
		std::stringstream sstr;
		std::stringstream sstrAdditional;
		std::string title;
		switch( err.reason )
		{
		case error::er_internal:
			title = "INTERNAL PARSER ERROR";
			break;
		case error::er_parser:
		case error::er_syntax:
			title = ( err.reason == error::er_parser? "Parser Error" : "Syntax Error" );
			sstrAdditional << err.errmsg;
			break;
		case error::er_symbol:
			title = "Syntax / Linkage Error";
			sstrAdditional << "\"" << err.errmsg << "\"" << " has already been defined";
			break;
		case error::er_usymbol:
			title = "Syntax Error";
			sstrAdditional << "\"" << err.errmsg << "\"" << " undefined symbol";
		}
		sstr << err.pathDoc << "\n\n" << "line " << err.line << "\n\n"
			<< sstrAdditional.str() << "\n\n" << err.fivelines << "\n" << "....." << std::endl;
		MessageBoxA( NULL, sstr.str().c_str(), title.c_str(), NULL );
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
	parseStatements();
	if( lexicon.getToken() != tk_end )
		raiseError( "Parser did not completely parse the script", error::er_parser );
	vecScope.pop_back();
	scriptMgr.scriptUnits[ scriptPath ].finishParsed = true;
}
void parser::parseBlock( symbol const symSub, vector< std::string > const & args )
{
	if( lexicon.getToken() != tk_opencur )
		raiseError( "\"{\" expected", error::er_syntax );
	lexicon.advance();
	vecScope.push_back( scope() );
	vecScope[ vecScope.size() - 1 ].blockIndex = symSub.blockIndex;
	scanCurrentScope( engine.getBlock( symSub.blockIndex ).kind, args );
	for( unsigned i = 0; i < args.size(); ++i )
		engine.getBlock( symSub.blockIndex ).vecCodes.push_back( code::varLev( vc_assign, search( args[i] )->id, 0 ) );
	parseStatements();
	vecScope.pop_back();

	if( lexicon.getToken() != tk_closecur )
		raiseError( "\"}\" expected", error::er_syntax );
	lexicon.advance();
}
void parser::parseInlineBlock( block::block_kind const bk_kind )
{
	size_t blockIndex = engine.fetchBlock();
	block & inlineBlock = engine.getBlock( blockIndex );
	inlineBlock.argc = 0;
	inlineBlock.hasResult = false;
	inlineBlock.kind = bk_kind;
	inlineBlock.nativeCallBack = 0;
	inlineBlock.name = "inlinedBlock";
	symbol virtualSymbol;
	virtualSymbol.blockIndex = vecScope[ vecScope.size() - 1 ].blockIndex;
	virtualSymbol.level = vecScope.size() + 1;
	virtualSymbol.id = (size_t)-1;
	parseBlock( virtualSymbol, vector< std::string >() );
	getBlock().vecCodes.push_back( code::subArg( vc_callFunction, blockIndex, 0 ) );
}
void parser::scanCurrentScope( block::block_kind kind, vector< std::string > const & args )
{
	//if it's a function, allow a result
	unsigned id = 0;
	unsigned level = vecScope.size() - 1;
	scope & currentScope = vecScope[ vecScope.size() -1 ];
	if( kind == block::bk_function )
	{
		symbol result;
		result.blockIndex = -1;
		result.id = id++;
		result.level = level;
		vecScope[ vecScope.size() - 1 ][ CSTRFUNCRESULT ] = result;
	}
	for( unsigned i = 0; i < args.size(); ++i )
	{
		symbol arg;
		arg.blockIndex = -1;
		arg.id = id++;
		arg.level = level;
		vecScope[ level ][ args[i] ] = arg;
	}

	lexer anchorpoint = lexicon;
	unsigned nested = level;
	token tok;
	do
	{
		tok = lexicon.getToken();
		lexicon.advance();

		if( tok == tk_opencur )
			++nested;
		else if( tok == tk_closecur )
			--nested;
		else if( nested == level )
		{
			if( tok == tk_LET )
			{
				if( lexicon.getToken() == tk_word )
				{
					std::string word = lexicon.getWord();
					if( currentScope.find( word ) != currentScope.end() )
						raiseError( word, error::er_symbol );
					symbol variable;
					variable.blockIndex = -1;
					variable.level = level;
					variable.id = id++;
					currentScope[ word ] = variable;
				}
			}
			else if( tok == tk_at || tok == tk_FUNCTION || tok == tk_TASK || tok == tk_SCRIPT_MAIN || tok == tk_SCRIPT_ENEMY )
			{
				if( lexicon.getToken() == tk_word )
				{
					std::string subroutine = lexicon.getWord();
					if( currentScope.find( subroutine ) != currentScope.end() )
						raiseError( subroutine, error::er_symbol );
					symbol routine;
					routine.blockIndex = engine.fetchBlock();
					routine.id = -1;
					routine.level = level + 1;
					currentScope[ subroutine ] = routine;
					block & blockRoutine = engine.getBlock( routine.blockIndex );
					blockRoutine.kind = (tok == tk_FUNCTION? block::bk_function : (tok == tk_TASK? block::bk_task : (tok == tk_at? block::bk_sub : block::bk_normal )));
					blockRoutine.hasResult = (tok == tk_FUNCTION);
					blockRoutine.name = subroutine;
					blockRoutine.nativeCallBack = 0;
					blockRoutine.argc = 0;
					if( tok == tk_SCRIPT_MAIN || tok == tk_SCRIPT_ENEMY )
					{
						engine.registerScript( subroutine, routine.blockIndex );
					}
					else if( lexicon.advance() == tk_lparen )
					{
						do
						{
							if( lexicon.advance() == tk_LET )
								if( lexicon.advance() == tk_word )
									++blockRoutine.argc;
						}while( lexicon.advance() == tk_comma );
						if( lexicon.getToken() != tk_rparen )
							raiseError( "\")\" expected", error::er_syntax );
					}
				}
			}
		}
	}while( tok != tk_end && nested >= level );

	lexicon = anchorpoint;
}
/*incomplete*/ void parser::parseStatements()
{
	bool finished = false;
	do
	{
		bool needSemicolon = true;
		if( lexicon.getToken() == tk_sharp )
		{
			if( lexicon.advance() == tk_word )
			{
				std::string preproc = lexicon.getWord();
				if( preproc == "TouhouDanmaku" )
				{
					if( lexicon.advance() != tk_openbra )
						raiseError( "\"[\" expected", error::er_syntax );
					if( lexicon.advance() != tk_word )
						raiseError( "script specification necessary", error::er_syntax );
					std::string scriptspec = lexicon.getWord();
					if( lexicon.advance() != tk_closebra )
						raiseError( "\"]\" expected", error::er_syntax );
					lexicon.advance();

				}
				else if( preproc == "include" )
				{
					if( lexicon.advance() != tk_string )
						raiseError( "script path [string] expected", error::er_syntax );
					std::string fullPath;
					std::string path = lexicon.getString();
					std::string::iterator it = path.begin();
					if( *it == '.' )
					{
						std::string::iterator it2 = scriptMgr.currentScriptPath.end() - 1;
						do
						{
							while( *(--it2) != '\\' )
								--it2;
						}while( *(++it) == '.' );
						fullPath = std::string( scriptMgr.currentScriptPath.begin(), it2 ) + std::string( it, path.end() );
					}
					else if( path.size() > 7 && std::string( it, it + 7 ) == "script\\" )
					{
						char * buff = new char[ 512 ]();
						GetCurrentDirectory( 512, buff );
						fullPath = std::string( buff ) + "\\" + path;
						delete buff;
					}
					else
						fullPath = path;
					std::string scriptstr = std::string( std::istreambuf_iterator< char >( std::ifstream( fullPath ) ), std::istreambuf_iterator< char >() );
					if( !scriptstr.size() )
						raiseError( "File does not exist or is an invalid document", error::er_syntax );
					//save
					unsigned lexLine = lexicon.getLine();
					unsigned lexPlace = lexicon.getCurrent() - scriptMgr.scriptString.c_str();
					scriptMgr.pragmaFiles.push_back( fullPath );
					std::string currentPath = scriptMgr.currentScriptPath;
					std::string currentScriptStr = scriptMgr.scriptString;

					//parse the new document
					scriptMgr.currentScriptPath = fullPath;
					scriptMgr.scriptString = scriptstr;
					lexicon = lexer( scriptMgr.scriptString.c_str() );
					scanCurrentScope( block::bk_normal, vector< std::string >() );
					parseStatements();

					//restore
					scriptMgr.currentScriptPath = currentPath;
					scriptMgr.scriptString = currentScriptStr;
					lexicon = lexer( scriptMgr.scriptString.c_str() + lexPlace, lexLine );
				}
			}
			needSemicolon = false;
		}

		else if( lexicon.getToken() == tk_FUNCTION || lexicon.getToken() == tk_at 
			|| lexicon.getToken() == tk_TASK || lexicon.getToken() == tk_SCRIPT_MAIN 
			|| lexicon.getToken() == tk_SCRIPT_ENEMY )
		{
			if ( lexicon.advance() != tk_word )
				raiseError( "the subroutine must be named", error::er_syntax );
			std::string subname = lexicon.getWord();
			symbol * subsym = search( subname );
			if( !subsym )
				raiseError( subname, error::er_usymbol );
			vector< std::string > args;
			if( lexicon.advance() == tk_lparen ) //function foo( let a, let b, ){}
			{
				do
				{
					if( lexicon.advance() == tk_LET )
					{
						if( lexicon.advance() != tk_word )
							raiseError( "improper syntax", error::er_syntax );
						args.push_back( lexicon.getWord() );
						lexicon.advance();
					}
				}while( lexicon.getToken() == tk_comma );
				if( lexicon.getToken() != tk_rparen )
					raiseError( "\")\" expected", error::er_syntax );
				lexicon.advance();
			}
			if( engine.getScript( subname ) != invalidIndex && args.size() )
				raiseError( "script_main and script_enemy routine types have zero parameters", error::er_syntax );
			if( subname == "Initialize" || subname == "MainLoop" || subname == "Finalize" || subname == "BackGround" )
			{
				if( engine.getBlock( subsym->blockIndex ).hasResult )
					raiseError( std::string() + "\"" + subname + "\" must be prefixed with \"@\"", error::er_syntax );
			}
			parseBlock( *subsym, args );
			needSemicolon = false;
		}
		
		else if( lexicon.getToken() == tk_RETURN )
		{
			if( lexicon.advance() != tk_semicolon )
			{
				parseExpression();
				symbol * res = searchResult();
				if( !res )
					raiseError( "\"return\" not nested within a functional scope", error::er_syntax );
				pushCode( code::varLev( vc_assign, res->id, vecScope.size() - res->level ) );
			}
			pushCode( code::code( vc_breakRoutine )  );
		}

		else if( lexicon.getToken() == tk_LET )
		{
			if( lexicon.advance() != tk_word )
				raiseError( "\"let\" improper declaration of a symbol", error::er_syntax );
			symbol * declsym = search( lexicon.getWord() );
			if( !declsym )
				raiseError( std::string() + "parser::parseStatements unreferenced symbol \"tk_LET\" with " + lexicon.getWord(), error::er_parser );
			if( lexicon.advance() == tk_assign )
			{
				lexicon.advance();
				parseExpression();
				pushCode( code::varLev( vc_assign, declsym->id, vecScope.size() - declsym->level ) );
			}
		}
		
		else if( lexicon.getToken() == tk_word )
		{
			symbol * sym = search( lexicon.getWord() );
			if( !sym )
				raiseError( lexicon.getWord(), error::er_usymbol );
			lexicon.advance();
			if( lexicon.getToken() == tk_assign && sym->id != invalidIndex )
			{
				lexicon.advance();
				parseExpression();
				pushCode( code::varLev( vc_assign, sym->id, vecScope.size() - sym->level ) );
			}
			else if( lexicon.getToken() == tk_openbra )
			{
				while( lexicon.getToken() == tk_openbra && sym->id != invalidIndex ) //word[32][32]='5';
				{
					lexicon.advance();
					parseExpression();
					if( lexicon.getToken() != tk_closebra )
						raiseError( "\"]\" expected", error::er_syntax );
					writeOperation( "index" );
					lexicon.advance();
				}
				if( lexicon.getToken() != tk_assign )
					raiseError( "\"=\" expected", error::er_syntax );
				lexicon.advance();
				parseExpression();
				pushCode( code::code( vc_overWrite ) );
			}
			else if( sym->blockIndex != invalidIndex )
			{
				unsigned argc = parseArguments();
				if( argc != engine.getBlock( sym->blockIndex ).argc )
					raiseError( "wrong number of arguments", error::er_syntax );
				instruction callInst = ( (engine.getBlock( sym->blockIndex ).kind == block::bk_task)? vc_callTask : vc_callFunction );
				pushCode( code::subArg( callInst, sym->blockIndex, argc ) );

			}
			else
			{
				raiseError( "parser::parseStatements() lextok == tk_word ", error::er_parser );
			}
		}
		
		else if( lexicon.getToken() == tk_LOOP )
		{
			unsigned loopBackIndex;
			if( lexicon.advance() == tk_lparen )
			{
				parseParentheses();
				writeOperation( "uniqueize" ); 
				loopBackIndex = getBlock().vecCodes.size(); 
				pushCode( code::code( vc_loopIfDecr ) );

			}
			else
			{
				loopBackIndex = getBlock().vecCodes.size();
			}
			parseInlineBlock( block::bk_loop );
			pushCode( code::loop( vc_loopBack, loopBackIndex ) );
			needSemicolon = false;
		}

		else if( lexicon.getToken() == tk_WHILE )
		{
			if( lexicon.advance() == tk_lparen )
			{
				parseParentheses();
				unsigned loopBackIndex = getBlock().vecCodes.size();
				pushCode( code::code( vc_loopIf ) );
				parseInlineBlock( block::bk_loop );
				pushCode( code::loop( vc_loopBack, loopBackIndex ) );
				needSemicolon = false;
			}
		}

		else if( lexicon.getToken() == tk_IF )
		{
			bool fin = false;
			pushCode( code::code( vc_caseBegin ) );
			do
			{
				token tok = lexicon.getToken();
				lexicon.advance();
				if( tok == tk_IF )
				{
					parseParentheses();
					pushCode( code::code( vc_checkIf ) );
					parseInlineBlock( block::bk_normal );
					if( lexicon.getToken() != tk_ELSE )
						fin = true;
				}
				else if( tok == tk_ELSE )
				{
					pushCode( code::code( vc_caseNext ) );
					if( lexicon.getToken() != tk_IF )
					{
						parseInlineBlock( block::bk_normal );
						fin = true;
					}
				}
			}while( !fin );
			pushCode( code::code( vc_caseEnd ) );
			needSemicolon = false;
		}

		else if( lexicon.getToken() == tk_YIELD )
		{
			lexicon.advance();
			pushCode( code::code( vc_yield ) );
		}

		if( needSemicolon && lexicon.getToken() != tk_semicolon )
			finished = true;
		if( lexicon.getToken() == tk_semicolon )
			lexicon.advance();
	}while( !finished );
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
	static void _negative( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( -(eng->getScriptData( argv[0] ).real) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _power( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( pow( eng->getScriptData( argv[0] ).real, eng->getScriptData( argv[1] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _concatenate( script_engine * eng, size_t * argv )
	{
		eng->uniqueizeScriptData( argv[0] );
		unsigned s = eng->getScriptData( argv[1] ).vec.size();
		for( unsigned i = 0; i < s; ++i )
		{
			size_t idx = invalidIndex;
			eng->scriptDataAssign( idx, eng->getScriptData( argv[1] ).vec[i] );
			eng->getScriptData( argv[0] ).vec.push_back( idx );
		}
	}
	static void _absolute( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( abs( eng->getScriptData( argv[0] ).real ) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _not( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( !(eng->getScriptData( argv[0] ).real) );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareEqual( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real == eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareNotEqual( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real != eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareGreater( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real > eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareGreaterEqual( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real >= eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareLess( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real < eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _compareLessEqual( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real <= eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _logicOr( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real || eng->getScriptData( argv[1] ).real );
		eng->scriptDataAssign( argv[0], tmp );
		eng->releaseScriptData( tmp );
	}
	static void _logicAnd( script_engine * eng, size_t * argv )
	{
		size_t tmp = eng->fetchScriptData( eng->getScriptData( argv[0] ).real && eng->getScriptData( argv[1] ).real );
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
	static void _appendArray( script_engine * eng, size_t * argv )
	{
		size_t idx = invalidIndex;
		eng->scriptDataAssign( idx, argv[1] );
		eng->getScriptData( argv[0] ).vec.push_back( idx );
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
	static void _print( script_engine * eng, size_t * argv )
	{
		std::string str = eng->getStringScriptData( argv[0] );
		std::cout << str << std::endl;
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
		{ "negative", &natives::_negative, 1 },
		{ "power", &natives::_power, 2 },
		{ "concatenate", &natives::_concatenate, 2 }, 
		{ "absolute", &natives::_absolute, 1 },
		{ "not", &natives::_not, 1 },
		{ "compareEqual", &natives::_compareEqual, 2 },
		{ "compareNotEqual", &natives::_compareNotEqual, 2 },
		{ "compareGreater", &natives::_compareGreater, 2 },
		{ "compareEqualEqual", &natives::_compareGreaterEqual, 2 },
		{ "compareLess", &natives::_compareLess, 2 },
		{ "compareLessEqual", &natives::_compareLessEqual, 2 },
		{ "roof", &natives::_roof, 1 },
		{ "floor", &natives::_floor, 1 },
		{ "index", &natives::_index, 2 },
		{ "appendArray", &natives::_appendArray, 2 },
		{ "uniqueize", &natives::_uniqueize, 1 },
		{ "rand", &natives::_rand, 2 },
		{ "rand_int", &natives::_rand_int, 2 },
		{ "print", &natives::_print, 1 }
	};
	for( unsigned i = 0; i <  sizeof( funcs ) / sizeof( native_function ); ++i )
	{
		symbol * s = search( funcs[i].name );
		if( s )
			raiseError( std::string() + "\"" + funcs[i].name +"\" "+ "has already been defined" , error::er_syntax );
		unsigned blockIndex = engine.fetchBlock();
		block & b = engine.getBlock( blockIndex );
		b.hasResult = true;
		b.kind = block::bk_function;
		b.name = funcs[i].name;
		b.argc = funcs[i].argc;
		b.nativeCallBack = funcs[i].nCallBack;
		symbol sym;
		sym.blockIndex = blockIndex;
		sym.id = invalidIndex;
		sym.level = 0;
		vecScope[0][ funcs[i].name ] = sym;
	}
}
void parser::writeOperation( std::string const & nativeFunc )
{
	symbol * func;
	if( !(func = search( nativeFunc )) || func->blockIndex == invalidIndex )
		raiseError( "parser::writeOperation", error::er_internal );
	block & blockFunc = engine.getBlock( func->blockIndex );
	if( !(blockFunc.hasResult && blockFunc.nativeCallBack != 0) )
		raiseError( "parser::writeOperation", error::er_internal );
	blockFunc.vecCodes.push_back( code::subArg( vc_callFunctionPush, func->blockIndex, blockFunc.argc ) );
}