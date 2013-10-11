#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <string>
#include <Windows.h>

class ObjMgr;

struct ObjHandle
{
	unsigned ObjIdx;
	unsigned MgrIdx;
	unsigned short Layer;
	unsigned short RefCount;
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
	struct Layer
	{
		std::vector< ObjMgr > vObjMgr;
	};
	struct Battery
	{
		LPDIRECT3DVERTEXDECLARATION9 pDefaultVDeclaration;
		LPDIRECT3DVERTEXDECLARATION9 pDefaultVtDeclaration;
		LPDIRECT3DVERTEXSHADER9 pDefault3DVShader;
		LPDIRECT3DPIXELSHADER9 pDefault3DPShader;
		LPD3DXCONSTANTTABLE pDefaultConstable;
		
		std::map< std::string, LPDIRECT3DTEXTURE9 > mapTextures;
		std::vector< Layer > vLayers;
		std::vector< ObjHandle > vObjHandles;
		std::vector< unsigned > vObjHandlesGC;
		Battery();
	};
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3ddev;
	GenCameraSetting CamSetting;
	Battery inventory;
	void InitLayers();
	void InitBattery();
	template< typename T >
	void D3dRelease( T *& RefPtr );
public:
	Direct3DEngine();
	Direct3DEngine( Direct3DEngine const & source );
	Direct3DEngine & operator = ( Direct3DEngine const & source );
	~Direct3DEngine();
	/* We will probably not need move semantics
	Direct3DEngine & operator = ( Direct3DEngine && source );
	Direct3DEngine( Direct3DEngine && source ); */
	void InitEng( HWND hWnd, bool windowed );
	void ToggleWindowed();
	void LoadTexture( std::string const pathname );
	void DeleteTexture( std::string const pathname );
	void DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing );
	void DrawTexture();
	void DrawFPS();
	void TestObjMgr();
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );
	LPDIRECT3DDEVICE9 GetDevice();

	void SetFog( D3DCOLOR Color, float Near, float Far );
	unsigned CreateObject( unsigned short Layer );
	void AddRefObjHandle( unsigned HandleIdx );
	void ReleaseObjHandle( unsigned HandleIdx );
	void ReleaseObject( unsigned HandleIdx );
};