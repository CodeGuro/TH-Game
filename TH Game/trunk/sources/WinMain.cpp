#include <Windows.h>
#include <D3DX10math.h>
#include <GameEngine.hpp>

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
	float pi = acos( 0.f );
	D3DXQUATERNION rot, ang, rot_inv, buff;
	D3DXQuaternionRotationAxis( &buff, &D3DXVECTOR3( 1.f, 0.f, 0.f ), acos( 0.f ) );
	ang = D3DXQUATERNION( 0.f, 0.f, 1.f, 0.f );
	D3DXQuaternionInverse( &rot_inv, D3DXQuaternionNormalize( &rot, &D3DXQUATERNION( sin( pi ), sin( pi), sin(pi), cos(pi) ) ) );

	D3DXQuaternionMultiply( &buff, D3DXQuaternionMultiply( &buff, &rot, &ang ), &rot_inv );

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
	GameEngine engine( hWnd );
	if( !engine.start() )
		return EXIT_FAILURE;

	do
	{
		PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}while( msg.message != WM_QUIT && !engine.advance( msg ) );

	return EXIT_SUCCESS;
}