#pragma once
#include <Direct3DEngine.hpp>
#include <scriptengine.hpp>
#include <Windows.h>

class GameEngine
{
private:
	Direct3DEngine draw_mgr;
	script_engine script_mgr;
	GameProperties game_properties;
public:
	GameEngine( HWND hWnd ) : draw_mgr( hWnd ), script_mgr( &draw_mgr, &game_properties )
	{
	}
	bool start()
	{
		return script_mgr.start();
	}
	bool advance( MSG msg )
	{
		script_mgr.run();
		draw_mgr.RenderFrame( msg );
		return script_mgr.IsFinished();
	}
};