#pragma once
#include <vector>
#include <map>
#include "defstypedefs.hpp"
#include "scriptengine.hpp"

class script_machine
{
private:
	script_engine * engine;
public:
	void advance();
};