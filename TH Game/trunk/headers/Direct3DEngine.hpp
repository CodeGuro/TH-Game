#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <Windows.h>
#include <ObjMgr.hpp>
#include <D3DSmartPtr.hpp>

enum ObjType
{
	ObjShot, ObjEffect
};

struct ObjHandle
{
	unsigned MgrIdx;
	unsigned short Layer;
	unsigned short RefCount;
	unsigned ObjVector;
	unsigned ObjVectorIdx;
	unsigned VertexBuffer;
	ObjType Type;
};

struct Layer
{
	std::vector< ObjMgr > vObjMgr;
};

struct VBuffer
{
	std::vector< Vertex > VertexBuffer;
	ULONG RefCount;
};

struct D3DVBuffer
{
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > Buffer;
	ULONG BufferSize;
};

class Battery
{
private:
friend class ObjMgr;
	D3DSmartPtr< LPDIRECT3D9 > d3d;
	D3DSmartPtr< LPDIRECT3DDEVICE9 > d3ddev;
	D3DSmartPtr< LPDIRECT3DVERTEXDECLARATION9 > pDefaultVDeclaration;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > pDefault3DVShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > pDefault3DPShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > pDefaultConstable;
	
	typedef std::vector< Layer > vLayer_t;
	D3DVBuffer PipelineVertexBuffer;
	std::map< std::string, LPDIRECT3DTEXTURE9 > mapTextures;
	vLayer_t vLayers;
	std::vector< ShotData > Bullet_Templates;
	std::vector< DelayData > Bullet_Delays;
	std::vector< std::vector< Object > > vvObjects;
	std::vector< unsigned > vvObjectsGC;
	std::vector< ObjHandle > vObjHandles;
	std::vector< unsigned > vObjHandlesGC;
	std::vector< VBuffer > vVertexBuffers;
	std::vector< unsigned > vVertexBuffersGC;
	std::string ShotImagePath;
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
	void LoadTexture( std::string const pathname );
	void DeleteTexture( std::string const pathname );

	unsigned CreateShot01( D3DXVECTOR2 const & position, FLOAT const speed, FLOAT const direction, FLOAT const graphic );
	void PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color );
	void LoadShotImage( std::string const & pathname );

	//ObjEffect Functions
	void ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount );
	void ObjEffect_SetVertexXY( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR2 Posxy );
	void ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv );
	void ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color );
	void ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState );
	void ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType );
	void ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer );

public:
	LPDIRECT3DTEXTURE9 GetTexture( std::string const & pathname );
	LPDIRECT3DDEVICE9 & GetDevice();
	LPDIRECT3D9 & GetD3D();
	D3DVBuffer & GetPipelineVBuffer();
	LPDIRECT3DVERTEXDECLARATION9 GetDefaultVDeclaration() const;
	LPDIRECT3DVERTEXSHADER9 GetDefaultVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefaultPShader() const;
	LPD3DXCONSTANTTABLE GetDefaultConstable() const;

	void CreateShotData( unsigned ID, BlendType blend, unsigned delay, RECT const & rect, D3DCOLOR color, DWORD flags, std::vector< std::vector< float > > const & AnimationData );
	void CreateDelayShotData( unsigned ID, RECT const & rect, D3DCOLOR const color, FLOAT const Scale, ULONG const DelayFrames );
	ShotData const & GetBulletTemplates( unsigned const graphic ) const;
	unsigned GetDelayDataSize() const;
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