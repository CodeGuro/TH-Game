#pragma once
#include "scriptengine.hpp"
#include "bytecode.hpp"
#include <string>

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
	};
	class lexer
	{
	private:
		const char * current;
		unsigned line;
		char character;
		float real;
		std::string word; //placeholder for string also

	public:
		void skip(); //whitespace
		token advance(); //get the next token
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
	struct scope : std::map< std::string, symbol >
	{
		block::block_kind blockKind;
	};
	struct docHandler
	{
		vector< std::string > fullPathDocs;
		std::string currentDoc;
		std::string docString;
	};
	struct blockMetaData
	{
		size_t blockIndex;
		vector< std::string > args;
		bool hasResult;
	};

	script_engine & engine;
	docHandler handler;
	vector< scope > vecScope;

	symbol * search( std::string const & str );
	symbol * searchResult();
	void findDocument( std::string const & pathDoc );
	void parseParentheses( size_t blockIndex );
	void parseClause( size_t blockIndex );
	void parsePrefix( size_t blockIndex );
	void parseSuffix( size_t blockIndex );
	void parseProduct( size_t blockIndex );
	void parseSum( size_t blockIndex );
	void parseComparison( size_t blockIndex );
	void parseLogic( size_t blockIndex );
	void parseExpression( size_t blockIndex );
	unsigned parseArguments( size_t blockIndex );
	void parseStatements( size_t blockIndex );
	void parseInline_block( size_t blockIndex );
	void parseBlock( blockMetaData const & properties );
	void scanCurrentScope( blockMetaData const & properties );
public:
	parser( script_engine & eng ); //automatic parsing, feed data to the engine's battery

};