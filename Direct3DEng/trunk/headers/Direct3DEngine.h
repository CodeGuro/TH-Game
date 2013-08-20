#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <string>
#include <Windows.h>

struct Vertex
{
	float posx;
	float posy;
	float posz;
	float posu;
	float posv;
	D3DCOLOR color;
};

struct MatrixObject
{
	D3DXMATRIX spacial;
	D3DXVECTOR3 velocity;
	D3DXVECTOR3 accel;
	D3DXQUATERNION orientvel;

	void SetSpeed( float Speed );
	void SetVelocity( D3DXVECTOR3 Velocity );
	void SetPosition( D3DXVECTOR3 Position );
	void SetScale( D3DXVECTOR3 Scale );
	void SetAngle( float Theta );
	void SetAngleEx( D3DXVECTOR3 Axis, float Theta );
	void SetAngularVelocity( float Theta );
	void SetAngularVelocityEx( D3DXVECTOR3 Axis, float Theta );
	void Advance();
};

class ObjMgr
{
private:
	unsigned VertexCount;
	LPDIRECT3DTEXTURE9 pTexture;
	std::vector< Vertex > vecVertices;
	std::vector< MatrixObject > vecObjects;
	std::vector< unsigned > vecIntermediateLayer;
	std::vector< unsigned > vecIntermediateLayerGC;
public:
	ObjMgr();
	ObjMgr( ObjMgr const & source );
	ObjMgr & operator = ( ObjMgr const & source );
	~ObjMgr();
	void SetVertexCount( unsigned const Count );
	void SetTexture( LPDIRECT3DTEXTURE9 pTex );
	unsigned PushObj( MatrixObject const Obj , Vertex const * PtrVertices);
	void EraseObj( unsigned Index );
	MatrixObject & GetObjRef( unsigned Index );
	MatrixObject * GetObjPtr( unsigned Index );
	void Advance();
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
		std::map< std::string, LPDIRECT3DTEXTURE9 > mapTextures;
		LPDIRECT3DVERTEXSHADER9 pDefault3DVShader;
		LPDIRECT3DPIXELSHADER9 pDefault3DPShader;
		LPD3DXCONSTANTTABLE pDefaultConstable;
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
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );
};