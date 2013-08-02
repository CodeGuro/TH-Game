#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

class Direct3DEngine
{
private:
	struct GeneralCamSetting
	{
		D3DXVECTOR3 Eye;
		D3DXVECTOR3 LookAt;
		D3DXVECTOR3 UpDir;
		D3DXMATRIX WorldViewProj2D;
		D3DXMATRIX WorldViewProj3D;
	};
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3ddev;
	GeneralCamSetting CamSettings;
	template< typename T >
	void D3dRelease( T *& RefPtr );
public:
	Direct3DEngine();
	Direct3DEngine( Direct3DEngine const & source );
	Direct3DEngine & operator = ( Direct3DEngine const & source );
	~Direct3DEngine();
	/* We will probably not need move semantics
	Direct3DEngine & operator = ( Direct3DEngine && source );
	Direct3DEngine( Direct3DEngine && source );
	*/
	void InitEng( HWND hWnd, bool windowed );
	void TestDevice();
};