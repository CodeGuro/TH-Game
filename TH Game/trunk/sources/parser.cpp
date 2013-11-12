#include <parser.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <math.h>
#include <sstream>
#include <scriptmachine.hpp>
#define CSTRFUNCRESULT "(RESULT)"

parser::lexer::lexer() : current( &character ), next( tk_end ), character( '\0' )
{
}
parser::lexer::lexer( char const * strstart ) : current( strstart ), line( 1 )
{
	advance();
}
parser::lexer::lexer( char const * strstart, unsigned lineStart, token tokStart ) : current( strstart ), next( tokStart), line( lineStart )
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
			if( word.size() )
				character = word[0];
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
				else if( word == "yield" )
					next = tk_YIELD;
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
	std::string operation;
	parseSum();
	do
	{
		operation.clear();
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
			parseSum();
			writeOperation( operation );
		}
	}while( operation.size() );
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
	while( lexicon.getToken() == tk_caret || lexicon.getToken() == tk_openbra )
	{
		token tok = lexicon.getToken();
		lexicon.advance();
		parseSuffix();
		writeOperation( std::string( tok == tk_caret ? "power" : "index" ) );
		if( tok == tk_openbra )
			lexicon.getToken() != tk_closebra ? raiseError( "\"]\" is missing for array indexing", error::er_syntax ) : lexicon.advance();
	}
}
void parser::parseClause()
{
	switch( lexicon.getToken() )
	{
	case tk_real:
		pushCode( code::dat( vc_pushVal, fetchScriptData( lexicon.getReal() ) ) );
		lexicon.advance();
		break;
	case tk_character:
		pushCode( code::dat( vc_pushVal, fetchScriptData( lexicon.getCharacter() ) ) );
		lexicon.advance();
		break;
	case tk_string:
		pushCode( code::dat( vc_pushVal, fetchScriptData( lexicon.getString() ) ) );
		writeOperation( "uniqueize" );
		lexicon.advance();
		break;
	case tk_word:
		{
			std::string name = lexicon.getWord();
			symbol * sym = search( name );
			if( !sym )
				raiseError( lexicon.getWord(), error::er_symbol );
			lexicon.advance();
			if( sym->blockIndex != invalidIndex )
			{
				if( inventory::getBlock( sym->blockIndex ).kind != block::bk_function )
					raiseError( std::string() + "\"" + name + "\"" + " is not a function", error::er_syntax );
				int argc;
				if( (argc = parseArguments()) != inventory::getBlock( sym->blockIndex ).argc )
					raiseError( "wrong number of arguments", error::er_syntax );
				pushCode( code::subArg( vc_callFunctionPush, sym->blockIndex, argc ) );
			}
			else
				pushCode( code::varLev( vc_pushVar, sym->id, vecScope.size() - sym->level ) );

		}
		break;
	case tk_lparen:
		{
			parseParentheses();
		}
		break;
	case tk_openbra:
		{
			size_t scriptDataIdx = fetchScriptData();
			getScriptData( scriptDataIdx ).type = getArrayType();
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
			if( lexicon.advance() != tk_rparen )
			{
				parseExpression();
				++argc;
			}
		}while( lexicon.getToken() == tk_comma );

		if( lexicon.getToken() != tk_rparen )
			raiseError( "\"(\" Expected", error::er_syntax );
		lexicon.advance();
	}
	return argc;
}
block & parser::getBlock()
{
	return inventory::getBlock( vecScope[ vecScope.size() - 1 ].blockIndex );
}
void parser::pushCode( code const & val )
{
	getBlock().vecCodes.push_back( val );
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

	if( scriptMgr.pragmaFiles.size() )
		inventory::registerInvalidMainScript( scriptMgr.pragmaFiles[ 0 ] );
	scriptMgr.pragmaFiles.resize( 0 );
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

	throw err;
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
void parser::parseScript( std::string const & scriptPath )
{
	try
	{
		vecScope.resize( 1 );
		vecScope.back().clear();
		vecScope.back().blockIndex = invalidIndex;
		parseDocument( std::string( "[natives]" ), std::string() );
		vecScope.push_back( scope() );
		vecScope.back().blockIndex = invalidIndex;
		scriptMgr.currentScriptPath = scriptPath;
		scriptMgr.scriptString = std::string( (std::istreambuf_iterator< char >( std::ifstream( scriptPath ) )), std::istreambuf_iterator< char >() );
		if( !scriptMgr.scriptString.size() )
			raiseError( std::string() + "Invalid document \"" + scriptMgr.currentScriptPath + "\"", error::er_parser );
		lexicon = lexer( scriptMgr.scriptString.c_str() );
		parseDocument( scriptPath, scriptMgr.scriptString );
		if( inventory::findScriptFromFile( scriptMgr.currentScriptPath ) == invalidIndex )
			inventory::registerInvalidMainScript( scriptMgr.currentScriptPath );
	}
	catch( error const & err )
	{
		if( scriptMgr.pragmaFiles.size() )
			inventory::registerInvalidMainScript( scriptMgr.pragmaFiles[ 0 ] );
		scriptMgr.pragmaFiles.resize( 0 );
	}
}
void parser::parseBlock( symbol const symSub, vector< std::string > const & args )
{
	if( lexicon.getToken() != tk_opencur )
		raiseError( "\"{\" expected", error::er_syntax );
	lexicon.advance();
	vecScope.push_back( scope() );
	vecScope[ vecScope.size() - 1 ].blockIndex = symSub.blockIndex;
	scanCurrentScope( inventory::getBlock( symSub.blockIndex ).kind, args );
	for( unsigned i = 0; i < args.size(); ++i )
		pushCode( code::varLev( vc_assign, search( args[i] )->id, 0 ) );
	parseStatements();
	vecScope.pop_back();

	if( lexicon.getToken() != tk_closecur )
		raiseError( "\"}\" expected", error::er_syntax );
	lexicon.advance();
}
void parser::parseInlineBlock( block::block_kind const bk_kind )
{
	size_t blockIndex = inventory::fetchBlock();
	block & inlineBlock = inventory::getBlock( blockIndex );
	inlineBlock.argc = 0;
	inlineBlock.kind = bk_kind;
	inlineBlock.nativeCallBack = 0;
	inlineBlock.name = "inlinedBlock";
	symbol virtualSymbol;
	virtualSymbol.blockIndex = blockIndex;
	virtualSymbol.level = vecScope.size() + 1;
	virtualSymbol.id = (size_t)-1;
	parseBlock( virtualSymbol, vector< std::string >() );
	pushCode( code::subArg( vc_callFunction, blockIndex, 0 ) );
}
void parser::scanCurrentScope( block::block_kind kind, vector< std::string > const & args )
{
	//if it's a function, allow a result
	unsigned id = 0;
	unsigned level = vecScope.size();
	scope & currentScope = vecScope[ vecScope.size() -1 ];
	if( kind == block::bk_function )
	{
		symbol result;
		result.blockIndex = -1;
		result.id = id++;
		result.level = level;
		currentScope[ CSTRFUNCRESULT ] = result;
	}
	for( unsigned i = 0; i < args.size(); ++i )
	{
		symbol arg;
		arg.blockIndex = -1;
		arg.id = id++;
		arg.level = level;
		currentScope[ args[i] ] = arg;
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
					routine.blockIndex = inventory::fetchBlock();
					routine.id = -1;
					routine.level = level + 1;
					currentScope[ subroutine ] = routine;
					block & blockRoutine = inventory::getBlock( routine.blockIndex );
					blockRoutine.kind = (tok == tk_FUNCTION? block::bk_function : (tok == tk_TASK? block::bk_task : (tok == tk_at? block::bk_sub : block::bk_normal )));
					blockRoutine.name = subroutine;
					blockRoutine.nativeCallBack = 0;
					blockRoutine.argc = 0;
					if( tok == tk_SCRIPT_MAIN || tok == tk_SCRIPT_ENEMY )
					{
						tok == tk_SCRIPT_MAIN ? inventory::registerMainScript( scriptMgr.currentScriptPath, subroutine ) : inventory::registerScript( subroutine );
						inventory::getScript( subroutine )->ScriptBlock = routine.blockIndex;
					}
					else if( lexicon.advance() == tk_lparen )
					{
						do
						{
							if( lexicon.advance() == tk_LET )
							{
								if( lexicon.advance() == tk_word )
									++blockRoutine.argc;
							}
							else if( lexicon.getToken() == tk_rparen )
								break;
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
							while( *(--it2) != '\\' );
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
					lexicon.advance();
					parseDocument( fullPath, scriptstr );
				}
			}
			needSemicolon = false;
		}

		else if( lexicon.getToken() == tk_FUNCTION || lexicon.getToken() == tk_at 
			|| lexicon.getToken() == tk_TASK || lexicon.getToken() == tk_SCRIPT_MAIN 
			|| lexicon.getToken() == tk_SCRIPT_ENEMY )
		{
			token routineDeclToken = lexicon.getToken();
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
			if( ( routineDeclToken == tk_SCRIPT_MAIN || routineDeclToken == tk_SCRIPT_ENEMY) && args.size() )
				raiseError( "script_main and script_enemy routine types have zero parameters", error::er_syntax );
			if( routineDeclToken == tk_at )
			{
				if( inventory::getBlock( subsym->blockIndex ).kind == block::bk_function )
					raiseError( std::string() + "\"" + subname + "\" must be prefixed with \"@\"", error::er_syntax );
				script_container * s_cont = getScript( getBlock().name );
				if( !s_cont ) raiseError( std::string() +"@\"" + subname + "\" must be defined 1 level above the scope of script's block", error::er_parser );
				if( subname == "Initialize" ) s_cont->InitializeBlock = subsym->blockIndex;
				else if( subname == "MainLoop" ) s_cont->MainLoopBlock = subsym->blockIndex;
				else if( subname == "Finalize" ) s_cont->FinalizeBlock = subsym->blockIndex;
				else if( subname == "BackGround" ) s_cont->BackGroundBlock = subsym->blockIndex;
				else raiseError( "Parser::ParseStatements unexpected error at parsing @routines", error::er_parser );
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
				pushCode( code::varLev( vc_pushVar, sym->id, vecScope.size() - sym->level ) );
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
			else if( lexicon.getToken() == tk_increment || lexicon.getToken() == tk_decrement )
			{
				pushCode( code::varLev( vc_pushVar, sym->id, vecScope.size() - sym->level ) );
				writeOperation( std::string() + (lexicon.getToken() == tk_increment ? "increment" : "decrement" ) );
				pushCode( code::varLev( vc_assign, sym->id, vecScope.size() - sym->level ) );
				lexicon.advance();
			}
			else
			{
				if( sym->blockIndex == invalidIndex )
					raiseError( std::string() + "\"" + lexicon.getWord() + "\" is not a routine", error::er_syntax );
				unsigned argc = parseArguments();
				if( argc != inventory::getBlock( sym->blockIndex ).argc )
					raiseError( "wrong number of arguments", error::er_syntax );
				instruction callInst = ( (inventory::getBlock( sym->blockIndex ).kind == block::bk_task)? vc_callTask : vc_callFunction );
				pushCode( code::subArg( callInst, sym->blockIndex, argc ) );
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
			
				unsigned loopBackIndex = getBlock().vecCodes.size();
				parseParentheses();
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
					if( lexicon.getToken() != tk_IF )
					{
						parseInlineBlock( block::bk_normal );
						fin = true;
					}
					else
						continue;
				}
				pushCode( code::code( vc_gotoEnd ) );
				pushCode( code::code( vc_caseNext ) );
			}while( !fin );
			pushCode( code::code( vc_caseEnd ) );
			needSemicolon = false;
		}

		else if( lexicon.getToken() == tk_YIELD )
		{
			lexicon.advance();
			pushCode( code::code( vc_yield ) );
		}

		else if( lexicon.getToken() == tk_BREAK )
		{
			bool isBreakable = false;
			for( unsigned u = vecScope.size() - 1 ; u != (unsigned)-1; --u )
			{
				if( vecScope[ u ].blockIndex != invalidIndex )
					if( inventory::getBlock( vecScope[ u ].blockIndex ).kind == block::bk_loop )
					{
						isBreakable = true;
						break;
					}
			}
			if( !isBreakable )
				raiseError( "\"break\" can only be break from loops.. use \"return\" instead", error::er_syntax );
			lexicon.advance();
			pushCode( code::code( vc_breakLoop ) );
		}

		if( needSemicolon && lexicon.getToken() != tk_semicolon )
			finished = true;
		if( lexicon.getToken() == tk_semicolon )
			lexicon.advance();
	}while( !finished );
}
void parser::parseDocument( std::string const & scriptPath, std::string const & scriptString )
{
	if( scriptMgr.includeSymbols.find( scriptPath ) == scriptMgr.includeSymbols.end() )
	{
		bool hasBeenIncluded = false;
		for( unsigned u = 0; u < scriptMgr.pragmaFiles.size(); ++ u )
		{
			if( scriptMgr.pragmaFiles[ u ] == scriptPath )
			{
				hasBeenIncluded = true;
				break;
			}
		}

		if( !hasBeenIncluded )
		{
			//save
			scope prev_scope = vecScope.back();
			unsigned lexLine = lexicon.getLine();
			unsigned lexPlace = lexicon.getCurrent() - scriptMgr.scriptString.c_str();
			token tokPlace = lexicon.getToken();

			std::string currentPath = scriptMgr.currentScriptPath;
			std::string currentScriptStr = scriptMgr.scriptString;

			//parse the new document
			scriptMgr.currentScriptPath = scriptPath;
			scriptMgr.scriptString = scriptString;

			scriptMgr.pragmaFiles.push_back( scriptPath );
			lexicon = lexer( scriptMgr.scriptString.c_str() );
			scanCurrentScope( block::bk_normal, vector< std::string >() );
			parseStatements();
			if( lexicon.getToken() != tk_end )
				raiseError( std::string() + "\"" + scriptMgr.pragmaFiles.back() + "\" did not parse fully", error::er_parser );
			scriptMgr.pragmaFiles.pop_back();

			//restore
			scriptMgr.currentScriptPath = currentPath;
			scriptMgr.scriptString = currentScriptStr;
			lexicon = lexer( scriptMgr.scriptString.c_str() + lexPlace, lexLine, tokPlace );


			for( auto it = vecScope.back().begin(); it != vecScope.back().end(); ++it ) //save recently parsed file symbols
			{
				if( prev_scope.find( it->first ) == prev_scope.end() )
					scriptMgr.includeSymbols[ scriptPath ][it->first] = it->second;
			}
		}
	}
	else
	{
		auto & include = scriptMgr.includeSymbols[ scriptPath ];
		for( auto it = include.begin(); it != include.end(); ++it )
			( vecScope.back().find( it->first ) != vecScope.back().end() ) ? raiseError( it->first, error::er_symbol ) : vecScope.back()[ it->first ] = it->second;
	}
}
std::string parser::getCurrentScriptPath() const
{
	return scriptMgr.currentScriptPath;
}
void parser::parseShotScript( std::string const & scriptPath )
{
	try
	{
		scriptMgr.scriptString = std::string( (std::istreambuf_iterator< char >( std::ifstream( scriptPath ) ) ), std::istreambuf_iterator< char >() );
		scriptMgr.currentScriptPath = scriptPath;
		if( !scriptMgr.scriptString.size() )
			raiseError( "User shot data did not load", error::er_parser );
		lexicon = lexer( scriptMgr.scriptString.c_str() );
		parseDelayData();
		parseShotData();
	}
	catch( error const & err )
	{
	}
}
void parser::parseShotData()
{
	lexer lexsave = lexicon;
	do
	{
		if( lexicon.getToken() != tk_word )
			lexicon.advance();
		else
		{
			std::string word = lexicon.getWord();
			lexicon.advance();
			if( word != "ShotData" ) continue;
			if( lexicon.getToken() != tk_opencur )
				raiseError( "\"{\" expected", error::er_syntax );
			lexicon.advance();

			size_t id = -1;
			BlendType render = BlendAlpha;
			float angular_velocity = 0;
			float rec[4] = { 0, 0, 0, 0 };
			float col[4] = { 255, 255, 255, 255 };
			vector< vector< float > > animation_data;
			unsigned delay = 0;
			DWORD flags = 0;

			while( lexicon.getToken() == tk_word )
			{
				std::string word = lexicon.getWord();

				if( word == "id" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_real )
						raiseError( "id lexer::tk_real expected", error::er_syntax );
					id = (size_t)lexicon.getReal();
					lexicon.advance();
				}
				else if( word == "render" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_word )
						raiseError( "render lexer::tk_word expected", error::er_syntax );
					if( lexicon.getWord() == "ALPHA_BLEND" ) render = BlendAlpha;
					else if( lexicon.getWord() == "ADDITIVE_BLEND" ) render = BlendAdd;
					else if( lexicon.getWord() == "SUBTRACTIVE_BLEND" ) render = BlendSub;
					else if( lexicon.getWord() == "MULTIPLY_BLEND" ) render = BlendMult;
					else raiseError( "render lexer::tk_word expected for \'ALPHA_BLEND | ADDITIVE_BLEND | SUBTRACTIVE_BLEND | MULTIPLY_BLEND\'", error::er_syntax );
					lexicon.advance();
				}
				else if( word == "rect" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_lparen )
						raiseError( "\"(\" expected", error::er_syntax );
			
					unsigned u;
					for( u = 0; lexicon.advance() == tk_real && u < 4 ; )
					{
						rec[ u++ ] = lexicon.getReal();
						if( lexicon.advance() != tk_comma )
							break;
					}

					if( lexicon.getToken() != tk_rparen )
						raiseError( "\")\" expected", error::er_syntax );
					if( u != 4 ) raiseError( "rect expects four elements", error::er_syntax );
					lexicon.advance();
				}
				else if( word == "color" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_lparen )
						raiseError( "\"(\" expected", error::er_syntax );
			
					unsigned u;
					for( u = 0; lexicon.advance() == tk_real && u < 4; )
					{
						col[ u++ ] = lexicon.getReal();
						if( lexicon.advance() != tk_comma )
							break;
					}

					if( lexicon.getToken() != tk_rparen )
						raiseError( "\")\" expected", error::er_syntax );
					if( u != 4 ) raiseError( "color expects four elements", error::er_syntax );
					lexicon.advance();
				}
				else if( word == "angular_velocity" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_real )
						raiseError( "lexer::tk_real expected", error::er_syntax );
					angular_velocity = lexicon.getReal();
					lexicon.advance();
				}
				else if( word == "AnimationData" )
				{
					if( lexicon.advance() != tk_opencur )
						raiseError( "\"{\" expected", error::er_syntax );
					unsigned j = 0;
					while( lexicon.advance() == tk_word && lexicon.getWord() == "animation_data" )
					{
						animation_data.resize( 1 + animation_data.size() );
						animation_data[ j ].resize( 5 );
						if( lexicon.advance() != tk_assign )
							raiseError( "\"=\" expected", error::er_syntax );
						if( lexicon.advance() != tk_lparen )
							raiseError( "\"(\" expected", error::er_syntax );
			
						unsigned u;
						for( u = 0; lexicon.advance() == tk_real && u < 5 ; )
						{
							animation_data[ j ][ u++ ] = lexicon.getReal();
							if( lexicon.advance() != tk_comma )
								break;
						}

						if( lexicon.getToken() != tk_rparen )
							raiseError( "\")\" expected", error::er_syntax );
						if( u != 5 ) raiseError( "animation_data expects five elements", error::er_syntax );
						++j;
					}

					if( lexicon.getToken() != tk_closecur )
						raiseError( "\"}\" expected", error::er_syntax );
				}
				else if( lexicon.getWord() == "delay" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_real )
						raiseError( "lexer::tk_real expected", error::er_syntax );
					delay = (unsigned)lexicon.getReal();
					if( GetDelayDataSize() < 1 + delay ) raiseError( "No such delay ID exists", error::er_syntax );
					lexicon.advance();
				}
				else if( lexicon.getWord() == "pixel_perfect" )
				{
					if( lexicon.advance() != tk_assign )
						raiseError( "\"=\" expected", error::er_syntax );
					if( lexicon.advance() != tk_word )
						raiseError( "lexicon::tk_word expected", error::er_syntax );
					int boolval = (( lexicon.getWord() == "TRUE" )? 1 : ( lexicon.getWord() == "FALSE" )? 0 : -1 );
					if( !(boolval == 0 || boolval == 1) )
						raiseError( "\"TRUE\" or \"FALSE\" expected", error::er_syntax );
					flags = (( boolval == 1 )? flags | 0x16 : flags & ~0x16);
					lexicon.advance();
				}
				else
					raiseError( lexicon.getWord(), error::er_usymbol );
			}

			if( lexicon.getToken() != tk_closecur )
				raiseError( "\"}\" expected", error::er_syntax );
			lexicon.advance();
			if( id == -1 ) raiseError( "\"id\" must be provided with a real number", error::er_syntax );
			RECT r = { (ULONG)rec[ 0 ], (ULONG)rec[ 1 ], (ULONG)rec[ 2 ], (ULONG)rec[ 3 ] };
			CreateShotData( id, render, delay, r, D3DCOLOR_RGBA( (UCHAR)col[ 0 ], (UCHAR)col[ 1 ], (UCHAR)col[ 2 ], (UCHAR)col[ 3 ] ), flags, animation_data );
		}
	} while( lexicon.getToken() != tk_end );
	lexicon = lexsave;
}
void parser::parseDelayData()
{
	lexer lexsave = lexicon;
	do
	{
		if( lexicon.getToken() == tk_word && lexicon.getWord() == "ShotImage" )
		{
			if( lexicon.advance() != tk_assign )
				raiseError( "\"=\" expected", error::er_syntax );
			if( lexicon.advance() != tk_string )
				raiseError( "lexer::tk_string expected", error::er_syntax );
			LoadShotImage( lexicon.getString() );
			break;
		}
		else lexicon.advance();
			
	}while( lexicon.getToken() != tk_end );
	lexsave = lexicon;
	do
	{
		token tok = lexicon.getToken();
		lexicon.advance();
		if( tok != tk_word )
			continue;
		else if( lexicon.getWord() != "DelayData" )
			continue;

		float id = -1;
		float scale = 1;
		float delayframes = 10;
		std::vector< float > rect;
		std::vector< float > col;
		if( lexicon.getToken() != tk_opencur )
			raiseError( "\"{\" expected", error::er_syntax );
		lexicon.advance();

		while( lexicon.getToken() == tk_word )
		{
			std::string word = lexicon.getWord();
			lexicon.advance();
			if( word == "id" || word == "frames" || word == "scale" )
			{
				if( lexicon.getToken() != tk_assign )
					raiseError( "\"=\" expected", error::er_syntax );
				if( lexicon.advance() != tk_real )
					raiseError( "id lexer::tk_real expected", error::er_syntax );
				((word == "id")? id : ((word == "scale")? scale : delayframes) ) = lexicon.getReal();
				lexicon.advance();
			}
			else if( word == "rect" || word == "color" )
			{
				if( lexicon.getToken() != tk_assign ) raiseError( "\"=\" expected", error::er_syntax );
				if( lexicon.advance() != tk_lparen ) raiseError( "\"(\" expected", error::er_syntax );
				for( unsigned u = 0; u < 4 && lexicon.advance() == tk_real; ++u )
				{
					word == "rect"? rect.push_back( lexicon.getReal() ) : col.push_back( lexicon.getReal() ); ;
					if( lexicon.advance() != tk_comma )
						break;
				}
				if( lexicon.getToken() != tk_rparen )
					raiseError( "\")\" expected", error::er_syntax );
				lexicon.advance();
				if( rect.size() != 4 ) raiseError( std::string() + "\"" + word + "\" size expected to have 4 elements", error::er_syntax );
			}

		}

		if( lexicon.getToken() != tk_closecur )
			raiseError( "\"}\" expected", error::er_syntax );
		lexicon.advance();

		if( id == -1 ) raiseError( "\"id\" must be provided with a real number", error::er_syntax );
		RECT r = { (ULONG)rect[ 0 ], (ULONG)rect[ 1 ], (ULONG)rect[ 2 ], (ULONG)rect[ 3 ] };
		D3DCOLOR color = D3DCOLOR_RGBA( (UCHAR)col[ 0 ], (UCHAR)col[ 1 ], (UCHAR)col[ 2 ], (UCHAR)col[ 3 ] );
		CreateDelayShotData( (ULONG)id, r, color, 1.f, 0 );
	}while( lexicon.getToken() != tk_end );
	lexicon = lexsave;
}
struct native_function
{
	char const * name;
	void (*nCallBack)( script_engine * eng, size_t * argv );
	unsigned argc;
};

