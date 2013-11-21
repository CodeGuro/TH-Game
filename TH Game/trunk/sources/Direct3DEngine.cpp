#include <Direct3DEngine.hpp>
#include <ObjMgr.hpp>
#include <sstream>
#include <fstream>
#include <cassert>
#include <MMSystem.h>

Battery::Battery( HWND const hWnd ) : d3d( Direct3DCreate9( D3D_SDK_VERSION ) ) 
{
	////////////////////////
	/////Initialization/////
	////////////////////////

	assert( hWnd && !(LPDIRECT3DDEVICE9)d3ddev );
	RECT rec;
	GetClientRect( hWnd, &rec );

	D3DDISPLAYMODE d3ddm;
	D3DPRESENT_PARAMETERS d3dpp;
	d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, d3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8 ) - 1, &d3ddm );
		
	d3dpp.BackBufferWidth = rec.right - rec.left;
	d3dpp.BackBufferHeight = rec.bottom - rec.top;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = (MessageBox(NULL, "Fullscreen Mode?", "FULLSCREEN/WINDOW", MB_YESNO | MB_ICONQUESTION ) == IDNO)? TRUE : FALSE;
	d3dpp.EnableAutoDepthStencil= TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz = d3dpp.Windowed == TRUE? 0 : d3ddm.RefreshRate;
	d3dpp.Flags = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		
	d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd , D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev );
		
	D3DXMatrixIdentity( &WorldMatrix );
	D3DXMatrixLookAtLH( &ViewMatrix, &D3DXVECTOR3(0, 1,-10), &D3DXVECTOR3(0,1,0), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, D3DXToRadian(45), 640.f/480.f, 1.0f, 100.0f );
		
	GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	
	GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	

	GetDevice()->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
	GetDevice()->Present( NULL, NULL, NULL, NULL );

	///////////////
	////shaders////
	///////////////

	LPD3DXBUFFER pshaderbuff = NULL;
	LPD3DXBUFFER pshadererrbuff = NULL;
	if( !pDefaultVDeclaration )
	{
		D3DVERTEXELEMENT9 ve[] = 
		{
			{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};
		GetDevice()->CreateVertexDeclaration( ve, &pDefaultVDeclaration );
	}

	if( !pDefault3DVShader && !pDefaultConstable )
	{
		if( D3D_OK != D3DXCompileShaderFromFile( "Default3D.vs", NULL, NULL, "vs_main", "vs_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, &pDefaultConstable ) )
			MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Vertex Shader Compiler Error", "DX Shader Error", NULL );
		GetDevice()->CreateVertexShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &pDefault3DVShader );
		pshaderbuff->Release();
		if(pshadererrbuff ) pshadererrbuff->Release();
	}
	if( !pDefault3DPShader )
	{
		if( D3D_OK != D3DXCompileShaderFromFile( "Default3D.ps", NULL, NULL, "ps_main", "ps_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, NULL ) )
			MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Pixel Shader Compiler Error", "DX Shader Error", NULL );	
		GetDevice()->CreatePixelShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &pDefault3DPShader );
		pshaderbuff->Release();
		if(pshadererrbuff ) pshadererrbuff->Release();
	}
	if( !PipelineVertexBuffer.Buffer )
		d3ddev->CreateVertexBuffer( PipelineVertexBuffer.BufferSize = 1000, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &PipelineVertexBuffer.Buffer, NULL );

	DirectSoundCreate8( NULL, &dsound, NULL );
	GetDSound()->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );

	//0 - background
	//1 - 3D
	//2 - enemy boss
	//3 - player
	//4 - enemies
	//5 - bullets
	//6 - effects
	//7 - sprites/text
	//8 - foreground
	vVertexBuffers.resize( 1 );
	vVertexBuffers.back().RefCount = 4;
	vvObjects.resize( 4 );
	GetLayers().resize( 8 );
	GetLayers()[ 4 ].vObjMgr.resize( 4 );
	for( unsigned u = 0; u < 4; ++u )
	{
		ObjMgr & mgr = GetLayers()[ 4 ].vObjMgr[ u ];
		mgr.SetVertexDeclaration( GetDefaultVDeclaration() );
		mgr.SetVertexShader( GetDefaultVShader() );
		mgr.SetPixelShader( GetDefaultPShader() );
		mgr.SetVShaderConstTable( GetDefaultConstable() );
		mgr.SetVertexBufferIdx( 0 );
		mgr.SetVertexCount( 6 );
		mgr.SetObjBufferIdx( u );
		mgr.SetBlendState( (BlendType)u );
	}
	RECT r = { 32, 16, 416, 464 };
	GetDevice()->SetScissorRect( &r );
}
Battery::Battery()
{
}
LPDIRECT3DDEVICE9 & Battery::GetDevice()
{
	return d3ddev;
}
LPDIRECT3D9 & Battery::GetD3D()
{
	return d3d;
}
LPDIRECT3DVERTEXDECLARATION9 Battery::GetDefaultVDeclaration() const
{
	return pDefaultVDeclaration;
}
LPDIRECT3DVERTEXSHADER9 Battery::GetDefaultVShader() const
{
	return pDefault3DVShader;
}
LPDIRECT3DPIXELSHADER9 Battery::GetDefaultPShader() const
{
	return pDefault3DPShader;
}
LPD3DXCONSTANTTABLE Battery::GetDefaultConstable() const
{
	return pDefaultConstable;
}
LPDIRECTSOUND8 Battery::GetDSound() const
{
	return dsound;
}
Battery::vLayer_t & Battery::GetLayers()
{
	return vLayers;
}
void Battery::LoadShotImage( std::string const & pathname )
{
	ShotImagePath = pathname;
	LoadTexture( pathname );
	for( unsigned u = 0; u < 4; ++u )
		GetLayers()[ 4 ].vObjMgr[ u ].SetTexture( GetTexture( pathname ) );
}
unsigned Battery::CreateObject( unsigned short Layer ) //0 - BG, 4 - Bullet, 5 - Effect, 8 - Foreground
{
	unsigned Result;
	unsigned VtxBuffIdx;
	unsigned ObjVector;
	if( Layer <= GetLayers().size() )
	{
		if( vObjHandlesGC.size() )
		{
			Result = vObjHandlesGC.back();
			vObjHandlesGC.pop_back();
		}
		else
		{
			Result = vObjHandles.size();
			vObjHandles.resize( 1 + Result );
		}
		if( vvObjectsGC.size() )
		{
			ObjVector = vvObjectsGC.back();
			vvObjectsGC.pop_back();
		}
		else
		{
			ObjVector = vvObjects.size();
			vvObjects.resize( 1 + ObjVector );
		}
		if( Layer != 4 )
		{
			if( vVertexBuffersGC.size() )
			{
				VtxBuffIdx = vVertexBuffersGC.back();
				vVertexBuffersGC.pop_back();
			}
			else
			{
				VtxBuffIdx = vVertexBuffers.size();
				vVertexBuffers.resize( 1 + VtxBuffIdx );
			}
		}
		else
			VtxBuffIdx = -1;
		ObjHandle & handle = vObjHandles[ Result ];
		handle.Layer = Layer;
		handle.RefCount = 1;
		handle.MgrIdx = GetLayers()[ Layer ].vObjMgr.size();
		handle.VertexBuffer = VtxBuffIdx;
		handle.ObjVector = ObjVector;
		handle.ObjVectorIdx = vvObjects[ ObjVector ].size();
		handle.Type = Layer == 4 ? ObjShot : ObjEffect;
		ObjMgr * objMgr = &(*GetLayers()[ Layer ].vObjMgr.insert( GetLayers()[ Layer ].vObjMgr.end(), ObjMgr() ) );
		objMgr->SetVertexDeclaration( pDefaultVDeclaration );
		objMgr->SetVertexShader( pDefault3DVShader );
		objMgr->SetPixelShader( pDefault3DPShader );
		objMgr->SetVShaderConstTable( pDefaultConstable );
		objMgr->SetVertexBufferIdx( VtxBuffIdx );
		objMgr->SetObjBufferIdx( handle.ObjVector );
		objMgr->SetBlendState( BlendAlpha );

		vvObjects[ ObjVector ].resize( 1 + handle.ObjVectorIdx );
		Object & obj = *GetObject( Result );
		obj.orientvel = D3DXQUATERNION( 0, 0, 0, 1 );
		obj.accel = D3DXVECTOR3( 0, 0, 0 );
		obj.SetAngle( 0 );
		obj.SetVelocity( D3DXVECTOR3( 0, 0, 0 ) );
		obj.SetRotation( 0 );
		obj.SetScale( D3DXVECTOR3( 1, 1, 1 ) );
		obj.VertexOffset = 0;
		obj.FlagMotion( 1 );
		obj.FlagCollidable( 0 );
		obj.FlagPixelPerfect( 0 );
		obj.FlagScreenDeletable( 1 );
	}
	else
		Result = -1;
	return Result;
}
void Battery::AddRefObjHandle( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
		++vObjHandles[ HandleIdx ].RefCount;
}
void Battery::ReleaseObjHandle( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
		if( !--vObjHandles[ HandleIdx ].RefCount )
			vObjHandlesGC.push_back( HandleIdx );
}
void Battery::ReleaseObject( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		auto & vObjects = vvObjects[ handle.ObjVector ];
		vObjects.erase( vObjects.begin() + handle.ObjVectorIdx );
		for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
			if( it->ObjVector == handle.ObjVector && it->ObjVectorIdx > handle.ObjVectorIdx )
				--(it->ObjVectorIdx);

		if( !vObjects.size() && handle.Type != ObjShot )
		{
			auto & Layer = GetLayers()[ handle.Layer ];
			auto objmgr_it = Layer.vObjMgr.begin() + handle.MgrIdx;
			Layer.vObjMgr.erase( objmgr_it );
			if( !--(vVertexBuffers[ handle.VertexBuffer ].RefCount) )
				vVertexBuffersGC.push_back( handle.VertexBuffer );
			for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
				if( it->Layer == handle.Layer && it->MgrIdx > handle.MgrIdx )
					--(it->MgrIdx);

			handle.MgrIdx = -1;
		}
		handle.VertexBuffer = -1;
		handle.ObjVector = -1;
		handle.ObjVectorIdx = -1;
		handle.Layer = -1;
	}
}
Object * Battery::GetObject( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.ObjVectorIdx != -1 && handle.ObjVector!= -1 )
			return &vvObjects[ handle.ObjVector ][ handle.ObjVectorIdx ];
	}
	return 0;
}
ObjMgr * Battery::GetObjMgr( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.MgrIdx != -1 )
			return &GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ];
	}
	return 0;
}
void Battery::LoadTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it == mapTextures.end() )
	{
		mapTextures[ pathname ];
		D3DXCreateTextureFromFile( d3ddev, pathname.c_str(), &mapTextures[ pathname ] );
	}
}
LPDIRECT3DTEXTURE9 Battery::GetTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it == mapTextures.end() )
		return 0;
	return it->second;
}
void Battery::DeleteTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it != mapTextures.end() )
		mapTextures.erase( it );
}
void Battery::LoadSound( std::string const & pathname )
{
	if( mapSoundEffects.find( pathname ) != mapSoundEffects.end() )
		return;
	mapSoundEffects[ pathname ];

	LPDIRECTSOUNDBUFFER8 & SoundBuffer = mapSoundEffects[ pathname ];
	
	std::ifstream SoundFile = std::ifstream( pathname, std::ifstream::binary );
	ULONG FileSize;
	WaveHeaderType WaveFileHeader;
	WAVEFORMATEX WaveFormat;
	DSBUFFERDESC BufferDesc;
	SoundFile.seekg( 0, SoundFile.end );
	FileSize = (ULONG)SoundFile.tellg();
	SoundFile.seekg( 0, SoundFile.beg );
	LPVOID AudioPtr;
	DWORD AudioBytes;

	SoundFile.read( (char*)&WaveFileHeader, sizeof( WaveFileHeader ) );

	if( ( WaveFileHeader.chunkId[ 0 ] != 'R' ) || ( WaveFileHeader.chunkId[ 1 ] != 'I' ) || 
	   ( WaveFileHeader.chunkId[ 2 ] != 'F' ) || ( WaveFileHeader.chunkId[ 3 ] != 'F' ) )
		return;
	if( ( WaveFileHeader.format[ 0 ] != 'W' ) || ( WaveFileHeader.format[ 1 ] != 'A' ) ||
	   ( WaveFileHeader.format[ 2 ] != 'V' ) || ( WaveFileHeader.format[ 3 ] != 'E' ) )
		return;

	if( ( WaveFileHeader.subChunkId[ 0 ] != 'f' ) || ( WaveFileHeader.subChunkId[ 1 ] != 'm' ) ||
	   ( WaveFileHeader.subChunkId[ 2 ] != 't' ) || ( WaveFileHeader.subChunkId[ 3 ] != ' ' ) )
		return;

	if( ( WaveFileHeader.dataChunkId[ 0 ] != 'd' ) || ( WaveFileHeader.dataChunkId[ 1 ] != 'a' ) ||
	   ( WaveFileHeader.dataChunkId[ 2 ] != 't' ) || ( WaveFileHeader.dataChunkId[ 3 ] != 'a' ) )
		return;

	WaveFormat.wFormatTag = WaveFileHeader.audioFormat;
	WaveFormat.nSamplesPerSec = WaveFileHeader.sampleRate;
	WaveFormat.wBitsPerSample = WaveFileHeader.bitsPerSample;
	WaveFormat.nChannels = WaveFileHeader.numChannels;
	WaveFormat.nBlockAlign = (WaveFormat.wBitsPerSample / 8) * WaveFormat.nChannels;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
	WaveFormat.cbSize = 0;

	BufferDesc.dwSize = sizeof(DSBUFFERDESC);
	BufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	BufferDesc.dwBufferBytes = WaveFileHeader.dataSize;
	BufferDesc.dwReserved = 0;
	BufferDesc.lpwfxFormat = &WaveFormat;
	BufferDesc.guid3DAlgorithm = GUID_NULL;

	LPDIRECTSOUNDBUFFER TmpBuffer;
	GetDSound()->CreateSoundBuffer( &BufferDesc, &TmpBuffer, NULL );
	TmpBuffer->QueryInterface( IID_IDirectSoundBuffer8, (LPVOID*)&SoundBuffer );
	TmpBuffer->Release();

	SoundBuffer->Lock( 0, WaveFileHeader.dataSize, &AudioPtr, &AudioBytes, NULL, NULL, 0 );
	SoundFile.read( (char*)AudioPtr, AudioBytes );
	SoundBuffer->Unlock( AudioPtr, AudioBytes, NULL, NULL );

	SoundFile.close();
}
void Battery::PlaySound( std::string const & pathname )
{
	auto it = mapSoundEffects.find( pathname );
	if( it == mapSoundEffects.end() )
		return;

	LPDIRECTSOUNDBUFFER8 buffer = it->second;

	buffer->SetCurrentPosition( 0 );
	buffer->SetVolume( DSBVOLUME_MAX );
	buffer->Play( 0, 0, 0 );

}
void Battery::DeleteSound( std::string const & pathname )
{
	auto it = mapSoundEffects.find( pathname );
	if( it != mapSoundEffects.end() )
		mapSoundEffects.erase( it );
}
unsigned Battery::CreateShot01( D3DXVECTOR2 const & position, FLOAT const speed, FLOAT const direction, FLOAT const graphic )
{
	unsigned Result;
	if( 5 <= GetLayers().size() )
	{
		if( vObjHandlesGC.size() )
		{
			Result = vObjHandlesGC.back();
			vObjHandlesGC.pop_back();
		}
		else
			Result = vObjHandles.size();
		if( vObjHandles.size() <= Result )
			vObjHandles.resize( 1 + Result );
		ObjHandle & handle = vObjHandles[ Result ];
		handle.Layer = 4;
		handle.RefCount = 1;
		/*Get ObjMgr from graphic*/
		ShotData const & shot_data= Bullet_Templates[ (unsigned)graphic ];
		handle.ObjVector = (ULONG)shot_data.Render;
		handle.ObjVectorIdx = vvObjects[ handle.ObjVector ].size();
		handle.VertexBuffer = -1;
		handle.MgrIdx = -1;
		handle.Layer = -1;
		vvObjects[ handle.ObjVector ].resize( 1 + handle.ObjVectorIdx );
		Object & obj = *GetObject( Result );
		obj.position = D3DXVECTOR3( position.x, position.y, 0.f );
		obj.velocity = D3DXVECTOR3( speed * cos( direction ), speed * sin( direction ), 0.f );
		obj.orientvel = D3DXQUATERNION( 0, 0, 0, 1 );
		obj.accel = D3DXVECTOR3( 0, 0, 0 );
		obj.SetRotation( D3DX_PI / 2 );
		obj.SetAngle( direction );
		obj.SetScale( D3DXVECTOR3( 1, 1, 1 ) );
		obj.VertexOffset= shot_data.VtxOffset;
		obj.FlagMotion( 1 );
		obj.FlagCollidable( 1 );
		obj.FlagScreenDeletable( 1 );
		obj.FlagPixelPerfect( shot_data.Flags & 0x16 ? 1 : 0 );
		return Result;
	}
	return -1;
}
void Battery::CreateShotData( unsigned ID, BlendType blend, unsigned delay, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData )
{
	assert( GetLayers().size() > 4 );
	ShotData shot_data;
	unsigned i = 0;
	do
	{
		shot_data.VtxOffset = vVertexBuffers[ 0 ].VertexBuffer.size();
		shot_data.Delay = delay;
		shot_data.Flags = flags;
		shot_data.Render = blend;
		shot_data.NextShot = AnimationData.size() ? 1 + Bullet_Templates.size() - (( i < AnimationData.size() - 1 )? 0 : i + 1 ) : Bullet_Templates.size();
		RECT r;
		if( AnimationData.size() )
		{
			shot_data.AnimationTime = (ULONG)AnimationData[ i ][ 0 ];
			shot_data.Radius = (ULONG)pow( pow( AnimationData[ i ][ 3 ] - AnimationData[ i ][ 1 ], 2.f ) + pow( AnimationData[ i ][ 4 ] - AnimationData[ i ][ 2 ], 2.f ), 0.5f );
			RECT r2 = { (ULONG)AnimationData[ i ][ 1 ], (ULONG)AnimationData[ i ][ 2 ], (ULONG)AnimationData[ i ][ 3 ], (ULONG)AnimationData[ i ][ 4 ] };
			r = r2;
		}
		else
		{
			shot_data.AnimationTime = 0;
			shot_data.Radius = (ULONG)pow( pow( (float)(rect.right - rect.left), 2.f ) + pow( (float)(rect.bottom - rect.top), 2.f ), 0.5f );
			r = rect;
		}
		PushQuadShotBuffer( r, color );
		Bullet_Templates.push_back( shot_data );
	}while( ++i < AnimationData.size() );
}
void Battery::CreateDelayShotData( unsigned ID, RECT const & rect, D3DCOLOR const color, FLOAT const Scale, ULONG const DelayFrames )
{
	assert( GetLayers().size() > 4 );
	ObjMgr & bullet_mgr = GetLayers()[ 4 ].vObjMgr[ 0 ];
	bullet_mgr.SetVertexCount( 6 );
	if( ID >= Bullet_Delays.size() ) Bullet_Delays.resize( 1 + ID );
	DelayData delaydata = { vVertexBuffers[ 0 ].VertexBuffer.size(), DelayFrames, Scale };
	PushQuadShotBuffer( rect, color );
	Bullet_Delays[ ID ] = delaydata;
}
ShotData const & Battery::GetBulletTemplates( unsigned const graphic ) const
{
	return Bullet_Templates[ graphic ];
}
unsigned Battery::GetDelayDataSize() const
{
	return Bullet_Delays.size();
}
void Battery::PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color )
{
	D3DSURFACE_DESC SurfaceDesc;
	GetTexture( ShotImagePath )->GetLevelDesc( 0, &SurfaceDesc );
	Vertex v[6] = 
	{
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color }
	};
	for( unsigned u = 0; u < 6; ++u ) vVertexBuffers[ 0 ].VertexBuffer.push_back( v[u] );
}
D3DVBuffer & Battery::GetPipelineVBuffer()
{
	return PipelineVertexBuffer;
}

