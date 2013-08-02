#include <Windows.h>
#include <Direct3DEngine.h>

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg == WM_DESTROY )
	{
		PostQuitMessage( 0 );
		return 0;
	}
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = CreateSolidBrush( RGB(0,0,0) );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Direct3D Eng";
	
	if( !RegisterClass( &wc ) )
		return EXIT_FAILURE;

	RECT rec = { 100, 100, 740, 580 };
	AdjustWindowRect( &rec, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, FALSE );
	HWND hWnd = CreateWindow( "Direct3D Eng", "DirectX Engine", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		rec.left, rec.top, rec.right - rec.left, rec.bottom - rec.top, NULL, NULL, hInstance, NULL );
	if( !hWnd )
		return EXIT_FAILURE;
	
	MSG msg;
	Direct3DEngine D3dEng;
	D3dEng.InitEng( hWnd, MessageBox(NULL, "Fullscreen Mode?", "FULLSCREEN/WINDOW", MB_YESNO | MB_ICONQUESTION ) == IDNO );
	D3dEng.TestDevice();
	//we'll now attempt to draw a simple texture to the focus window
	do
	{
		PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}while( msg.message != WM_QUIT );

	return EXIT_SUCCESS;
}