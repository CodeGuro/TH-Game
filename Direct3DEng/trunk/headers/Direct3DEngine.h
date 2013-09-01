#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <string>
#include <Windows.h>

struct Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex;
	D3DCOLOR color;
};

struct Object
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 velocity;
	D3DXVECTOR3 accel;
	D3DXVECTOR3 scale;
	D3DXQUATERNION direction;
	D3DXQUATERNION orient;
	D3DXQUATERNION orientvel;
	ULONG libidx;

	void SetSpeed( float Speed );
	void SetVelocity( D3DXVECTOR3 Velocity );
	void SetAccel( D3DXVECTOR3 Accel );
	void SetPosition( D3DXVECTOR3 Position );
	void SetScale( D3DXVECTOR3 Scaling );
	void SetAngle( float Theta );
	void SetAngleEx( D3DXVECTOR3 Axis, float Theta );
	void SetRotation( float Theta );
	void SetRotationEx( D3DXVECTOR3 Axis, float Theta );
	void SetRotationVelocity( float Theta );
	void SetRotationVelocityEx( D3DXVECTOR3 Axis, float Theta );
	void Advance();
};

class ObjMgr
{
private:
	unsigned VertexCount;
	LPDIRECT3DTEXTURE9 pTexture;
	D3DSURFACE_DESC SurfaceDesc;
	std::vector< Vertex > vecVertexLibrary;
	std::vector< Vertex > vecVertices;
	std::vector< Object > vecObjects;
	std::vector< unsigned > vecIntermediateLayer;
	std::vector< unsigned > vecIntermediateLayerGC;
public:
	ObjMgr();
	ObjMgr( ObjMgr const & source );
	ObjMgr & operator = ( ObjMgr const & source );
	~ObjMgr();
	void SetVertexCount( unsigned const Count );
	void SetTexture( LPDIRECT3DTEXTURE9 pTex );
	void PushQuadLib( RECT Quad, D3DCOLOR Color );
	void PushVertexLib( std::vector< Vertex > const & VecVerts );
	unsigned PushObj( unsigned const Index );
	void EraseObj( unsigned Index );
	Object & GetObjRef( unsigned Index );
	Object * GetObjPtr( unsigned Index );
	D3DSURFACE_DESC GetSurfaceDesc();
	void AdvanceTransformed();
};

class Direct3DEngine
{
private:
	struct GenCameraSetting
	{
		D3DXMATRIX WorldMat;
		D3DXMATRIX ViewMat;
		D3DXMATRIX ProjMat;
	};
	struct Battery
	{
		LPDIRECT3DVERTEXSHADER9 pDefault3DVShader;
		LPDIRECT3DPIXELSHADER9 pDefault3DPShader;
		LPD3DXCONSTANTTABLE pDefaultConstable;

		std::map< std::string, LPDIRECT3DTEXTURE9 > mapTextures;
		std::vector< ObjMgr > vecObjManagers;
	};
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3ddev;
	GenCameraSetting CamSetting;
	Battery inventory;
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
	void ToggleWindowed();
	void LoadTexture( std::string const pathname );
	void DeleteTexture( std::string const pathname );
	void DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing );
	void DrawTexture();
	void DrawFPS();
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );
};