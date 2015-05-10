#pragma once
#include <vector>
//#include <lightweight_vector.hpp>
using std::vector;
//#define vector lightweight_vector
typedef unsigned size_t;

template< typename T >
inline bool CheckValidIdx( T const val )
{
	return val != (T)-1;
}

enum ObjType
{
	ObjShot, ObjEffect, ObjPlayer, ObjFont
};