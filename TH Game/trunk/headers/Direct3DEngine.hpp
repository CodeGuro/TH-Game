#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <Windows.h>
#include <DSound.h>
#include <ObjMgr.hpp>
#include <D3DSmartPtr.hpp>
#include <defstypedefs.hpp>

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
	vector< ObjMgr > vObjMgr;
};

struct VBuffer
{
	vector< Vertex > VertexBuffer;
	ULONG RefCount;
};

struct D3DVBuffer
{
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > Buffer;
	ULONG BufferSize;
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
friend class ObjMgr;
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
	vector< DelayData > Bullet_Delays;
	vector< vector< Object > > vvObjects;
	vector< unsigned > vvObjectsGC;
	vector< ObjHandle > vObjHandles;
	vector< unsigned > vObjHandlesGC;
	vector< VBuffer > vVertexBuffers;
	vector< unsigned > vVertexBuffersGC;
	std::string ShotImagePath;

protected:
	D3DXMATRIX WorldMatrix;
	D3DXMATRIX ViewMatrix;
	D3DXMATRIX ProjectionMatrix;

	vLayer_t & GetLayers();
	LPDIRECT3DDEVICE9 & GetDevice();
	LPDIRECT3D9 & GetD3D();
	D3DVBuffer & GetPipelineVBuffer();
	LPDIRECT3DVERTEXDECLARATION9 GetDefaultVDeclaration() const;
	LPDIRECT3DVERTEXSHADER9 GetDefaultVShader() const;
	LPDIRECT3DPIXELSHADER9 GetDefaultPShader() const;
	LPD3DXCONSTANTTABLE GetDefaultConstable() const;
	LPDIRECTSOUND8 GetDSound() const;

	//objects
	unsigned CreateObject( unsigned short Layer );
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

	//shot
	unsigned CreateShot01( D3DXVECTOR2 const & position, FLOAT const speed, FLOAT const direction, FLOAT const graphic );
	void PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color );
	void LoadShotImage( std::string const & pathname );
	void CreateShotData( unsigned ID, BlendType blend, unsigned delay, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData );
	void CreateDelayShotData( unsigned ID, RECT const & rect, D3DCOLOR const color, FLOAT const Scale, ULONG const DelayFrames );
	ShotData const & GetBulletTemplates( unsigned const graphic ) const;
	unsigned GetDelayDataSize() const;

	//ObjEffect Functions
	void ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount );
	void ObjEffect_SetVertexXY( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR2 Posxy );
	void ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv );
	void ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color );
	void ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState );
	void ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType );
	void ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer );
	void ObjEffect_SetScale( unsigned HandleIdx, D3DXVECTOR3 const & scale );

	//constructor
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