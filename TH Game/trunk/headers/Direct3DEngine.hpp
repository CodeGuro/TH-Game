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
	void SetAlignmentX( int X );
	void SetAlignmentY( int Y );
};

struct WaveHeaderType
{
		char chunkId[ 4 ];
		unsigned long chunkSize;
		char format[ 4 ];
		char subChunkId[ 4 ];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[ 4 ];
		unsigned long dataSize;
};

class Direct3DEngine
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
	D3DSmartPtr< LPD3DXCONSTANTTABLE > pDefault3DConstable;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > pDefault2DVShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > pDefault2DPShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > pDefault2DConstable;
	D3DSmartPtr< LPDIRECTSOUND8 > dsound;
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > pVBGridTerrain;
	
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

	void GenerateGridTerrain( unsigned Rows, unsigned Columns, float Spacing );

public:
	D3DXMATRIX WorldMatrix;
	D3DXMATRIX ViewMatrix;
	D3DXMATRIX ProjectionMatrix;
	D3DCOLOR ClearColor;

	vLayer_t & GetLayers();
	LPDIRECT3DDEVICE9 & GetDevice();
	LPDIRECT3D9 & GetD3D();
	LPDIRECT3DVERTEXDECLARATION9 GetDefaultVDeclaration() const;
	LPDIRECT3DVERTEXSHADER9 GetDefault3DVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefault3DPShader() const;
	LPD3DXCONSTANTTABLE GetDefault3DConstable() const;
	LPDIRECT3DVERTEXSHADER9 GetDefault2DVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefault2DPShader() const;
	LPD3DXCONSTANTTABLE GetDefault2DConstable() const;
	LPDIRECTSOUND8 GetDSound() const;

	//objects
	unsigned CreateObjHandle();
	unsigned CreateObject( ObjType type );
	void AddRefObjHandle( unsigned HandleIdx );
	void ReleaseObject( unsigned HandleIdx ); // decrements handle refcount
	void DestroyObject( unsigned HandleIdx ); // destroy only
	Object * GetObject( unsigned HandleIdx );

	//object factory
	ObjMgr * GetObjMgr( unsigned HandleIdx );

	//textures
	void LoadTexture( std::string const & pathname );
	LPDIRECT3DTEXTURE9 GetTexture( std::string const & pathname );
	void DeleteTexture( std::string const & pathname );

	//sounds
	void LoadSound( std::string const & pathname );
	void PlaySound( std::string const & pathname, const float volume );
	void StopSound( std::string const & pathname );
	void DeleteSound( std::string const & pathname );

	//text
	unsigned CreateFontObject();
	FontObject * GetFontObject( unsigned HandleIdx );

	//shot
	unsigned CreateShot( ULONG GraphicID );
	void PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color );
	bool LoadShotImage( std::string const & pathname );
	void CreateShotData( unsigned ID, BlendType blend, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData );

	//ObjEffect Functions
	void ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount );
	unsigned ObjEffect_GetVertexCount( unsigned HandleIdx );
	void ObjEffect_SetVertexXYZ( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR3 Posxyz );
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
	void DrawObjects_V2();
	void DrawObjects( ObjMgr const & objMgr );
	void AdvanceObjects( ObjMgr const & objMgr );
	void UpdateObjectCollisions();

	//camera
	void SetLookAtViewMatrix( D3DXVECTOR3 const & eye, D3DXVECTOR3 const & at );
	void SetFog( float fognear, float fogfar, float fred, float fgreen, float fblue );
	void SetPerspectiveClip( float zNear, float zFar )
	{
		D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, (float)D3DXToRadian( 45.f ), 640.f/480.f, zNear, zFar );
	}

	//constructor
	Direct3DEngine( HWND const hWnd );
	void ToggleWindowed();
	void DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing );
	void DrawFPS();
	void RenderFrame( MSG const Msg );
	void ProcUserInput( MSG const Msg );
};