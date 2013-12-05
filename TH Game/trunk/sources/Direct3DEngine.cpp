#include <Direct3DEngine.hpp>
#include <ObjMgr.hpp>
#include <sstream>
#include <fstream>
#include <cassert>
#include <MMSystem.h>

extern const std::string DefaultShader;

unsigned VBufferMgr::FetchVertexBuffer()
{
	unsigned res;
	if( GC.size() )
	{
		res = GC.back();
		GC.pop_back();
	}
	else
	{
		res = VertexBuffers.size();
		VertexBuffers.resize( res + 1 );
	}
	VertexBuffers[ res ].RefCount = 1;
	return res;
}
vector< Vertex > & VBufferMgr::GetVertexBuffer( unsigned Idx )
{
	return VertexBuffers[ Idx ].VertexBuffer;
}
void VBufferMgr::AddRefVertexBuffer( unsigned Idx )
{
	++VertexBuffers[ Idx ].RefCount;
}
void VBufferMgr::DisposeVertexBuffer( unsigned Idx )
{
	if( VertexBuffers[ Idx ].RefCount == 0 )
		assert( 0 );
	if( !--VertexBuffers[ Idx ].RefCount )
	{
		VertexBuffers[ Idx ].VertexBuffer.resize( 0 );
		GC.push_back( Idx );
	}
}

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
		if( D3D_OK != D3DXCompileShader( DefaultShader.c_str(), DefaultShader.size(), NULL, NULL, "vs_main", "vs_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, &pDefaultConstable ) )
			MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Vertex Shader Compiler Error", "DX Shader Error", NULL );
		GetDevice()->CreateVertexShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &pDefault3DVShader );
		pshaderbuff->Release();
		if(pshadererrbuff ) pshadererrbuff->Release();
	}
	if( !pDefault3DPShader )
	{
		if( D3D_OK != D3DXCompileShader( DefaultShader.c_str(), DefaultShader.size(), NULL, NULL, "ps_main", "ps_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, NULL ) )
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
	unsigned const BulletMgrCount = 4;
	unsigned const LayerCount = 8;
	unsigned const BulletLayer = 4;
	unsigned VertexBufferIdx = FetchVertexBuffer();
	for( unsigned u = 1; u < BulletMgrCount ; ++u ) AddRefVertexBuffer( VertexBufferIdx );
	vvObjects.resize( BulletMgrCount );
	GetLayers().resize( LayerCount );
	GetLayers()[ BulletLayer ].vObjMgr.resize( BulletMgrCount );
	for( unsigned u = 0; u < BulletMgrCount; ++u )
	{
		ObjMgr & mgr = GetLayers()[ BulletLayer ].vObjMgr[ u ];
		mgr.VDeclaration = GetDefaultVDeclaration();
		mgr.VShader = GetDefaultVShader();
		mgr.PShader = GetDefaultPShader();
		mgr.Constable = GetDefaultConstable();
		mgr.VertexBufferIdx = VertexBufferIdx;
		mgr.VertexCount = 6;
		mgr.ObjBufferIdx = u ;
		mgr.BlendState = (BlendType)u;
		mgr.ObjFontIdx = -1;
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
		GetLayers()[ 4 ].vObjMgr[ u ].pTexture = GetTexture( pathname ) ;
}
unsigned Battery::CreateObjHandle()
{
	unsigned Result;
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
	memset( &vObjHandles[ Result ], -1, sizeof( vObjHandles[ Result ] ) );
	vObjHandles[ Result ].RefCount = 1;
	return Result;
}
unsigned Battery::CreateObject( ObjType type ) //0 - 3D, 1 - Background, 2 - Boss, 3 - Player, 4 - Bullet, 5 - Effect, 6 - Foreground, 7 - Text
{
	unsigned Result;
	unsigned Layer;
	ObjMgr * objMgr;

	switch( type )
	{
	case ObjShot: Layer = 4; break;
	case ObjEffect: Layer = 5; break;
	case ObjFont: Layer = 7; break;
	default:
		Layer = 4;
	}

	if( type == ObjShot )
		Result = CreateShot( 0 );
	else
	{
		Result = CreateObjHandle();
		ObjHandle & handle = vObjHandles[ Result ];
		if( vvObjectsGC.size() )
		{
			handle.ObjVector = vvObjectsGC.back();
			vvObjectsGC.pop_back();
		}
		else
		{
			handle.ObjVector = vvObjects.size();
			vvObjects.resize( 1 + handle.ObjVector );
		}
		if( Layer != 4 && Layer != 7 )
		{
			handle.VertexBuffer = FetchVertexBuffer();
		}
		else
			handle.VertexBuffer = -1;

		handle.Layer = Layer;
		handle.RefCount = 1;
		handle.MgrIdx = GetLayers()[ Layer ].vObjMgr.size();
		handle.ObjVectorIdx = vvObjects[ handle.ObjVector ].size();
		handle.ObjFontIdx = ( (Layer == 7 )? CreateFontObject() : -1 );
		handle.Type = type;

		if( Layer != 4 )
		{
			ObjMgr * objMgr = &(*GetLayers()[ Layer ].vObjMgr.insert( GetLayers()[ Layer ].vObjMgr.end(), ObjMgr() ) );
			objMgr->VDeclaration = pDefaultVDeclaration;
			objMgr->VShader = pDefault3DVShader;
			objMgr->PShader = pDefault3DPShader;
			objMgr->Constable = pDefaultConstable;
			objMgr->VertexBufferIdx = handle.VertexBuffer;
			objMgr->ObjBufferIdx = handle.ObjVector;
			objMgr->BlendState = BlendAlpha;
			objMgr->ObjFontIdx = handle.ObjFontIdx;
		}

		vvObjects[ handle.ObjVector ].resize( 1 + handle.ObjVectorIdx );
		Object & obj = *GetObject( Result );
		obj.FlagCollidable( Layer == 4 ? 1 : 0 );
		obj.FlagPixelPerfect( 0 );
		obj.FlagScreenDeletable( 1 );
		obj.FlagBullet( 0 );
		obj.FlagGraze( 0 );
		obj.SetVelocity( D3DXVECTOR3( 1, 1, 1 ) );
		obj.SetSpeed( 0 );
		obj.SetAngle( 0 );
		obj.SetRotationVelocity( 0 );
		obj.SetAccel( D3DXVECTOR3( 0, 0, 0 ) );
		obj.SetScale( D3DXVECTOR3( 1, 1, 1 ) );
		obj.VertexOffset = 0;
	}

	return Result;
}
void Battery::AddRefObjHandle( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
		++vObjHandles[ HandleIdx ].RefCount;
}
void Battery::ReleaseObjHandle( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
		if( !--vObjHandles[ HandleIdx ].RefCount )
			vObjHandlesGC.push_back( HandleIdx );
}
void Battery::ReleaseObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.ObjVector ) && CheckValidIdx( handle.ObjVectorIdx ) )
		{
			auto & vObjects = vvObjects[ handle.ObjVector ];
			vObjects.erase( vObjects.begin() + handle.ObjVectorIdx );

			for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
				if( it->ObjVector == handle.ObjVector && it->ObjVectorIdx > handle.ObjVectorIdx )
					--(it->ObjVectorIdx);
		}
		if( handle.Type != ObjShot )
		{
			if( CheckValidIdx( handle.Layer ) )
			{
				auto & Layer = GetLayers()[ handle.Layer ];
				if( CheckValidIdx( handle.MgrIdx ) )
				{
					auto objmgr_it = Layer.vObjMgr.begin() + handle.MgrIdx;
					assert( handle.ObjVector == objmgr_it->ObjBufferIdx );
					assert( handle.MgrIdx == objmgr_it - Layer.vObjMgr.begin() );
					assert( handle.VertexBuffer == objmgr_it->VertexBufferIdx );
					assert( handle.ObjFontIdx == objmgr_it->ObjFontIdx );
					vvObjectsGC.push_back( handle.ObjVector );
					Layer.vObjMgr.erase( objmgr_it );
					if( CheckValidIdx( handle.VertexBuffer ) )
						DisposeVertexBuffer( handle.VertexBuffer );
					if( CheckValidIdx( handle.ObjFontIdx ) )
						vFontObjects.erase( vFontObjects.begin() + handle.ObjFontIdx );
					for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
						if( it->Layer == handle.Layer && it->MgrIdx > handle.MgrIdx )
							--(it->MgrIdx);
				}
			}
		}
		auto RefCount = handle.RefCount;
		memset( &handle, -1, sizeof( handle ) );
		handle.RefCount = RefCount;
	}
}
Object * Battery::GetObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.ObjVectorIdx ) && CheckValidIdx( handle.ObjVector ) )
			return &vvObjects[ handle.ObjVector ][ handle.ObjVectorIdx ];
	}
	return 0;
}
ObjMgr * Battery::GetObjMgr( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) && CheckValidIdx( handle.Layer ) )
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
unsigned Battery::CreateFontObject()
{
	unsigned res;
	RECT r = { 0, 0, 640, 480 };
	res = vFontObjects.size();
	vFontObjects.resize( 1 + res );
	D3DXCreateFont( GetDevice(), 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Calibri", &vFontObjects[ res ].pFont );
	vFontObjects.back().Color = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	vFontObjects.back().Format = DT_TOP | DT_LEFT;
	vFontObjects.back().Rect = r;
	vFontObjects.back().String = "Hello World!";
	return res;
}
FontObject * Battery::GetFontObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.ObjFontIdx ) )
			return &vFontObjects[ handle.ObjFontIdx ];
	}
	return 0;
}
unsigned Battery::CreateShot( ULONG GraphicID )
{
	unsigned Result;
	Result = CreateObjHandle();
	ObjHandle & handle = vObjHandles[ Result ];
	handle.Layer = 4;
	handle.RefCount = 1;
	/*Get ObjMgr from graphic*/
	unsigned const TemplateOffset = Bullet_TemplateOffsets[ GraphicID ];
	ShotData const & shot_data= Bullet_Templates[ TemplateOffset ];
	handle.MgrIdx = (ULONG)shot_data.Render;
	handle.ObjVector = (ULONG)shot_data.Render;
	handle.ObjVectorIdx = vvObjects[ handle.ObjVector ].size();
	handle.Type = ObjShot;
	auto & objvector = vvObjects[ handle.ObjVector ];
	objvector.resize( 1 + handle.ObjVectorIdx );
	objvector[ handle.ObjVectorIdx ].ShotInit();
	objvector[ handle.ObjVectorIdx ].SetShotDataParams( shot_data, TemplateOffset );
	return Result;
}
void Battery::CreateShotData( unsigned ID, BlendType blend, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData )
{
	assert( GetLayers().size() > 4 );
	ShotData shot_data;
	if( Bullet_TemplateOffsets.size() <= ID )
		Bullet_TemplateOffsets.resize( 1 + ID );
	Bullet_TemplateOffsets[ ID ] = Bullet_Templates.size();
	unsigned i = 0;
	do
	{
		shot_data.VtxOffset = GetVertexBuffer( 0 ).size();
		shot_data.Flags = flags;
		shot_data.Render = blend;
		shot_data.NextShot = AnimationData.size() ? 1 + Bullet_Templates.size() - (( i < AnimationData.size() - 1 )? 0 : i + 1 ) : Bullet_Templates.size();
		RECT r;
		if( AnimationData.size() )
		{
			shot_data.AnimationTime = (ULONG)AnimationData[ i ][ 0 ];
			shot_data.Radius = pow( pow( AnimationData[ i ][ 3 ] - AnimationData[ i ][ 1 ], 2.f ) + pow( AnimationData[ i ][ 4 ] - AnimationData[ i ][ 2 ], 2.f ), 0.5f );
			RECT r2 = { (ULONG)AnimationData[ i ][ 1 ], (ULONG)AnimationData[ i ][ 2 ], (ULONG)AnimationData[ i ][ 3 ], (ULONG)AnimationData[ i ][ 4 ] };
			r = r2;
		}
		else
		{
			shot_data.AnimationTime = -1;
			shot_data.Radius = pow( pow( (float)(rect.right - rect.left), 2.f ) + pow( (float)(rect.bottom - rect.top), 2.f ), 0.5f );
			r = rect;
		}
		PushQuadShotBuffer( r, color );
		Bullet_Templates.push_back( shot_data );
	}while( ++i < AnimationData.size() );
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
	for( unsigned u = 0; u < 6; ++u ) GetVertexBuffer( 0 ).push_back( v[u] );
}

