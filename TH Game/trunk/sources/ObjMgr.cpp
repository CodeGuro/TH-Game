#include <ObjMgr.hpp>
#include <Direct3DEngine.hpp>

ObjMgr::ObjMgr() : VertexCount( 0 ), PrimitiveType( D3DPT_TRIANGLELIST )
{
}
void ObjMgr::SetVertexCount( unsigned const Count )
{
	VertexCount = Count;
}
void ObjMgr::SetTexture( LPDIRECT3DTEXTURE9 pTex )
{
	pTexture = pTex;
};
void ObjMgr::SetVertexDeclaration( LPDIRECT3DVERTEXDECLARATION9 VDecl )
{
	VDeclaration = VDecl;
}
void ObjMgr::SetVertexShader( LPDIRECT3DVERTEXSHADER9 Shader )
{
	VShader = Shader;
}
void ObjMgr::SetPixelShader( LPDIRECT3DPIXELSHADER9 Shader )
{
	PShader = Shader;
}
void ObjMgr::SetVShaderConstTable( LPD3DXCONSTANTTABLE Table )
{
	Constable = Table;
}
void ObjMgr::SetPrimitiveType( D3DPRIMITIVETYPE PrimType )
{
	PrimitiveType = PrimType;
}
void ObjMgr::SetVertexBufferIdx( unsigned const Idx )
{
	VertexBufferIdx = Idx;
}
void ObjMgr::SetObjBufferIdx( unsigned const Idx )
{
	ObjBufferIdx = Idx;
}
void ObjMgr::SetBlendState( BlendType const Blend )
{
	BlendState = Blend;
}
void ObjMgr::AdvanceDrawDanmaku( Direct3DEngine * D3DEng )
{
	LPDIRECT3DDEVICE9 d3ddev = D3DEng->GetDevice();
	D3DXMATRIX mat;
	D3DVBuffer & VertexBuffer = D3DEng->GetPipelineVBuffer();
	auto & vObjects = D3DEng->vvObjects[ ObjBufferIdx ];
	ULONG min_buffersize = vObjects.size() * VertexCount * sizeof( Vertex );
	if( VertexBuffer.BufferSize < min_buffersize )
	{
		VertexBuffer.Buffer->Release();
		d3ddev->CreateVertexBuffer( VertexBuffer.BufferSize = min_buffersize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &VertexBuffer.Buffer, NULL );
	}
	Vertex * pstart;
	Vertex * ptr;
	VertexBuffer.Buffer->Lock( 0, min_buffersize, (void**)&pstart, 0 );
	ptr = pstart;

	for( unsigned u = 0; u < vObjects.size(); ++u )
	{
		Vertex * src = &D3DEng->vVertexBuffers[ VertexBufferIdx ].VertexBuffer[ vObjects[ u ].VertexOffset ];
		D3DXMatrixTransformation( &mat, NULL, NULL, &vObjects[ u ].scale, NULL, &( vObjects[ u ].direction * vObjects[ u ].orient ), vObjects[ u ].FlagPixelPerfect( -1 ) ? &D3DXVECTOR3( floor( vObjects[ u ].position.x + 0.5f), floor(vObjects[ u ].position.y + 0.5f), floor( vObjects[ u ].position.z + 0.5f ) ) : &vObjects[ u ].position );

		for( unsigned v = 0; v < VertexCount; ++v )
		{
			D3DXVec3TransformCoord( &ptr->pos, &src->pos, &mat );
			ptr->tex = src->tex;
			ptr->color = src->color;
			++ptr;
			++src;
		}
		vObjects[ u ].Advance();
	}

	VertexBuffer.Buffer->Unlock();

	DWORD ZSet;
	d3ddev->SetTexture( 0, pTexture );
	d3ddev->SetVertexDeclaration( VDeclaration );
	d3ddev->SetVertexShader( VShader );
	d3ddev->SetPixelShader( PShader );

	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->GetRenderState( D3DRS_ZENABLE, &ZSet );
	d3ddev->SetRenderState( D3DRS_ZENABLE, FALSE );
	
	switch( BlendState )
	{
	case BlendMult:
		d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
		d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
		d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	case BlendAdd:
		d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	case BlendSub:
		d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT ); 
		break;
	
	case BlendAlpha:
		d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	default:
		abort();
	}
	d3ddev->SetStreamSource( 0, VertexBuffer.Buffer, 0, sizeof( Vertex ) );
	ULONG VCount = ptr - pstart;
	d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? VCount / 3 :
		PrimitiveType == D3DPT_TRIANGLESTRIP ? VCount - 2 : 0 );
	d3ddev->SetRenderState( D3DRS_ZENABLE, ZSet );
}

void Object::SetSpeed( float Speed )
{
	FLOAT placeholder;
	D3DXQuaternionToAxisAngle( &direction, &velocity, &placeholder );
	D3DXVec3Scale( &velocity, &velocity, Speed / D3DXVec3Length( &velocity ) );
}
void Object::SetVelocity( D3DXVECTOR3 Velocity )
{
	velocity = Velocity;
}
void Object::SetAccel( D3DXVECTOR3 Accel )
{
	accel = Accel;
}
void Object::SetPosition( D3DXVECTOR3 Position )
{
	position = Position;
}
void Object::SetScale( D3DXVECTOR3 Scaling )
{
	scale = Scaling;
}
void Object::SetAngle( float Theta )
{
	D3DXQuaternionRotationAxis( &direction, &D3DXVECTOR3( 0, 0, 1 ), Theta );
}
void Object::SetAngleEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &direction, &Axis, Theta );
}
void Object::SetRotation( float Theta )
{
	D3DXQuaternionRotationAxis( &orient, &D3DXVECTOR3( 0, 0, 1 ), Theta );
}
void Object::SetRotationEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &orient, &Axis, Theta );
}
void Object::SetRotationVelocity( float Theta )
{
	D3DXQuaternionRotationAxis( &orientvel, &D3DXVECTOR3( 0, 0, 1 ), Theta );
}
void Object::SetRotationVelocityEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &orientvel, &Axis, Theta );
}
void Object::Advance()
{
	D3DXQUATERNION inv;
	D3DXQuaternionInverse( &inv, &direction );
	if( FlagMotion( -1 ) )
		position += velocity += accel;
	orient = FlagPixelPerfect( - 1 )? D3DXQUATERNION( 0, 0, 0, 1 ) * inv : orient * orientvel;
}
bool Object::FlagMotion( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x1) != 0;
	case 0:
		flags = flags & ~0x1;
		return false;
	case 1:
	default:
		flags = flags | 0x1;
		return true;
	}
}
bool Object::FlagCollidable( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x2) != 0;
	case 0:
		flags = flags & ~0x2;
		return false;
	case 1:
	default:
		flags = flags | 0x2;
		return true;
	}
}
bool Object::FlagScreenDeletable( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x4) != 0;
	case 0:
		flags = flags & ~0x4;
		return false;
	case 1:
	default:
		flags = flags | 0x4;
		return true;
	}
}
bool Object::FlagGraze( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x8) != 0;
	case 0:
		flags = flags & ~0x8;
		return false;
	case 1:
	default:
		flags = flags | 0x8;
		return true;
	}
}
bool Object::FlagPixelPerfect( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x10) != 0;
	case 0:
		flags = flags & ~0x10;
		return false;
	case 1:
	default:
		flags = flags | 0x10;
		return true;
	}
}