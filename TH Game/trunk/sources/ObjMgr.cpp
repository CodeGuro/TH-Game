#include <ObjMgr.hpp>
#include <Direct3DEngine.hpp>

ObjMgr::ObjMgr() : VertexCount( 0 ), PrimitiveType( D3DPT_TRIANGLELIST )
{
	memset( &SurfaceDesc, -1, sizeof( SurfaceDesc ) );
}
void ObjMgr::SetVertexCount( unsigned const Count )
{
	VertexCount = Count;
}
void ObjMgr::SetTexture( LPDIRECT3DTEXTURE9 pTex )
{
	pTexture = pTex;
	pTex->GetLevelDesc( 0, &SurfaceDesc );
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
void ObjMgr::PushQuadLib( RECT Quad, D3DCOLOR Color )
{
	if( VertexCount == 6 )
	{
		Vertex v[6] = 
		{
			{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
			{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
			{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
			{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
			{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
			{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color }
		};
		for( unsigned u = 0; u < 6; ++u ) vecVertexLibrary.push_back( v[u] );
	}
}
void ObjMgr::PushVertexLib( std::vector< Vertex > const & VecVerts )
{
	if( VecVerts.size() == VertexCount )
	{
		for( unsigned u = 0; u < VecVerts.size(); ++u )
			vecVertexLibrary.push_back( VecVerts[ u ] );
	}
}
void ObjMgr::ResizeVertexLib( unsigned VCount )
{
	VertexCount = VCount;
	vecVertexLibrary.resize( VCount );
}
unsigned ObjMgr::GetVertexCountLib()
{
	return vecVertexLibrary.size();
}
unsigned ObjMgr::PushObj( unsigned const Index )
{
	unsigned result;
	if( vecIntermediateLayerGC.size() )
	{
		result = vecIntermediateLayerGC.back();
		vecIntermediateLayerGC.pop_back();
	}
	else
		vecIntermediateLayer.resize( 1 + ( result = vecIntermediateLayer.size() ) );
	vecIntermediateLayer[ result ].ObjIdx = vecObjects.size();
	vecObjects.resize( 1 + vecObjects.size() );
	auto & back = vecObjects.back();
	back.libidx = Index;
	back.SetPosition( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetVelocity( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetAccel( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetScale( D3DXVECTOR3( 1, 1, 1 ) );
	back.SetAngle( 0.f );
	back.SetRotation( 0.f );
	back.SetRotationVelocity( 0.f );
	back.flags = 0;
	back.FlagMotion( 1 );
	back.FlagCollidable( 0 );
	back.FlagScreenDeletable( 1 );
	return result;
}
unsigned ObjMgr::PushEmptyObj()
{
	unsigned result;
	if( vecIntermediateLayerGC.size() )
	{
		result = vecIntermediateLayerGC.back();
		vecIntermediateLayerGC.pop_back();
	}
	else
		vecIntermediateLayer.resize( 1 + ( result = vecIntermediateLayer.size() ) );
	vecIntermediateLayer[ result ].ObjIdx = vecObjects.size();
	Object obj = { D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 1, 1, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), 0, BlendAlpha, 0 };
	vecObjects.push_back( obj );
	return result;
}
void ObjMgr::EraseObj( unsigned const Index )
{
	if( Index != -1 )
	{
		unsigned ObjIdx = vecIntermediateLayer[ Index ].ObjIdx;
		vecIntermediateLayerGC.push_back( Index );
		for( unsigned i = 0; i < vecIntermediateLayer.size(); ++i )
		{
			if( vecIntermediateLayer[ i ].ObjIdx > ObjIdx )
				--(vecIntermediateLayer[ i ].ObjIdx);
		}
		vecObjects.erase( vecObjects.begin() + ObjIdx );
	}
}
Object * ObjMgr::GetObjPtr( unsigned const Index )
{
	return &(vecObjects[ vecIntermediateLayer[ Index ].ObjIdx ]);
}
Vertex * ObjMgr::GetLibVertexPtr( unsigned const Index )
{
	return &vecVertexLibrary[ vecIntermediateLayer[ Index ].ObjIdx * VertexCount ];
}
D3DSURFACE_DESC ObjMgr::GetSurfaceDesc()
{
	return SurfaceDesc;
}
void ObjMgr::AdvanceDrawDanmaku( Direct3DEngine * D3DEng )
{
	LPDIRECT3DDEVICE9 d3ddev = D3DEng->GetDevice();
	D3DXMATRIX mat;

	for( unsigned u = 0; u < sizeof( VertexBuffers ) / sizeof( VBuffer ); ++u )
	{
		if( !VertexBuffers[ u ].VertexBuffer )
			d3ddev->CreateVertexBuffer( VertexBuffers[ u ].VBufferLength = sizeof( Vertex ) * 600, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &VertexBuffers[ u ].VertexBuffer, NULL );
		VertexBuffers[ u ].VertexBuffer->Lock( 0, 0, (void**)&VertexBuffers[ u ].ptrinit, 0 );
		VertexBuffers[ u ].ptr = VertexBuffers[ u ].ptrinit;
	}

	for( unsigned u = 0; u < vecObjects.size(); ++u )
	{
		VBuffer * buffer;
		switch( vecObjects[ u ].render )
		{
		case BlendAlpha: buffer = &VertexBuffers[ 0 ]; break;
		case BlendAdd: buffer = &VertexBuffers[ 1 ]; break;
		case BlendSub: buffer = &VertexBuffers[ 2 ]; break;
		case BlendMult: buffer = &VertexBuffers[ 3 ]; break;
		default: abort();
		}

		while( buffer->VBufferLength < vecObjects.size() * VertexCount * sizeof( Vertex ) )
		{
			ULONG newsize = 10 + 10 * buffer->VBufferLength;
			ULONG displacement = buffer->ptr - buffer->ptrinit;
			D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > newbuff;
			d3ddev->CreateVertexBuffer( newsize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &newbuff, NULL );
			newbuff->Lock( 0, 0, (void**)&buffer->ptrinit, 0 );
			buffer->ptr = buffer->ptrinit;
			for( unsigned i = 0; i < displacement; ++i )
				*buffer->ptr++ = buffer->ptrinit[ i ];
			buffer->VertexBuffer->Unlock();
			buffer->VertexBuffer = newbuff;
			buffer->VBufferLength = newsize;
		}
		Vertex * src = &vecVertexLibrary[ D3DEng->GetBulletTemplates( vecObjects[ u ].ShotData ).VtxOffset ];
		D3DXMatrixTransformation( &mat, NULL, NULL, &vecObjects[ u ].scale, NULL, &( vecObjects[ u ].direction * vecObjects[ u ].orient ), vecObjects[ u ].FlagPixelPerfect( -1 ) ? &D3DXVECTOR3( floor( vecObjects[ u ].position.x + 0.5f), floor(vecObjects[ u ].position.y + 0.5f), floor( vecObjects[ u ].position.z + 0.5f ) ) : &vecObjects[ u ].position );

		for( unsigned v = 0; v < VertexCount; ++v )
		{
			D3DXVec3TransformCoord( &buffer->ptr->pos, &src->pos, &mat );
			buffer->ptr->tex = src->tex;
			buffer->ptr->color = src->color;
			++buffer->ptr;
			++src;
		}
		vecObjects[ u ].Advance();
	}

	for( unsigned u = 0; u < sizeof( VertexBuffers ) / sizeof( VBuffer ); ++u )
		VertexBuffers[ u ].VertexBuffer->Unlock();

	DWORD ZSet;
	d3ddev->SetTexture( 0, pTexture );
	d3ddev->SetVertexDeclaration( VDeclaration );
	d3ddev->SetVertexShader( VShader );
	d3ddev->SetPixelShader( PShader );
	d3ddev->GetRenderState( D3DRS_ZENABLE, &ZSet );
	d3ddev->SetRenderState( D3DRS_ZENABLE, FALSE );

	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	d3ddev->SetStreamSource( 0, VertexBuffers[ 3 ].VertexBuffer, 0, sizeof( Vertex ) );
	d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? (VertexBuffers[ 3 ].ptr - VertexBuffers[ 3 ].ptrinit) / 3 :
		PrimitiveType == D3DPT_TRIANGLESTRIP ? (VertexBuffers[ 3 ].ptr - VertexBuffers[ 3 ].ptrinit) - 2 : 0 );

	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	d3ddev->SetStreamSource( 0, VertexBuffers[ 1 ].VertexBuffer, 0, sizeof( Vertex ) );
	d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? (VertexBuffers[ 1 ].ptr - VertexBuffers[ 1 ].ptrinit) / 3 :
		PrimitiveType == D3DPT_TRIANGLESTRIP ? (VertexBuffers[ 1 ].ptr - VertexBuffers[ 1 ].ptrinit) - 2 : 0 );

	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT ); //src = ( 0.5, 0.5, 0.5, 0 ), dst = ( 0.5, 0.5, 0.5, 1 ) // dst + src = ( .25, .25, .25 ) + ( 
	d3ddev->SetStreamSource( 0, VertexBuffers[ 2 ].VertexBuffer, 0, sizeof( Vertex ) );
	d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? (VertexBuffers[ 2 ].ptr - VertexBuffers[ 2 ].ptrinit) / 3 :
		PrimitiveType == D3DPT_TRIANGLESTRIP ? (VertexBuffers[ 2 ].ptr - VertexBuffers[ 2 ].ptrinit) - 2 : 0 );
	

	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	d3ddev->SetStreamSource( 0, VertexBuffers[ 0 ].VertexBuffer, 0, sizeof( Vertex ) );
	d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? (VertexBuffers[ 0 ].ptr - VertexBuffers[ 0 ].ptrinit) / 3 :
		PrimitiveType == D3DPT_TRIANGLESTRIP ? (VertexBuffers[ 0 ].ptr - VertexBuffers[ 0 ].ptrinit) - 2 : 0 );

	d3ddev->SetRenderState( D3DRS_ZENABLE, ZSet );
}
unsigned ObjMgr::GetObjCount() const
{
	return vecObjects.size();
}

void Object::SetSpeed( float Speed )
{
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
		return (flags & 0x16) != 0;
	case 0:
		flags = flags & ~0x16;
		return false;
	case 1:
	default:
		flags = flags | 0x16;
		return true;
	}
}