//ObjEffect functions
void Battery::ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
		{
			GetVertexBuffer( handle.VertexBuffer ).resize( VertexCount );
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].VertexCount = VertexCount;
		}
	}
}
void Battery::ObjEffect_SetVertexXY( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR2 Posxy )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].pos = D3DXVECTOR3( Posxy.x, Posxy.y, 0.f );
	}
}
void Battery::ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].tex = Posuv;
	}
}
void Battery::ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].color = Color;
	}
}
void Battery::ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].BlendState = BlendState;
	}
}
void Battery::ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].PrimitiveType = PrimitiveType;
	}
}
void Battery::ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.Layer ) && CheckValidIdx( handle.Layer ) )
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

//ObjShot functions
void Battery::ObjShot_SetGraphic( unsigned HandleIdx, ULONG ID )
{
	Object * pObj = GetObject( HandleIdx );
	if( pObj && ID < Bullet_TemplateOffsets.size() && pObj->FlagBullet( -1 ) )
	{
		//create a new shot -> copy old shot to new shot with new ID -> release old handle -> copy new handle to new handle -> release new handle
		ULONG BufferOffset = Bullet_TemplateOffsets[ ID ];
		ShotData const & shot_data = Bullet_Templates[ BufferOffset ];
		pObj->SetShotDataParams( shot_data, BufferOffset );
		Object ObjCopy = *pObj;
		unsigned ResHandle = CreateShot( ID );
		*GetObject( ResHandle ) = *pObj;
		ReleaseObject( HandleIdx );
		vObjHandles[ HandleIdx ] = vObjHandles[ ResHandle ];
		ReleaseObjHandle( ResHandle );
	}
}

