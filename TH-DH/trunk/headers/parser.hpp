#pragma once
#include "scriptengine.hpp"
#include "bytecode.hpp"
#include <string>

/*To parse the script into bytecode*/
class parser
{
	friend class parser_interface;
private:
	enum token
	{
		tk_end, tk_lparen, tk_rparen, tk_openbra, tk_closebra, tk_opencur, tk_closecur, tk_openabs, tk_closeabs,
		tk_not, tk_less, tk_lessequal, tk_greater, tk_greaterequal, tk_compare, tk_compare_not, tk_assign, tk_ampersand, tk_and, tk_or,
		tk_plus, tk_minus, tk_asterisk, tk_slash, tk_caret, tk_percent, tk_tilde, tk_increment, tk_decrement,
		tk_add_assign, tk_subtract_assign, tk_divide_assign, tk_multiply_assign, tk_power_assign, tk_percent_assign,
		tk_at, tk_comma, tk_semicolon, tk_sharp, tk_singlequote, tk_doublequote,
		tk_real, tk_char, tk_string, tk_word, tk_invalid,
		tk_IF, tk_ELSE, tk_WHILE, tk_LOOP, tk_YIELD, tk_BREAK, tk_LET, tk_FUNCTION, tk_TASK, tk_RETURN, tk_SCRIPT_STAGE_MAIN, tk_SCRIPT_ENEMY, 
	};
	class lexer
	{
	private:
		const char * current;
		unsigned line;
		char character;
		std::string word; //placeholder for string also

	public:
		void skip();
		token advance();
		const std::string & get_string() const;
		const std::string & get_word() const;
		unsigned get_line() const;
		char get_character() const;
	};
	struct identifier
	{
		size_t id;
		size_t blockIndex;
	};
	struct scope : std::map< std::string, identifier >
	{
		block::block_kind blockKind;
	};
	struct docHandler
	{
		vector< std::string > fullPathDocs;
		std::string currentDoc;
		std::string docString;
	};

	script_engine & engine;
	docHandler handler;

	void findDocument( std::string const & pathDoc );
public:
	parser( script_engine & eng ); 

};