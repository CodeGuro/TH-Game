#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <string>
#include <Windows.h>
#include <DSound.h>
#include <ObjMgr.hpp>
#include <D3DSmartPtr.hpp>
#include <defstypedefs.hpp>

struct ObjHandle
{
	unsigned MgrIdx;
	unsigned short Layer;
	unsigned short RefCount;
	unsigned ObjVector;
	unsigned ObjVectorIdx;
	unsigned VertexBuffer;
	unsigned ObjFontIdx;
	ObjType Type;
};

struct Layer
{
	vector< ObjMgr > vObjMgr;
};

struct D3DVBuffer
{
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > Buffer;
	ULONG BufferSize;
};

struct FontObject
{
	D3DSmartPtr< LPD3DXFONT > pFont;
	std::string String;
	RECT Rect;
	DWORD Format;
	D3DCOLOR Color;
};

struct WaveHeaderType
{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
};

class Battery
{
private:
	struct VBuffer
	{
		vector< Vertex > VertexBuffer;
		ULONG RefCount;
	};
	D3DSmartPtr< LPDIRECT3D9 > d3d;
	D3DSmartPtr< LPDIRECT3DDEVICE9 > d3ddev;
	D3DSmartPtr< LPDIRECT3DVERTEXDECLARATION9 > pDefaultVDeclaration;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > pDefault3DVShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > pDefault3DPShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > pDefaultConstable;
	D3DSmartPtr< LPDIRECTSOUND8 > dsound;
	
	typedef vector< Layer > vLayer_t;
	D3DVBuffer PipelineVertexBuffer;
	std::map< std::string, D3DSmartPtr< LPDIRECT3DTEXTURE9 > > mapTextures;
	std::map< std::string, D3DSmartPtr< LPDIRECTSOUNDBUFFER8 > > mapSoundEffects;

	vLayer_t vLayers;
	vector< ShotData > Bullet_Templates;
	vector< unsigned > Bullet_TemplateOffsets;
	vector< VBuffer > VertexBuffers;
	vector< ULONG > VertexBuffersGC;
	vector< FontObject > vFontObjects;
	vector< unsigned > vFontObjectsGC;
	vector< vector< Object > > vvObjects;
	vector< unsigned > vvObjectsGC;
	vector< ObjHandle > vObjHandles;
	vector< unsigned > vObjHandlesGC;
	std::string ShotImagePath;

protected:
	D3DXMATRIX WorldMatrix;
	D3DXMATRIX ViewMatrix;
	D3DXMATRIX ProjectionMatrix;

	vLayer_t & GetLayers();
	LPDIRECT3DDEVICE9 & GetDevice();
	LPDIRECT3D9 & GetD3D();
	LPDIRECT3DVERTEXDECLARATION9 GetDefaultVDeclaration() const;
	LPDIRECT3DVERTEXSHADER9 GetDefaultVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefaultPShader() const;
	LPD3DXCONSTANTTABLE GetDefaultConstable() const;
	LPDIRECTSOUND8 GetDSound() const;

	//objects
	unsigned CreateObjHandle();
	unsigned CreateObject( ObjType type );
	void AddRefObjHandle( unsigned HandleIdx );
	void ReleaseObjHandle( unsigned HandleIdx );
	void ReleaseObject( unsigned HandleIdx );
	Object * GetObject( unsigned HandleIdx );

	//object factory
	ObjMgr * GetObjMgr( unsigned HandleIdx );

	//textures
	void LoadTexture( std::string const & pathname );
	LPDIRECT3DTEXTURE9 GetTexture( std::string const & pathname );
	void DeleteTexture( std::string const & pathname );

	//sounds
	void LoadSound( std::string const & pathname );
	void PlaySound( std::string const & pathname );
	void DeleteSound( std::string const & pathname );

	//text
	unsigned CreateFontObject();
	FontObject * GetFontObject( unsigned HandleIdx );

	//shot
	unsigned CreateShot( ULONG GraphicID );
	void PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color );
	void LoadShotImage( std::string const & pathname );
	void CreateShotData( unsigned ID, BlendType blend, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData );

	//ObjEffect Functions
	void ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount );
	void ObjEffect_SetVertexXY( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR2 Posxy );
	void ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv );
	void ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color );
	void ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState );
	void ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType );
	void ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer );

	//ObjShot Functions
	void ObjShot_SetGraphic( unsigned HandleIdx, ULONG ID );

	//ObjEffect Functions
	void ObjFont_SetSize( unsigned HandleIdx, ULONG Size );
	void ObjFont_SetFaceName( unsigned HandleIdx, std::string const & FaceName );

	//misc
	unsigned FetchVertexBuffer();
	vector< Vertex > & GetVertexBuffer( unsigned Idx );
	void AddRefVertexBuffer( unsigned Idx );
	void DisposeVertexBuffer( unsigned Idx );
	void DrawObjects();
	void UpdateObjectCollisions();

	//constructor
	Battery( HWND const hWnd );
	Battery();
};

class Direct3DEngine : protected virtual Battery
{
public:
	Direct3DEngine();
	void ToggleWindowed();
	void DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing );
	void DrawFPS();
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );

	void SetFog( D3DCOLOR Color, float Near, float Far );
};