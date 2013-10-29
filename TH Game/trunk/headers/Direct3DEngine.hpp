#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <string>
#include <Windows.h>
#include <ObjMgr.hpp>
#include <D3DSmartPtr.hpp>

struct ObjHandle
{
	unsigned ObjIdx;
	unsigned MgrIdx;
	unsigned short Layer;
	unsigned short RefCount;
};

struct Layer
{
	std::vector< ObjMgr > vObjMgr;
};

class Battery
{
private:
	D3DSmartPtr< LPDIRECT3D9 > d3d;
	D3DSmartPtr< LPDIRECT3DDEVICE9 > d3ddev;
	D3DSmartPtr< LPDIRECT3DVERTEXDECLARATION9 > pDefaultVDeclaration;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > pDefault3DVShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > pDefault3DPShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > pDefaultConstable;
		
	typedef std::vector< Layer > vLayer_t;
	std::map< std::string, LPDIRECT3DTEXTURE9 > mapTextures;
	vLayer_t vLayers;
	std::vector< ObjHandle > vObjHandles;
	std::vector< unsigned > vObjHandlesGC;

protected:
	D3DXMATRIX WorldMatrix;
	D3DXMATRIX ViewMatrix;
	D3DXMATRIX ProjectionMatrix;
	vLayer_t & GetLayers();
	unsigned CreateObject( unsigned short Layer );
	void AddRefObjHandle( unsigned HandleIdx );
	void ReleaseObjHandle( unsigned HandleIdx );
	void ReleaseObject( unsigned HandleIdx );
	Object * GetObject( unsigned HandleIdx );
	ObjMgr * GetObjMgr( unsigned HandleIdx );

public:
	void LoadTexture( std::string const pathname );
	void DeleteTexture( std::string const pathname );
	LPDIRECT3DTEXTURE9 GetTexture( std::string const & pathname );
	LPDIRECT3DDEVICE9 & GetDevice();
	LPDIRECT3D9 & GetD3D();
	LPDIRECT3DVERTEXDECLARATION9 GetDefaultVDeclaration() const;
	LPDIRECT3DVERTEXSHADER9 GetDefaultVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefaultPShader() const;
	LPD3DXCONSTANTTABLE GetDefaultConstable() const;

	Battery( HWND const hWnd );
	Battery();
};

class Direct3DEngine : protected virtual Battery
{
public:
	friend class ObjMgr;
	Direct3DEngine();
	/* We will probably not need move semantics
	Direct3DEngine & operator = ( Direct3DEngine && source );
	Direct3DEngine( Direct3DEngine && source ); */
	void ToggleWindowed();
	void DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing );
	void DrawTexture();
	void DrawFPS();
	void DrawObjects();
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );

	void SetFog( D3DCOLOR Color, float Near, float Far );
};