//ObjEffect functions
void Battery::ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.VertexBuffer != -1 )
		{
			vVertexBuffers[ handle.VertexBuffer ].VertexBuffer.resize( VertexCount );
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].SetVertexCount( VertexCount );
		}
	}
}
void Battery::ObjEffect_SetVertexXY( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR2 Posxy )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.VertexBuffer != -1 )
			vVertexBuffers[ handle.VertexBuffer ].VertexBuffer[ VIndex ].pos = D3DXVECTOR3( Posxy.x, Posxy.y, 0.f );
	}
}
void Battery::ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.VertexBuffer != -1 )
			vVertexBuffers[ handle.VertexBuffer ].VertexBuffer[ VIndex ].tex = Posuv;
	}
}
void Battery::ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.VertexBuffer != -1 )
			vVertexBuffers[ handle.VertexBuffer ].VertexBuffer[ VIndex ].color = Color;
	}
}
void Battery::ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.MgrIdx != -1 )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].SetBlendState( BlendState );
	}
}
void Battery::ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.MgrIdx != -1 )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].SetPrimitiveType( PrimitiveType );
	}
}
void Battery::ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.Layer != -1  && handle.Layer != Layer )
		{
			auto & objvec = GetLayers()[ handle.Layer ].vObjMgr;
			ULONG mgrIdx = GetLayers()[ Layer ].vObjMgr.size();
			GetLayers()[ Layer ].vObjMgr.push_back( objvec[ handle.MgrIdx ] );
			objvec.erase( objvec.begin() + handle.MgrIdx );
			for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
				if( it->Layer == handle.Layer && it->MgrIdx > handle.MgrIdx )
					--(it->MgrIdx);
			handle.Layer = (USHORT)Layer;
			handle.MgrIdx = mgrIdx;
		}
	}
}

