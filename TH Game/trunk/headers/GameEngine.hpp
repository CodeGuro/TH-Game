#pragma once
#include <Direct3DEngine.hpp>
#include <scriptengine.hpp>
#include <Windows.h>

class GameEngine : public Direct3DEngine
{
private:
	script_engine script_mgr;
public:
	GameEngine( HWND hWnd ) : Direct3DEngine( hWnd ), script_mgr( this )
	{
	}
	bool start()
	{
		return script_mgr.start();
	}
	bool advance( MSG msg )
	{
		
		script_mgr.advance();
		RenderFrame( msg );
		return script_mgr.IsFinished();
	}
};