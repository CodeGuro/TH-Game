#include <ObjMgr.hpp>
#include <Direct3DEngine.hpp>

ObjMgr::ObjMgr() : BlendOp( BlendAlpha ), VertexCount( 0 ), VBufferLength( 0 ), PrimitiveType( D3DPT_TRIANGLELIST ), pTexture( NULL ), VertexBuffer( NULL ), VDeclaration( NULL ), VShader( NULL), PShader( NULL ), Constable( NULL )
{
	memset( &SurfaceDesc, -1, sizeof( SurfaceDesc ) );
}
ObjMgr::ObjMgr( ObjMgr const & source ) : BlendOp( source.BlendOp ), PrimitiveType( source.PrimitiveType ), VertexCount( source.VertexCount ), VBufferLength( source.VBufferLength ), SurfaceDesc( source.SurfaceDesc ),
	pTexture( source.pTexture ), VertexBuffer( source.VertexBuffer ), VDeclaration( source.VDeclaration ), VShader( source.VShader ), PShader( source.PShader ), Constable( source.Constable ), 
	vecVertexLibrary( source.vecVertexLibrary ), vecObjects( source.vecObjects ),
	vecIntermediateLayer( source.vecIntermediateLayer ), vecIntermediateLayerGC( source.vecIntermediateLayerGC )
{
	if( source.pTexture ) source.pTexture->AddRef();
	if( source.VertexBuffer ) source.VertexBuffer->AddRef() ;
	if( source.VDeclaration ) source.VDeclaration->AddRef();
	if( source.VShader ) source.VShader->AddRef();
	if( source.PShader ) source.PShader->AddRef();
	if( source.Constable ) source.Constable->AddRef();
}
ObjMgr & ObjMgr::operator = ( ObjMgr const & source )
{
	if( pTexture ) pTexture->Release();
	if( VertexBuffer ) VertexBuffer->Release();
	if( VDeclaration ) VDeclaration->Release();
	if( VShader ) VShader->Release();
	if( PShader ) PShader->Release();
	if( Constable ) Constable->Release();

	source.pTexture->AddRef();
	source.VertexBuffer->AddRef();
	source.VDeclaration->AddRef();
	source.VShader->AddRef();
	source.PShader->AddRef();
	source.Constable->AddRef();

	pTexture = source.pTexture;
	PrimitiveType = source.PrimitiveType;
	VertexBuffer = source.VertexBuffer;
	VDeclaration = source.VDeclaration;
	VShader = source.VShader;
	PShader = source.PShader;
	Constable = source.Constable;
	BlendOp = BlendAdd;
	SurfaceDesc = source.SurfaceDesc;
	VertexCount = source.VertexCount;
	VBufferLength = source.VBufferLength;
	vecVertices = source.vecVertices;
	vecObjects = source.vecObjects;
	vecIntermediateLayer = source.vecIntermediateLayer;
	vecIntermediateLayerGC = source.vecIntermediateLayerGC;
	return *this;
}
ObjMgr::~ObjMgr()
{
	if( pTexture ) pTexture->Release();
	if( VertexBuffer ) VertexBuffer->Release();
	if( VDeclaration ) VDeclaration->Release();
	if( VShader ) VShader->Release();
	if( PShader ) PShader->Release();
	if( Constable ) Constable->Release();
}
void ObjMgr::SetVertexCount( unsigned const Count )
{
	VertexCount = Count;
	vecVertices.resize( vecObjects.size() * VertexCount );
}
void ObjMgr::SetTexture( LPDIRECT3DTEXTURE9 pTex )
{
	if( this->pTexture ) pTexture->Release();
	if( pTex ) pTex->AddRef();
	if( pTex ) pTex->GetLevelDesc(0, &SurfaceDesc );
	pTexture = pTex;
};
void ObjMgr::SetVertexDeclaration( LPDIRECT3DVERTEXDECLARATION9 VDecl )
{
	if( VDeclaration ) VDeclaration->Release();
	if( VDecl ) VDecl->AddRef();
	VDeclaration = VDecl;
}
void ObjMgr::SetVertexShader( LPDIRECT3DVERTEXSHADER9 Shader )
{
	if( VShader ) VShader->Release();
	if( Shader ) Shader->AddRef();
	VShader = Shader;
}
void ObjMgr::SetPixelShader( LPDIRECT3DPIXELSHADER9 Shader )
{
	if( PShader ) PShader->Release();
	if( Shader ) Shader->AddRef();
	PShader = Shader;
}
void ObjMgr::SetVShaderConstTable( LPD3DXCONSTANTTABLE Table )
{
	if( Constable ) Constable->Release();
	if( Table ) Table->AddRef();
	Constable = Table;
}
void ObjMgr::SetPrimitiveType( D3DPRIMITIVETYPE PrimType )
{
	PrimitiveType = PrimType;
}
void ObjMgr::SetBlendMode( BlendType Blend )
{
	BlendOp = Blend;
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
	vecVertices.resize( vecVertices.size() + VertexCount );
	auto & back = vecObjects.back();
	back.libidx = Index;
	back.SetPosition( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetVelocity( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetAccel( D3DXVECTOR3( 0, 0, 0 ) );
	back.SetScale( D3DXVECTOR3( 1, 1, 1 ) );
	back.SetAngle( 0.f );
	back.SetRotation( 0.f );
	back.SetRotationVelocity( 0.f );
	return result;
}
void ObjMgr::EraseObj( unsigned const Index )
{
	unsigned ObjIdx = vecIntermediateLayer[ Index ].ObjIdx;
	vecIntermediateLayerGC.push_back( Index );
	for( unsigned i = 0; i < vecIntermediateLayer.size(); ++i )
	{
		if( vecIntermediateLayer[ i ].ObjIdx > ObjIdx )
			--(vecIntermediateLayer[ i ].ObjIdx);
	}
	vecVertices.erase( vecVertices.begin() + VertexCount * ObjIdx, vecVertices.begin() + VertexCount * ( 1 + ObjIdx ) );
	vecObjects.erase( vecObjects.begin() + ObjIdx );
}
Object & ObjMgr::GetObjRef( unsigned const Index )
{
	return vecObjects[ vecIntermediateLayer[ Index ].ObjIdx ];
}
Object * ObjMgr::GetObjPtr( unsigned const Index )
{
	return &(vecObjects[ vecIntermediateLayer[ Index ].ObjIdx ]);
}
D3DSURFACE_DESC ObjMgr::GetSurfaceDesc()
{
	return SurfaceDesc;
}
void ObjMgr::AdvanceTransformedDraw( Direct3DEngine * D3DEng )
{
	unsigned s = vecObjects.size();
	vecVertices.resize( vecObjects.size() * VertexCount );
	D3DXMATRIX mat;
	if( vecVertexLibrary.size() )
	{
		for( unsigned u = 0; u < s; ++u )
		{
			Vertex * dst = &vecVertices[ u * VertexCount ];
			Vertex * src = &vecVertexLibrary[ vecObjects[ u ].libidx ];
			D3DXMatrixTransformation( &mat, NULL, NULL, &vecObjects[ u ].scale, NULL, &( vecObjects[ u ].direction * vecObjects[ u ].orient ), &vecObjects[ u ].position );
			for( unsigned v = 0; v < VertexCount; ++v )
			{
				D3DXVec3TransformCoord( &dst->pos, &src->pos, &mat );
				dst->tex = src->tex;
				dst->color = src->color;
				++dst;
				++src;
			};
			vecObjects[ u ].Advance();
		}
		if( VBufferLength < vecVertices.size() * sizeof( Vertex ) )
		{
			VBufferLength = vecVertices.size() * sizeof( Vertex );
			if( VertexBuffer ) VertexBuffer->Release();
			D3DEng->GetDevice()->CreateVertexBuffer( vecVertices.size() * sizeof( Vertex ), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &VertexBuffer, NULL );
		}
		void * ptr;
		VertexBuffer->Lock( 0, 0, &ptr, NULL );
		memcpy( ptr, &vecVertices[ 0 ], sizeof( Vertex ) * vecVertices.size() );
		VertexBuffer->Unlock();
		LPDIRECT3DDEVICE9 d3ddev = D3DEng->GetDevice();
		d3ddev->SetTexture( 0, pTexture );
		d3ddev->SetVertexDeclaration( VDeclaration );
		d3ddev->SetVertexShader( VShader );
		d3ddev->SetPixelShader( PShader );
		switch( BlendOp )
		{
		case BlendAlpha:
			d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case BlendAdd:
			d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTALPHA );
			d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case BlendSub:
			d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT );
			break;
		case BlendInvAlph:
			d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA );
			d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
			d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case BlendInvAdd:
			d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA );
			d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTALPHA );
			d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		}
		d3ddev->SetStreamSource( 0, VertexBuffer, 0, sizeof( Vertex ) );
		d3ddev->DrawPrimitive( PrimitiveType, 0, PrimitiveType == D3DPT_TRIANGLELIST ? vecObjects.size() * VertexCount / 3 :
			PrimitiveType == D3DPT_TRIANGLESTRIP ? (VertexCount - 2) * vecObjects.size() : 0 );
	}
}
unsigned ObjMgr::GetObjCount()
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
	position += velocity += accel;
	orient *= orientvel;
}