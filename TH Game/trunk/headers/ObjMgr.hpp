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

enum BlendType
{
	BlendMult, BlendSub, BlendAdd, BlendAlpha
};

struct Object
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 velocity; //this should perhaps also be un-normalized direction
	D3DXVECTOR3 accel;
	D3DXVECTOR3 scale;
	D3DXQUATERNION direction; //this should perhaps not exist
	D3DXQUATERNION orient;
	D3DXQUATERNION orientvel;
	ULONG VertexOffset;
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

	float GetSpeed() const;
	float GetAngle() const;


	//flags, -1 to get, 1 for on, 0 for off
	bool FlagMotion( int flag );
	bool FlagCollidable( int flag );
	bool FlagScreenDeletable( int flag );
	bool FlagGraze( int flag );
	bool FlagPixelPerfect( int flag );
	void Advance();
};

struct ShotData
{
	ULONG VtxOffset;
	ULONG Delay;
	ULONG Radius;
	ULONG AnimationTime;
	BlendType Render;
	DWORD Flags;
	ULONG NextShot;
};

struct DelayData
{
	ULONG VtxOffset;
	ULONG DelayFrames;
	FLOAT Scale;
};

class ObjMgr
{
private:
	unsigned VertexCount;
	unsigned VertexBufferIdx;
	unsigned ObjBufferIdx;
	D3DPRIMITIVETYPE PrimitiveType;
	BlendType BlendState;
	D3DSmartPtr< LPDIRECT3DTEXTURE9 > pTexture;
	D3DSmartPtr< LPDIRECT3DVERTEXDECLARATION9 > VDeclaration;
	D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > VShader;
	D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > PShader;
	D3DSmartPtr< LPD3DXCONSTANTTABLE > Constable;

public:
	ObjMgr();
	void SetVertexCount( unsigned const Count );
	void SetVertexBufferIdx( unsigned const Idx );
	void SetObjBufferIdx( unsigned const Idx );
	void SetTexture( LPDIRECT3DTEXTURE9 pTex );
	void SetVertexDeclaration( LPDIRECT3DVERTEXDECLARATION9 VDecl );
	void SetVertexShader( LPDIRECT3DVERTEXSHADER9 Shader );
	void SetPixelShader( LPDIRECT3DPIXELSHADER9 Shader );
	void SetVShaderConstTable( LPD3DXCONSTANTTABLE Table );
	void SetPrimitiveType( D3DPRIMITIVETYPE PrimType );
	void SetBlendState( BlendType const Blend);
	void AdvanceDrawDanmaku( Direct3DEngine * D3DEng );
};