#include <iostream>
#include <scriptengine.hpp>
#include <array>
int main( int const argc, char const * const * argv )
{
	std::cout << "Hello, world!\n";
	script_engine engine;
	engine.start();
	std::cin.get();
	return EXIT_SUCCESS;
}