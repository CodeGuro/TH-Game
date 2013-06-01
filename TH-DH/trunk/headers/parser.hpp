#pragma once
#include "scriptengine.hpp"
#include "bytecode.hpp"
#include <string>
#include <map>

/*To parse the script into bytecode*/
class parser
{
private:
	enum token
	{
		tk_end, tk_lparen, tk_rparen, tk_openbra, tk_closebra, tk_opencur, tk_closecur, tk_openabs, tk_closeabs,
		tk_compare_less, tk_compare_lessequal, tk_compare_greater, tk_compare_greaterequal, tk_compare_equal, tk_compare_notequal,
		 tk_assign, tk_ampersand, tk_vertical, tk_and, tk_or,
		tk_plus, tk_minus, tk_asterisk, tk_slash, tk_caret, tk_percent, tk_tilde, tk_increment, tk_decrement, tk_not,
		tk_add_assign, tk_subtract_assign, tk_divide_assign, tk_multiply_assign, tk_power_assign, tk_remainder_assign,
		tk_at, tk_comma, tk_semicolon, tk_sharp, tk_dot,
		tk_real, tk_character, tk_string, tk_word, tk_invalid,
		tk_IF, tk_ELSE, tk_WHILE, tk_LOOP, tk_YIELD, tk_BREAK, tk_LET, tk_FUNCTION, tk_TASK, tk_RETURN, tk_SCRIPT_STAGE_MAIN, tk_SCRIPT_ENEMY,
		tk_TOUHOUDANMAKU, tk_INCLUDE
	};
	class lexer
	{
	private:
		const char * current;
		unsigned line;
		char character;
		float real;
		std::string word; //placeholder for string also
		token next;

	public:
		lexer( char const * strstart );
		lexer();
		void skip(); //whitespace
		token advance(); //get the next token
		token getToken() const;
		const std::string & getString() const;
		const std::string & getWord() const;
		unsigned getLine() const;
		float getReal() const;
		char getCharacter() const;
	};
	struct symbol
	{
		size_t id;
		size_t level; //for script environment runtime
		size_t blockIndex;
	};
	struct scriptHandler
	{
		struct scriptObj
		{
			std::map< std::string , symbol > exportSymbols;
			bool finishParsed;
		};
		std::map< std::string, scriptObj > scriptUnits;
		std::string currentScriptPath; //currently working on
		std::string scriptString; //current
		vector< std::string > pragmaFiles; //for the current unit
	};
	struct scope : std::map< std::string, symbol >
	{
		size_t blockIndex;
		vector< std::string > args;
	};
	struct error
	{
		enum errReason
		{
			er_syntax, er_parser, er_internal
		};
		errReason reason;
		std::string pathDoc;
		unsigned line;
		std::string fivelines;
		std::string errmsg;
	};

	script_engine & engine;
	scriptHandler scriptMgr;
	lexer lexicon;
	vector< scope > vecScope;

	symbol * search( std::string const & name );
	symbol * searchResult();
	void findDocument( std::string const & pathDoc );
	void parseParentheses();
	void parseClause();
	void parsePrefix();
	void parseSuffix();
	void parseProduct();
	void parseSum();
	void parseComparison();
	void parseLogic();
	void parseExpression();
	unsigned parseArguments();
	void parseStatements();
	void parseInlineBlock( block::block_kind kind ); //auto-call (if/loops)
	void parseBlock( block::block_kind kind, vector< std::string > const args );
	void scanCurrentScope( block::block_kind kind, vector< std::string > const args );
	void parsePreProcess();
	void parseScript( std::string const & scriptPath );
	block & getBlock(); //get the current working block
	void pushCode( code const & val ); //on the current working block
	void mapScriptPaths( std::string const & pathStart );
	void registerScript( std::string const & fullPath );
	scriptHandler::scriptObj & getScript( std::string const & fullPath );
	void raiseError( std::string errmsg, error::errReason reason );
	void importExportSymbols( std::string const & fullPath );
	void importNativeSymbols(); //in the topmost scope
public:
	parser( script_engine & eng ); //automatic parsing, feed data to the engine's battery

};