parser::parser() 
{
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
		{ "print", &natives::_print, 1 },
		{ "true", &natives::_true, 0 },
		{ "false", &natives::_false, 0 },
		{ "increment", &natives::_increment, 1 },
		{ "decrement", &natives::_decrement, 1 },
		{ "ToString", &natives::_ToString, 1 },
		{ "cos", &natives::_cos, 1 },
		{ "sin", &natives::_sin, 1 },
		{ "tan", &natives::_tan, 1 },
		{ "atan", &natives::_atan, 1 },
		{ "atan2", &natives::_atan2, 2 },
		{ "CreateEnemyFromScript", &natives::_CreateEnemyFromScript, 1 },
		{ "CreateEnemyFromFile", &natives::_CreateEnemyFromFile, 1 },
		{ "QueueScriptTermination", &natives::_QueueScriptTermination, 0 },
		{ "Obj_Create", &natives::_Obj_Create, 1 },
		{ "Obj_Delete", &natives::_Obj_Delete, 1 },
		{ "Obj_SetPosition", &natives::_Obj_SetPosition, 4 },
		{ "Obj_SetTexture", &natives::_Obj_SetTexture, 2 },
		{ "Obj_SetSpeed", &natives::_Obj_SetSpeed, 2 },
		{ "Obj_SetAcceleration", &natives::_Obj_SetAcceleration, 4 },
		{ "Obj_SetAngle", &natives::_Obj_SetAngle, 2 },
		{ "Obj_SetVelocity", &natives::_Obj_SetVelocity, 4 },
		{ "Obj_CreateVertex", &natives::_Obj_CreateVertex, 2 },
		{ "Obj_SetPrimitiveType", &natives::_Obj_SetPrimitiveType, 2 },
		{ "Obj_SetRenderState", &natives::_Obj_SetRenderState, 2 },
		{ "Obj_SetVertexUV", &natives::_Obj_SetVertexUV, 4 },
		{ "Obj_SetVertexXY", &natives::_Obj_SetVertexXY, 4 },
		{ "Obj_SetVertexColor", &natives::_Obj_SetVertexColor, 6 },
		{ "Obj_SetLayer", &natives::_Obj_SetLayer, 2 },
		{ "ALPHA_BLEND", &natives::_ALPHA_BLEND, 0 },
		{ "ADDITIVE_BLEND", &natives::_ADDITIVE_BLEND, 0 },
		{ "PRIMITIVE_TRIANGLELIST", &natives::_PRIMITIVE_TRIANGLELIST, 0 },
		{ "PRIMITIVE_TRIANGLESTRIP", &natives::_PRIMITIVE_TRIANGLESTRIP, 0 },
		{ "PRIMITIVE_TRIANGLEFAN", &natives::_PRIMITIVE_TRIANGLEFAN, 0 },
		{ "LoadTexture", &natives::_LoadTexture, 1 },
		{ "LoadUserShotData", &natives::_LoadUserShotData, 1 },
		{ "CreateShot01", &natives::_CreateShot01, 5 }
	};
	for( unsigned i = 0; i <  sizeof( funcs ) / sizeof( native_function ); ++i )
	{
		unsigned blockIndex = fetchBlock();
		block & b = inventory::getBlock( blockIndex );
		b.kind = block::bk_function;
		b.name = funcs[i].name;
		b.argc = funcs[i].argc;
		b.nativeCallBack = funcs[i].nCallBack;
		symbol sym;
		sym.blockIndex = blockIndex;
		sym.id = invalidIndex;
		sym.level = 0;
		scriptMgr.includeSymbols[ "[natives]" ][funcs[i].name] = sym;
	}
}

void parser::writeOperation( std::string const & nativeFunc )
{
	symbol * func;
	if( !(func = search( nativeFunc )) || func->blockIndex == invalidIndex )
		raiseError( "parser::writeOperation", error::er_internal );
	block & blockFunc = inventory::getBlock( func->blockIndex );
	if( !(blockFunc.kind == block::bk_function && blockFunc.nativeCallBack != 0) )
		raiseError( "parser::writeOperation", error::er_internal );
	pushCode( code::subArg( vc_callFunctionPush, func->blockIndex, blockFunc.argc ) );
}