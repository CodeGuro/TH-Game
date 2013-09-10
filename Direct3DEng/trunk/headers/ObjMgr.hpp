#pragma once
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>

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

enum BlendType
{
	BlendAlpha,	BlendAdd, BlendSub,
	BlendInvAlph, BlendInvAdd
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
	LPDIRECT3DTEXTURE9 pTexture;
	LPDIRECT3DVERTEXBUFFER9 VertexBuffer;
	LPDIRECT3DVERTEXDECLARATION9 VDeclaration;
	LPDIRECT3DVERTEXSHADER9 VShader;
	LPDIRECT3DPIXELSHADER9 PShader;
	LPD3DXCONSTANTTABLE Constable;
	std::vector< Vertex > vecVertexLibrary;
	std::vector< Vertex > vecVertices;
	std::vector< Object > vecObjects;
	std::vector< ObjHandle > vecIntermediateLayer;
	std::vector< unsigned > vecIntermediateLayerGC;

public:
	ObjMgr();
	ObjMgr( ObjMgr const & source );
	ObjMgr & operator = ( ObjMgr const & source );
	~ObjMgr();
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
	void EraseObj( unsigned const Index );
	Object & GetObjRef( unsigned const Index );
	Object * GetObjPtr( unsigned const Index );
	D3DSURFACE_DESC GetSurfaceDesc();
	void AdvanceTransformedDraw( Direct3DEngine * D3DEng );
	unsigned GetObjCount();
};
