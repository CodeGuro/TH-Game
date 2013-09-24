#include <iostream>
#include <scriptengine.hpp>
#include <array>

int main( int const argc, char const * const * const argv )
{
	script_engine engine;
	engine.start();
	while( true )
		engine.advance();
	std::cin.get();
	return EXIT_SUCCESS;
}