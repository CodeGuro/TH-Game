#pragma once
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
		tk_IF, tk_ELSE, tk_WHILE, tk_LOOP, tk_ASCENT, tk_DESCENT, tk_YIELD, tk_BREAK, tk_LET, tk_FUNCTION, tk_TASK, tk_RETURN, tk_SCRIPT_MAIN, tk_SCRIPT,
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
		lexer( char const * strstart, unsigned lineStart, token tokStart );
		lexer();
		void skip(); //whitespace
		token advance(); //get the next token
		token getToken() const;
		std::string getString() const;
		std::string getWord() const;
		unsigned getLine() const;
		float getReal() const;
		char getCharacter() const;
		const char * getCurrent() const;
	};
	struct symbol
	{
		size_t id;
		size_t level;
		size_t blockIndex;
	};
	struct scriptHandler
	{
		std::string currentScriptPath; //currently working on
		std::string scriptString; //current
		vector< std::string > pragmaFiles; //for the current unit
		std::map< std::string, std::map< std::string, symbol > > includeSymbols;
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
			er_syntax, er_symbol, er_usymbol, er_parser, er_internal
		};
		errReason reason;
		std::string pathDoc;
		unsigned line;
		std::string fivelines;
		std::string errmsg;
	};
	scriptHandler scriptMgr;
	lexer lexicon;
	vector< scope > vecScope;
	script_engine * eng;
	symbol * search( std::string const & name );
	symbol * searchResult();
	size_t getBlockIndex();
	void findDocument( std::string const & pathDoc );
	void writeOperation( std::string const & nativeFunc );
	void parseExpression();
	void parseLogic();
	void parseComparison();
	void parseSum();
	void parseProduct();
	void parseSuffix();
	void parsePrefix();
	void parseClause();
	void parseParentheses();
	unsigned parseArguments();
	void parseStatements();
	void parseInlineBlock( block::block_kind const bk_kind ); //auto-call (if/loops)
	void parseBlock( symbol const symSub, vector< std::string > const & args );
	void scanCurrentScope( block::block_kind kind, vector< std::string > const & args );
	block & getCurrentBlock(); //get the current working block
	void pushCode( code const & val ); //on the current working block
	void raiseError( std::string errmsg, error::errReason reason );
	void parseDocument( std::string const & scriptPath, std::string const & scriptString );
	void registerNatives();

public:
	parser( script_engine * eng );
	void parseScript( std::string const & scriptPath );
	void parseShotScript( std::string const & scriptPath );
	std::string getCurrentScriptPath() const;
};