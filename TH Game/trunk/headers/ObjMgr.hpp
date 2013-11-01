#pragma once
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>
#include <D3DSmartPtr.hpp>
#include <cassert>

class Direct3DEngine;

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
	union
	{
		ULONG ShotData;
		ULONG libidx;
	};
	DWORD flags;

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

	//flags, -1 to get, 1 for on, 0 for off
	bool FlagMotion( int flag );
	bool FlagCollidable( int flag );
	bool FlagScreenDeletable( int flag );
	bool FlagGraze( int flag );
	bool FlagPixelPerfect( int flag );
	void Advance();
};

enum BlendType
{
	BlendAlpha,	BlendAdd, BlendSub,
	BlendInvAlph, BlendInvAdd
};

struct ShotData
{
	ULONG VtxOffset;
	ULONG Delay;
	ULONG Radius;
	ULONG AnimationTime;
	ULONG NextShot;
};

struct DelayData
{
	ULONG VtxOffset;
	ULONG DelayFrames;
	FLOAT Scale;
};

struct Shot
{
	D3DXVECTOR2 PosXY;
	FLOAT Direction;
	FLOAT Speed;
	FLOAT Acceleration;
	FLOAT Rotation;
	FLOAT Rotational_Velocity;
	ULONG ShotDataID;
	ULONG Flags;
	ULONG Time;
};

class ObjMgr
{
private:
	struct ObjHandle
	{
		unsigned ObjIdx;
	};

	BlendType BlendOp;
	D3DSURFACE_DESC SurfaceDesc;

	unsigned VertexCount;
	unsigned VBufferLength;
	D3DPRIMITIVETYPE PrimitiveType;
	D3DSmartPtr< LPDIRECT3DTEXTURE9 > pTexture;
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > VertexBuffer;
	D3DSmartPtr< LPDIRECT3DVERTEXDECLARATION9 > VDeclaration;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > VShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > PShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > Constable;
	std::vector< Vertex > vecVertexLibrary;
	std::vector< ShotData > Bullet_Templates;
	std::vector< DelayData > Bullet_Delays;
	std::vector< Object > vecObjects;
	std::vector< ObjHandle > vecIntermediateLayer;
	std::vector< unsigned > vecIntermediateLayerGC;

public:
	ObjMgr();
	void SetVertexCount( unsigned const Count );
	void SetTexture( LPDIRECT3DTEXTURE9 pTex );
	void SetVertexDeclaration( LPDIRECT3DVERTEXDECLARATION9 VDecl );
	void SetVertexShader( LPDIRECT3DVERTEXSHADER9 Shader );
	void SetPixelShader( LPDIRECT3DPIXELSHADER9 Shader );
	void SetVShaderConstTable( LPD3DXCONSTANTTABLE Table );
	void SetPrimitiveType( D3DPRIMITIVETYPE PrimType );
	void SetBlendMode( BlendType Blend );
	void PushQuadLib( RECT Quad, D3DCOLOR Color );
	void PushVertexLib( std::vector< Vertex > const & VecVerts );
	void ResizeVertexLib( unsigned VCount );
	unsigned GetVertexCountLib();
	unsigned PushObj( unsigned const Index ); //returns an index to the handle
	unsigned PushEmptyObj(); //returns an index to the handle
	void EraseObj( unsigned const Index );
	Object & GetObjRef( unsigned const Index );
	Object * GetObjPtr( unsigned const Index );
	Vertex * GetLibVertexPtr( unsigned const Index );
	D3DSURFACE_DESC GetSurfaceDesc();
	void AdvanceTransformedDraw( Direct3DEngine * D3DEng, bool Danmaku );
	unsigned GetObjCount() const;
	unsigned GetDelayDataSize() const;
	void CreateShotData( unsigned ID, BlendType blend, unsigned delay, RECT const & rect, D3DCOLOR color, std::vector< std::vector< float > > const & AnimationData );
	void CreateDelayShotData( unsigned ID, RECT const & rect, D3DCOLOR const color, FLOAT const Scale, ULONG const DelayFrames );
};