//misc
void Battery::DrawObjects()
{
	D3DXMATRIX world, view, proj;
	D3DXMatrixIdentity( &world );
	D3DXMatrixOrthoLH( &proj, 640.f, -480.f, 0.f, 100.f );
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0,0,-1.f), &D3DXVECTOR3(0,0,0 ), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixTranslation( &world, -320.f - 0.5f, -240.f - 0.5f, 0.f );
	pDefaultConstable->SetMatrix( GetDevice(), "WorldViewProjMat", &(world*view*proj) );
	for( auto L = GetLayers().begin(); L < GetLayers().end(); ++L )
	{
		GetDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, (L < GetLayers().end() - 2 )? TRUE : FALSE );
		for( auto Objmgr = L->vObjMgr.begin(); Objmgr != L->vObjMgr.end(); )
		{
			D3DXMATRIX mat;
			Vertex * pstart = 0;
			Vertex * ptr = 0;
			if( CheckValidIdx( Objmgr->ObjBufferIdx ) && CheckValidIdx( Objmgr->VertexBufferIdx ) )
			{
				auto & vObjects = vvObjects[ Objmgr->ObjBufferIdx ];
				ULONG min_buffersize = vObjects.size() * Objmgr->VertexCount * sizeof( Vertex );
				if( PipelineVertexBuffer.BufferSize < min_buffersize )
				{
					PipelineVertexBuffer.Buffer->Release();
					d3ddev->CreateVertexBuffer( PipelineVertexBuffer.BufferSize = min_buffersize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &PipelineVertexBuffer.Buffer, NULL );
				}
				PipelineVertexBuffer.Buffer->Lock( 0, min_buffersize, (void**)&pstart, 0 );
				ptr = pstart;

				for( auto pObj = vObjects.begin(); pObj < vObjects.end(); )
				{
					if( pObj->FlagBullet( -1 ) )
					{
						if( !pObj->Time-- )
						{
							ULONG NextShot = Bullet_Templates[ pObj->BufferOffset ].NextShot;
							pObj->SetShotDataParams( Bullet_Templates[ NextShot ], NextShot );
						}
					}
					Vertex * src = &GetVertexBuffer( Objmgr->VertexBufferIdx )[ pObj->VertexOffset ];
					D3DXMatrixTransformation( &mat, NULL, NULL, &pObj->scale, NULL, &pObj->orient, pObj->FlagPixelPerfect( -1 ) ? &D3DXVECTOR3( floor( pObj->position.x + 0.5f), floor( pObj->position.y + 0.5f), floor( pObj->position.z + 0.5f ) ) : &pObj->position );

					if( !pObj->FlagScreenDeletable( -1 ) || !( pObj->position.x < 0.f || pObj->position.y < 0.f ) && ( pObj->position.x < 448.f && pObj->position.y < 480.f ) )
					{
						for( unsigned v = 0; v < Objmgr->VertexCount; ++v )
						{
							D3DXVec3TransformCoord( &ptr->pos, &src->pos, &mat );
							ptr->tex = src->tex;
							ptr->color = src->color;
							++ptr;
							++src;
						}
						pObj->Advance();
						++pObj;
					}
					else
					{
						auto objidx = pObj - vObjects.begin();
						auto ObjMgrIdx = Objmgr - L->vObjMgr.begin();
						unsigned u;
						for( u = 0; u < vObjHandles.size(); ++u )
						{
							ObjHandle & handle = vObjHandles[ u ];
							if( handle.Layer == L - vLayers.begin() && handle.MgrIdx == Objmgr - L->vObjMgr.begin()  && handle.ObjVector == Objmgr->ObjBufferIdx && handle.ObjVectorIdx == objidx )
							{
								ReleaseObject( u );
								break;
							}
						}
						if( u == vObjHandles.size() )
							vObjects.erase( pObj );

						Objmgr = L->vObjMgr.begin() + ObjMgrIdx;
						pObj = vObjects.begin() + objidx;
						continue;
					}
				}

				PipelineVertexBuffer.Buffer->Unlock();

				if( Objmgr == L->vObjMgr.end() )
					continue;
			}
			GetDevice()->SetTexture( 0, Objmgr->pTexture );
			GetDevice()->SetVertexDeclaration( Objmgr->VDeclaration );
			GetDevice()->SetVertexShader( Objmgr->VShader );
			GetDevice()->SetPixelShader( Objmgr->PShader );

			GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			GetDevice()->SetRenderState( D3DRS_ZENABLE, L == GetLayers().begin() ? TRUE : FALSE );
	
			switch( Objmgr->BlendState )
			{
			case BlendMult:
				GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
				GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
				GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
				break;

			case BlendAdd:
				GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
				GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
				GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
				break;

			case BlendSub:
				GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
				GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
				GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT ); 
				break;
	
			case BlendAlpha:
				GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
				GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
				GetDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
				break;

			default:
				abort();
			}
			GetDevice()->SetStreamSource( 0, PipelineVertexBuffer.Buffer, 0, sizeof( Vertex ) );
			DWORD VCount = ptr - pstart;
			if( VCount )
				GetDevice()->DrawPrimitive( Objmgr->PrimitiveType, 0, Objmgr->PrimitiveType == D3DPT_TRIANGLELIST ? VCount / 3 :
					Objmgr->PrimitiveType == D3DPT_TRIANGLESTRIP || Objmgr->PrimitiveType == D3DPT_TRIANGLEFAN ? VCount - 2 : 0 );

			if( CheckValidIdx( Objmgr->ObjFontIdx ) )
			{
				FontObject & FontObj = vFontObjects[ Objmgr->ObjFontIdx ];
				FontObj.pFont->DrawTextA( NULL, FontObj.String.c_str(), -1, &FontObj.Rect, FontObj.Format, FontObj.Color );
			}
			++Objmgr;
		}
	}
	GetDevice()->SetTexture( 0, 0 );
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
	DrawObjects();
	DrawFPS();
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