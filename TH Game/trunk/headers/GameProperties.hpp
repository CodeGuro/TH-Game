#pragma once

class GameProperties
{
private:
	size_t points;
	size_t lives; //(USHORT)-1 is inf
	float volume; // 0 - 100

public:
	GameProperties() : points( 0 ), lives( 0 ), volume( 0.f )
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

	float get_volume()
	{
		return volume;
	}

	void set_volume( float volume )
	{
		this->volume = volume;
	}
};