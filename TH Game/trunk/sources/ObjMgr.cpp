#include <ObjMgr.hpp>
#include <Direct3DEngine.hpp>

ObjMgr::ObjMgr() : BlendOp( BlendAlpha ), VertexCount( 0 ), VBufferLength( 0 ), PrimitiveType( D3DPT_TRIANGLELIST )
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
	Object obj = { D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 1, 1, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), D3DXQUATERNION( 0, 0, 0, 1 ), 0, 0 };
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
Object & ObjMgr::GetObjRef( unsigned const Index )
{
	return vecObjects[ vecIntermediateLayer[ Index ].ObjIdx ];
}
Object * ObjMgr::GetObjPtr( unsigned const Index )
{
	return &(vecObjects[ vecIntermediateLayer[ Index ].ObjIdx ]);
}
Vertex * ObjMgr::GetLibVertexPtr( unsigned const Index )
{
	return &vecVertexLibrary[ Index ];
}
D3DSURFACE_DESC ObjMgr::GetSurfaceDesc()
{
	return SurfaceDesc;
}
void ObjMgr::CreateShotData( unsigned ID, BlendType blend, unsigned delay, RECT const & rect, D3DCOLOR color, std::vector< std::vector< float > > const & AnimationData )
{
	VertexCount = 6;
	ShotData shot_data;
	if( !AnimationData.size() )
	{
		shot_data.VtxOffset = vecVertexLibrary.size();
		shot_data.Delay = delay;
		shot_data.Radius = (ULONG)( pow( pow( (float)(rect.right - rect.left), 2.f ) + pow( (float)(rect.bottom - rect.top), 2.f ), 0.5f ) );
		shot_data.AnimationTime = -1;
		shot_data.NextShot = Bullet_Templates.size();
		PushQuadLib( rect, color );
		Bullet_Templates.push_back( shot_data );
	}
	else
	{
		for( unsigned i = 0; i < AnimationData.size(); ++i )
		{
			shot_data.VtxOffset = vecVertexLibrary.size();
			shot_data.Delay = delay;
			shot_data.AnimationTime = (ULONG)AnimationData[ i ][ 0 ];
			shot_data.Radius = (ULONG)pow( pow( AnimationData[ i ][ 3 ] - AnimationData[ i ][ 1 ], 2.f ) + pow( AnimationData[ i ][ 4 ] - AnimationData[ i ][ 2 ], 2.f ), 0.5f );
			shot_data.NextShot = Bullet_Templates.size() - (( i < AnimationData.size() - 1 )? 0 : i );
			RECT r = { (ULONG)AnimationData[ i ][ 1 ], (ULONG)AnimationData[ i ][ 2 ], (ULONG)AnimationData[ i ][ 3 ], (ULONG)AnimationData[ i ][ 4 ] };
			PushQuadLib( r, color );
			Bullet_Templates.push_back( shot_data );
		}
	}
}
void ObjMgr::CreateDelayShotData( unsigned ID, RECT const & rect, D3DCOLOR const color, FLOAT const Scale, ULONG const DelayFrames )
{
	if( ID >= Bullet_Delays.size() ) Bullet_Delays.resize( 1 + ID );
	DelayData delaydata = { vecVertexLibrary.size(), DelayFrames, Scale };
	PushQuadLib( rect, color );
	Bullet_Delays[ ID ] = delaydata;
}
void ObjMgr::AdvanceTransformedDraw( Direct3DEngine * D3DEng, bool Danmaku )
{
	if( vecVertexLibrary.size() )
	{
		LPDIRECT3DDEVICE9 d3ddev = D3DEng->GetDevice();
		unsigned s = vecObjects.size();
		D3DXMATRIX mat;
		if( VBufferLength < vecObjects.size() * VertexCount * sizeof( Vertex ) )
		{
			VBufferLength = vecObjects.size() * VertexCount * sizeof( Vertex );
			if( VertexBuffer ) VertexBuffer->Release();
			d3ddev->CreateVertexBuffer(  vecObjects.size() * VertexCount * sizeof( Vertex ), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &VertexBuffer, NULL );
		}
		Vertex * vtxptr;
		VertexBuffer->Lock( 0, 0, (void**)&vtxptr, 0 );
		for( unsigned u = 0; u < s; ++u )
		{
			Vertex * dst = &vtxptr[ u * VertexCount ];//&vecVertices[ u * VertexCount ];
			Vertex * src = Danmaku ? &vecVertexLibrary[ Bullet_Templates[ vecObjects[ u ].ShotData ].VtxOffset ] : &vecVertexLibrary[ vecObjects[ u ].libidx ];
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
		VertexBuffer->Unlock();
		DWORD ZSet;
		d3ddev->SetTexture( 0, pTexture );
		d3ddev->SetVertexDeclaration( VDeclaration );
		d3ddev->SetVertexShader( VShader );
		d3ddev->SetPixelShader( PShader );
		d3ddev->GetRenderState( D3DRS_ZENABLE, &ZSet );
		d3ddev->SetRenderState( D3DRS_ZENABLE, FALSE );
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
		d3ddev->SetRenderState( D3DRS_ZENABLE, ZSet );
	}
}
unsigned ObjMgr::GetObjCount() const
{
	return vecObjects.size();
}
unsigned ObjMgr::GetDelayDataSize() const
{
	return Bullet_Delays.size();
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
	if( FlagMotion( -1 ) )
	{
		position += velocity += accel;
		orient *= orientvel;
	}
}
bool Object::FlagMotion( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags = flags & 0x1) != 0;
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
		return (flags = flags & 0x2) != 0;
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
		return (flags = flags & 0x4) != 0;
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
		return (flags = flags & 0x8) != 0;
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
		return (flags = flags & 0x16) != 0;
	case 0:
		flags = flags & ~0x16;
		return false;
	case 1:
	default:
		flags = flags | 0x16;
		return true;
	}
}