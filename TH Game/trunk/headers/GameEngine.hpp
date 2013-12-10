#pragma once
#include <Direct3DEngine.hpp>
#include <scriptengine.hpp>
#include <Windows.h>

class GameEngine : public Direct3DEngine, public script_engine
{
public:
	GameEngine( HWND hWnd ) : Battery( hWnd )
	{
	}
	LPDIRECT3DDEVICE9 GetDevice(){ return Battery::GetDevice(); }
};