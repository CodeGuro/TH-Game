#pragma once
#include <stdexcept>
#include <string>

class FatalException : public std::exception
{
private:
	std::string message;
public:
	FatalException()
	{
	}
	FatalException( std::string msg ) : message( msg )
	{
	}
	virtual const char * what() const
	{
		return message.c_str();
	}

};