Direct3DEngine::Direct3DEngine()
{
}
void Direct3DEngine::ToggleWindowed()
{
	D3DPRESENT_PARAMETERS d3dpp;
	LPDIRECT3DSWAPCHAIN9 psc;
	D3DDISPLAYMODE d3ddm;

	GetD3D()->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, GetD3D()->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8 ) - 1, &d3ddm );
	GetDevice()->GetSwapChain( 0, &psc );
	psc->GetPresentParameters( &d3dpp );
	d3dpp.Windowed = (BOOL)!d3dpp.Windowed;
	d3dpp.FullScreen_RefreshRateInHz = (d3dpp.Windowed? 0 : d3ddm.RefreshRate);
	GetDevice()->Reset( &d3dpp );
	GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	GetDevice()->SetVertexShader( GetDefaultVShader() );
	GetDevice()->SetPixelShader( GetDefaultPShader() );

	RECT rec = { 0, 0, 640, 480 };
	AdjustWindowRect( &rec, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, FALSE );
	if( d3dpp.Windowed )
		MoveWindow( d3dpp.hDeviceWindow, 100, 100, rec.right - rec.left, rec.bottom - rec.top, FALSE );
	
}
void Direct3DEngine::RenderFrame( MSG const msg )
{
	GetDevice()->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
	GetDevice()->BeginScene();
	DrawGridTerrain( 1000, 1000, 1.f );
//	DrawTexture();
	DrawObjects();
//	DrawFPS();
	GetDevice()->EndScene();
	GetDevice()->Present( NULL, NULL, NULL, NULL );
	ProcUserInput( msg );
}
void Direct3DEngine::DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing )
{
	GetDevice()->SetTransform( D3DTS_WORLD, &WorldMatrix );
	GetDevice()->SetTransform( D3DTS_VIEW, &ViewMatrix );
	GetDevice()->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );
	LPDIRECT3DVERTEXBUFFER9 pvb;
	GetDevice()->CreateVertexBuffer( 2 * sizeof( Vertex ) * ( Rows + Columns), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pvb, NULL );

	void * ptr;
	Vertex * pverts;
	pvb->Lock( 0, 0, &ptr, D3DLOCK_DISCARD );
	pverts = (Vertex*)ptr;
	for( unsigned i = 0; i < Columns; ++i )
	{
		Vertex v1 = { D3DXVECTOR3( Spacing / 2.f * (float)Columns - Spacing * (float)i, 0, Spacing / 2 * (float)Rows ), D3DXVECTOR2( 0, 0 ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) };
		Vertex v2 = { D3DXVECTOR3( Spacing / 2.f * (float)Columns - Spacing * (float)i, 0, Spacing / -2 * (float)Rows ), D3DXVECTOR2( 0, 0 ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) };
		*pverts++ = v1;
		*pverts++ = v2;
	}
	for( unsigned i = 0; i < Rows; ++i )
	{
		Vertex v1 = { D3DXVECTOR3( Spacing / 2.f * (float)Columns, 0, Spacing / 2.f * Columns - Spacing * (float)i ), D3DXVECTOR2( 0, 0 ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) };
		Vertex v2 = { D3DXVECTOR3( Spacing / -2.f * (float)Columns, 0, Spacing / 2.f * Columns - Spacing * (float)i ), D3DXVECTOR2( 0, 0 ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) };
		*pverts++ = v1;
		*pverts++ = v2;
	}
	pvb->Unlock();
	GetDevice()->SetVertexDeclaration( GetDefaultVDeclaration() );
	GetDevice()->SetStreamSource( 0, pvb, 0, sizeof( Vertex ) );
	GetDefaultConstable()->SetMatrix( GetDevice(), "WorldViewProjMat", &( WorldMatrix * ViewMatrix * ProjectionMatrix ) );
	GetDevice()->SetVertexShader( GetDefaultVShader() );
	GetDevice()->SetPixelShader( GetDefaultPShader() );
	GetDevice()->SetTexture( 0, NULL );
	GetDevice()->DrawPrimitive( D3DPT_LINELIST, 0, Rows + Columns );
	pvb->Release();
}
void Direct3DEngine::DrawTexture()
{
	LPDIRECT3DVERTEXBUFFER9 pvb;
	const char * csTexture = "tester.png";
	LoadTexture( csTexture );
	
	Vertex verts[] =
	{
		{ D3DXVECTOR3( -128.f, -128.f, 0.f ), D3DXVECTOR2( 0.f, 0.f ), D3DCOLOR_ARGB( 200, 0, 0, 255 ) },
		{ D3DXVECTOR3(128.f, -128.f, 0.f ), D3DXVECTOR2( 1.f, 0.f ), D3DCOLOR_ARGB( 200, 255, 0 , 0 ) },
		{ D3DXVECTOR3(-128.f, 128.f, 0.f ), D3DXVECTOR2( 0.f, 1.f ), D3DCOLOR_ARGB( 200, 255, 255, 255 ) },
		{ D3DXVECTOR3(128.f, 128.f, 0.f ), D3DXVECTOR2( 1.f, 1.f ), D3DCOLOR_ARGB( 200, 0, 255, 0 ) },
	};
	GetDevice()->CreateVertexBuffer( sizeof( verts ), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pvb, 0 );
	
	void * ptr;
	pvb->Lock( 0, NULL, &ptr, D3DLOCK_DISCARD );
	memcpy( ptr, verts, sizeof( verts ) );
	pvb->Unlock();

	D3DXMATRIX world, view, proj;
	D3DXMatrixIdentity( &world );
	D3DXMatrixOrthoLH( &proj, 640.f, -480.f, 0.f, 100.f );
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0,0,-1.f), &D3DXVECTOR3(0,0,0 ), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixTranslation( &world, -640.f/4 - 0.5f, 0.f - 0.5f, 0.f );
	GetDefaultConstable()->SetMatrix( GetDevice(), "WorldViewProjMat", &(world*view*proj) );
	//GetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	GetDevice()->SetTexture( 0, GetTexture( std::string( csTexture ) ) );
	GetDevice()->SetStreamSource( 0, pvb, 0, sizeof( Vertex ) );
	GetDevice()->SetVertexDeclaration( GetDefaultVDeclaration() );
	GetDevice()->SetVertexShader( GetDefaultVShader() );
	GetDevice()->SetPixelShader( GetDefaultPShader() );
	GetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	GetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	GetDevice()->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	GetDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	pvb->Release();

}
void Direct3DEngine::DrawFPS()
{

	static DWORD Frame;
	static DWORD FrameShow;
	static DWORD TicDelta;
	DWORD CurrentTick = GetTickCount();
	if( CurrentTick - TicDelta >= 1000  )
	{
		FrameShow = Frame;
		TicDelta = CurrentTick;
		Frame = -1;
	}
	LPD3DXFONT pFont;
	D3DXCreateFont( GetDevice(), 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Lucida", &pFont );
	RECT rec = { 0, 0, 640, 480 };
	std::stringstream ss;
	ss << FrameShow;
	pFont->DrawTextA( NULL, ss.str().c_str(), -1, &rec, DT_BOTTOM | DT_SINGLELINE | DT_RIGHT, D3DCOLOR_ARGB( 100, 255, 255, 255 ) );
	pFont->Release();
	++Frame;
	
}
void Direct3DEngine::DrawObjects()
{
	D3DXMATRIX world, view, proj;
	D3DXMatrixIdentity( &world );
	D3DXMatrixOrthoLH( &proj, 640.f, -480.f, 0.f, 100.f );
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0,0,-1.f), &D3DXVECTOR3(0,0,0 ), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixTranslation( &world, -320.f - 0.5f, -240.f - 0.5f, 0.f );
	GetDefaultConstable()->SetMatrix( GetDevice(), "WorldViewProjMat", &(world*view*proj) );
	for( auto L = GetLayers().begin(); L < GetLayers().end(); ++L )
	{
		GetDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, (L > GetLayers().begin() && L < GetLayers().end() - 1)? TRUE : FALSE );
		for( auto Obj = L->vObjMgr.begin(); Obj != L->vObjMgr.end(); ++Obj )
			Obj->AdvanceDrawDanmaku( this );
	}
}
void Direct3DEngine::ProcUserInput( MSG const Msg )
{
	if( Msg.message == WM_KEYDOWN )
	{
		D3DXMATRIX DisplaceMat;
		switch( Msg.wParam )
		{
		case VK_UP:
			D3DXMatrixTranslation( &DisplaceMat, 0.0f, 0.0f, -0.1f );
			break;
		case VK_DOWN:
			D3DXMatrixTranslation( &DisplaceMat, 0.0f, 0.0f, 0.1f );
			break;
		case VK_LEFT:
			D3DXMatrixRotationY( &DisplaceMat, D3DX_PI / 180.f );
			break;
		case VK_RIGHT:
			D3DXMatrixRotationY( &DisplaceMat, D3DX_PI / -180.f );
			break;
		case VK_F11:
			ToggleWindowed();
		default:
			D3DXMatrixIdentity( &DisplaceMat );
		}
		ViewMatrix *= DisplaceMat;
	}
}