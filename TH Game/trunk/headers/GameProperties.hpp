#pragma once

#pragma once

class GameProperties
{
private:
	size_t points;
	size_t lives; //(USHORT)-1 is inf

public:
	GameProperties() : points( 0 ), lives( 0 )
	{
	}

	size_t get_points()
	{
		return points;
	}

	void set_points( size_t points )
	{
		this->points = points;
	}

	size_t get_lives()
	{
		return lives;
	}

	void set_lives( size_t lives )
	{
		this->lives = lives;
	}
};