#include <Direct3DEngine.hpp>
#include <ObjMgr.hpp>
#include <sstream>
#include <fstream>
#include <regex>
#include <cassert>
#include <iostream>

extern const std::string DefaultShader;
unsigned const BACKGROUND_LAYER = 0;
unsigned const THREED_LAYER = 1;
unsigned const ENEMY_LAYER = 2;
unsigned const PLAYER_LAYER = 3;
unsigned const BULLET_LAYER = 4;
unsigned const EFFECT_LAYER = 5;
unsigned const FOREGROUND_LAYER = 6;
unsigned const TEXT_LAYER = 7;
unsigned const LAYER_COUNT = 8;

bool isFileExistsMsg( const std::string &fpath, const std::string funcName )
{
	std::smatch sm;
	std::regex_search( fpath, sm, std::regex( "([^\\/\\\\]*$)" ) ); 
	std::ifstream ifile( fpath );
	if( !ifile )
		MessageBoxA( NULL, (std::string( "File does not exist: " ) + sm[0].str() + "\n" 
		"When searching for:\n"+ fpath).c_str(), (funcName + ": File does not exist").c_str(), NULL );
	return (bool)ifile;
}

void CompileShaders( LPDIRECT3DDEVICE9 pdev, char const * vmainfunc, D3DSmartPtr< LPDIRECT3DVERTEXSHADER9 > & VShader, D3DSmartPtr< LPD3DXCONSTANTTABLE > & CTable, char * pmainfunc, D3DSmartPtr< LPDIRECT3DPIXELSHADER9 > & PShader )
{
	LPD3DXBUFFER pshaderbuff = NULL;
	LPD3DXBUFFER pshadererrbuff = NULL;
	if( VShader || CTable || PShader )
		MessageBox( NULL, "Shaders already initialized", "Shader Initialization Error", 0 );

	if( D3D_OK != D3DXCompileShader( DefaultShader.c_str(), DefaultShader.size(), NULL, NULL, vmainfunc, "vs_3_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, &CTable ) )
		MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Vertex Shader Compiler Error", "DX Shader Error", NULL );
	if( pshadererrbuff ) pshadererrbuff->Release();
	pdev->CreateVertexShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &VShader );
	pshaderbuff->Release();

	if( D3D_OK != D3DXCompileShader( DefaultShader.c_str(), DefaultShader.size(), NULL, NULL, pmainfunc, "ps_3_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, NULL ) )
		MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Pixel Shader Compiler Error", "DX Shader Error", NULL );
	if( pshadererrbuff ) pshadererrbuff->Release();
	pdev->CreatePixelShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &PShader );
	pshaderbuff->Release();
}

Direct3DEngine::Direct3DEngine( HWND const hWnd )
{
	////////////////////////
	/////Initialization/////
	////////////////////////

	assert( hWnd && !(LPDIRECT3DDEVICE9)d3ddev );
	RECT rec;
	GetClientRect( hWnd, &rec );

	D3DDISPLAYMODE d3ddm;
	D3DPRESENT_PARAMETERS d3dpp;
	{
		LPDIRECT3D9 d3d_ptr = Direct3DCreate9( D3D_SDK_VERSION );
		d3d = d3d_ptr;
		d3d_ptr->Release();
	}
	d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, d3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8 ) - 1, &d3ddm );
		
	d3dpp.BackBufferWidth = rec.right - rec.left;
	d3dpp.BackBufferHeight = rec.bottom - rec.top;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = (MessageBox( NULL, "Fullscreen Mode?", "FULLSCREEN/WINDOW", MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_TOPMOST ) == IDNO)? TRUE : FALSE;
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
	

	GetDevice()->Clear( 0, 0, D3DCLEAR_TARGET, ClearColor, 1.f, 0 );
	GetDevice()->Present( NULL, NULL, NULL, NULL );

	///////////////
	////shaders////
	///////////////

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

	/*Create the 2D vertex and 3D shaders*/
	CompileShaders( GetDevice(), "vs_main3D", pDefault3DVShader, pDefault3DConstable, "ps_main3D", pDefault3DPShader );
	CompileShaders( GetDevice(), "vs_main2D", pDefault2DVShader, pDefault2DConstable, "ps_main2D", pDefault2DPShader );


	if( !PipelineVertexBuffer.Buffer )
		d3ddev->CreateVertexBuffer( PipelineVertexBuffer.BufferSize = 1000, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &PipelineVertexBuffer.Buffer, NULL );

	DirectSoundCreate8( NULL, &dsound, NULL );
	GetDSound()->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );

	unsigned const BulletMgrCount = 4;
	unsigned const LayerCount = LAYER_COUNT;
	unsigned const BulletLayer = BULLET_LAYER;
	unsigned VertexBufferIdx = FetchVertexBuffer();
	for( unsigned u = 1; u < BulletMgrCount ; ++u ) AddRefVertexBuffer( VertexBufferIdx );
	vvObjects.resize( BulletMgrCount );
	GetLayers().resize( LayerCount );
	GetLayers()[ BulletLayer ].vObjMgr.resize( BulletMgrCount );
	for( unsigned u = 0; u < BulletMgrCount; ++u )
	{
		ObjMgr & mgr = GetLayers()[ BulletLayer ].vObjMgr[ u ];
		mgr.VDeclaration = GetDefaultVDeclaration();
		mgr.VShader = GetDefault2DVShader();
		mgr.PShader = GetDefault2DPShader();
		mgr.Constable = GetDefault2DConstable();
		mgr.VertexBufferIdx = VertexBufferIdx;
		mgr.VertexCount = 6;
		mgr.ObjBufferIdx = u ;
		mgr.BlendState = (BlendType)u;
		mgr.ObjFontIdx = -1;
	}
	RECT r = { 32, 16, 416, 464 };
	GetDevice()->SetScissorRect( &r );
	SetFog( 3.f, 10.f, 100.f, 30.f, 180.f );
	SetLookAtViewMatrix( D3DXVECTOR3( 0.f, 5.f, 10.f ), D3DXVECTOR3( 0.f, 0.f, 0.f ) );
	GetDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
}
LPDIRECT3DDEVICE9 & Direct3DEngine::GetDevice()
{
	return d3ddev;
}
LPDIRECT3D9 & Direct3DEngine::GetD3D()
{
	return d3d;
}
LPDIRECT3DVERTEXDECLARATION9 Direct3DEngine::GetDefaultVDeclaration() const
{
	return pDefaultVDeclaration;
}
LPDIRECT3DVERTEXSHADER9 Direct3DEngine::GetDefault3DVShader() const
{
	return pDefault3DVShader;
}
LPDIRECT3DPIXELSHADER9 Direct3DEngine::GetDefault3DPShader() const
{
	return pDefault3DPShader;
}
LPD3DXCONSTANTTABLE Direct3DEngine::GetDefault3DConstable() const
{
	return pDefault3DConstable;
}
LPDIRECT3DVERTEXSHADER9 Direct3DEngine::GetDefault2DVShader() const
{
	return pDefault2DVShader;
}
LPDIRECT3DPIXELSHADER9 Direct3DEngine::GetDefault2DPShader() const
{
	return pDefault2DPShader;
}
LPD3DXCONSTANTTABLE Direct3DEngine::GetDefault2DConstable() const
{
	return pDefault2DConstable;
}
LPDIRECTSOUND8 Direct3DEngine::GetDSound() const
{
	return dsound;
}
Direct3DEngine::vLayer_t & Direct3DEngine::GetLayers()
{
	return vLayers;
}
bool Direct3DEngine::LoadShotImage( std::string const & pathname )
{
	ShotImagePath = pathname;
	if( !isFileExistsMsg( pathname, "LoadShotImage" ) )
		return false;

	LoadTexture( pathname );
	for( unsigned u = 0; u < 4; ++u )
		GetLayers()[ BULLET_LAYER ].vObjMgr[ u ].pTexture = GetTexture( pathname ) ;
	return true;
}
unsigned Direct3DEngine::CreateObjHandle()
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
unsigned Direct3DEngine::CreateObject( ObjType type )
{
	unsigned Result;
	unsigned Layer;
	ObjMgr * objMgr;

	switch( type )
	{
	case ObjShot: Layer = BULLET_LAYER; break;
	case ObjEffect: Layer = EFFECT_LAYER; break;
	case ObjPlayer: Layer = PLAYER_LAYER; break;
	case ObjFont: Layer = TEXT_LAYER; break;
	default:
		abort();
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
		if( type != ObjShot && type != ObjFont )
		{
			handle.VertexBuffer = FetchVertexBuffer();
		}
		else
			handle.VertexBuffer = -1;

		handle.Layer = Layer;
		handle.RefCount = 1;
		handle.MgrIdx = GetLayers()[ Layer ].vObjMgr.size();
		handle.ObjVectorIdx = vvObjects[ handle.ObjVector ].size();
		handle.ObjFontIdx = ( (type == ObjFont )? CreateFontObject() : -1 );
		handle.Type = type;

		if( type != ObjShot )
		{
			ObjMgr * objMgr = &(*GetLayers()[ Layer ].vObjMgr.insert( GetLayers()[ Layer ].vObjMgr.end(), ObjMgr() ) );

			switch( type )
			{
			case ObjEffect:
			default:
				objMgr->VShader = GetDefault2DVShader();
				objMgr->PShader = GetDefault2DPShader();
				objMgr->Constable = GetDefault2DConstable();
				break;
			}
			objMgr->VDeclaration = pDefaultVDeclaration;
			objMgr->VertexBufferIdx = handle.VertexBuffer;
			objMgr->ObjBufferIdx = handle.ObjVector;
			objMgr->BlendState = BlendAlpha;
			objMgr->ObjFontIdx = handle.ObjFontIdx;
		}

		vvObjects[ handle.ObjVector ].resize( 1 + handle.ObjVectorIdx );
		Object & obj = *GetObject( Result );
		obj.FlagCollidable( 0 );
		obj.FlagCollision( 0 );
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
		obj.Radius = 4.f;
	}

	return Result;
}
void Direct3DEngine::AddRefObjHandle( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
		++vObjHandles[ HandleIdx ].RefCount;
}
void Direct3DEngine::ReleaseObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
		if( !--vObjHandles[ HandleIdx ].RefCount )
		{
			DestroyObject( HandleIdx );
			vObjHandlesGC.push_back( HandleIdx );
		}
}
void Direct3DEngine::DestroyObject( unsigned HandleIdx ) // keeps handle intact, w/clear
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
					vvObjects[ handle.ObjVector ].resize( 0 );
					vvObjectsGC.push_back( handle.ObjVector );
					Layer.vObjMgr.erase( objmgr_it );
					if( CheckValidIdx( handle.VertexBuffer ) )
						DisposeVertexBuffer( handle.VertexBuffer );
					if( CheckValidIdx( handle.ObjFontIdx ) )
						vFontObjectsGC.push_back( handle.ObjFontIdx );
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
Object * Direct3DEngine::GetObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.ObjVectorIdx ) && CheckValidIdx( handle.ObjVector ) )
			return &vvObjects[ handle.ObjVector ][ handle.ObjVectorIdx ];
	}
	return 0;
}
ObjMgr * Direct3DEngine::GetObjMgr( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) && CheckValidIdx( handle.Layer ) )
			return &GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ];
	}
	return 0;
}
void Direct3DEngine::LoadTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it == mapTextures.end() )
	{
		if( !isFileExistsMsg( pathname, "LoadTexture" ) )
			return;
		mapTextures[ pathname ];
		D3DXCreateTextureFromFile( d3ddev, pathname.c_str(), &mapTextures[ pathname ] );
	}
}
LPDIRECT3DTEXTURE9 Direct3DEngine::GetTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it == mapTextures.end() )
		return 0;
	return it->second;
}
void Direct3DEngine::DeleteTexture( std::string const & pathname )
{
	auto it = mapTextures.find( pathname );
	if( it != mapTextures.end() )
		mapTextures.erase( it );
}
void Direct3DEngine::LoadSound( std::string const & pathname )
{
	if( mapSoundEffects.find( pathname ) != mapSoundEffects.end() )
		return;
	else if( !isFileExistsMsg( pathname, "LoadSound" ) )
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

	if( (( WaveFileHeader.chunkId[ 0 ] != 'R' ) || ( WaveFileHeader.chunkId[ 1 ] != 'I' ) || 
		( WaveFileHeader.chunkId[ 2 ] != 'F' ) || ( WaveFileHeader.chunkId[ 3 ] != 'F' ))
			&&
		(( WaveFileHeader.format[ 0 ] != 'W' ) || ( WaveFileHeader.format[ 1 ] != 'A' ) || 
		( WaveFileHeader.format[ 2 ] != 'V' ) || ( WaveFileHeader.format[ 3 ] != 'E' ))
			&&
		(( WaveFileHeader.subChunkId[ 0 ] != 'f' ) || ( WaveFileHeader.subChunkId[ 1 ] != 'm' ) || 
		( WaveFileHeader.subChunkId[ 2 ] != 't' ) || ( WaveFileHeader.subChunkId[ 3 ] != ' ' ))
			&&
		(( WaveFileHeader.dataChunkId[ 0 ] != 'd' ) || ( WaveFileHeader.dataChunkId[ 1 ] != 'a' ) || 
		( WaveFileHeader.dataChunkId[ 2 ] != 't' ) || ( WaveFileHeader.dataChunkId[ 3 ] != 'a' ))
	  )
	{
		SoundFile.close();
		mapSoundEffects.erase( pathname );
		return;
	}

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
void Direct3DEngine::StopSound( std::string const & pathname )
{
	auto it = mapSoundEffects.find( pathname );
	if( it == mapSoundEffects.end() )
		return;

	LPDIRECTSOUNDBUFFER8 buffer = it->second;

	buffer->SetCurrentPosition( 0 );
	buffer->SetVolume( DSBVOLUME_MAX );
	buffer->Stop();
}
void Direct3DEngine::PlaySound( std::string const & pathname )
{
	auto it = mapSoundEffects.find( pathname );
	if( it == mapSoundEffects.end() )
		return;

	LPDIRECTSOUNDBUFFER8 buffer = it->second;

	buffer->SetCurrentPosition( 0 );
	buffer->SetVolume( DSBVOLUME_MAX );
	buffer->Play( 0, 0, 0 );
}
void Direct3DEngine::DeleteSound( std::string const & pathname )
{
	auto it = mapSoundEffects.find( pathname );
	if( it != mapSoundEffects.end() )
		mapSoundEffects.erase( it );
}
unsigned Direct3DEngine::CreateFontObject()
{
	unsigned res;
	if( vFontObjectsGC.size() )
	{
		res = vFontObjectsGC.back();
		vFontObjectsGC.pop_back();
	}
	else
	{
		res = vFontObjects.size();
		vFontObjects.resize( 1 + res );
	}
	RECT r = { 0, 0, 640, 480 };
	D3DSmartPtr< LPD3DXFONT > pFont;
	D3DXCreateFont( GetDevice(), 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &pFont );
	auto & Result= vFontObjects[ res ];
	Result.pFont = pFont;
	Result.Color = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	Result.Format = DT_TOP | DT_LEFT;
	Result.Rect = r;
	Result.String = "Hello World!";
	return res;
}
FontObject * Direct3DEngine::GetFontObject( unsigned HandleIdx )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.ObjFontIdx ) )
			return &vFontObjects[ handle.ObjFontIdx ];
	}
	return 0;
}
unsigned Direct3DEngine::CreateShot( ULONG GraphicID )
{
	unsigned Result;
	Result = CreateObjHandle();
	ObjHandle & handle = vObjHandles[ Result ];
	handle.Layer = BULLET_LAYER;
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
void Direct3DEngine::CreateShotData( unsigned ID, BlendType blend, RECT const & rect, D3DCOLOR color, DWORD flags, vector< vector< float > > const & AnimationData )
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
			shot_data.Radius = (AnimationData[ i ][ 3 ] - AnimationData[ i ][ 1 ]) / 4.f;
			RECT r2 = { (LONG)AnimationData[ i ][ 1 ], (LONG)AnimationData[ i ][ 2 ], (LONG)AnimationData[ i ][ 3 ], (LONG)AnimationData[ i ][ 4 ] };
			r = r2;
		}
		else
		{
			shot_data.AnimationTime = -1;
			shot_data.Radius = (float)(rect.right - rect.left) / 4.f;
			r = rect;
		}
		PushQuadShotBuffer( r, color );
		Bullet_Templates.push_back( shot_data );
	}while( ++i < AnimationData.size() );
}
void Direct3DEngine::PushQuadShotBuffer( RECT const Quad, D3DCOLOR const Color )
{
	D3DSURFACE_DESC SurfaceDesc;
	GetTexture( ShotImagePath )->GetLevelDesc( 0, &SurfaceDesc );
	Vertex v[ 6 ] = 
	{
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, -(float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.top / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( (float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.right / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color },
		{ D3DXVECTOR3( -(float)(Quad.right - Quad.left)/2, (float)(Quad.bottom - Quad.top) / 2, 0.f ), D3DXVECTOR2( (float)Quad.left / (float)SurfaceDesc.Width, (float)Quad.bottom / (float)SurfaceDesc.Height ), Color }
	};
	for( unsigned u = 0; u < 6; ++u ) GetVertexBuffer( 0 ).push_back( v[ u ] );
}

//ObjEffect functions
void Direct3DEngine::ObjEffect_CreateVertex( unsigned HandleIdx, ULONG VertexCount )
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
void Direct3DEngine::ObjEffect_SetVertexXYZ( unsigned HandleIdx, ULONG VIndex,  D3DXVECTOR3 Posxyz )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].pos = Posxyz;
	}
}
void Direct3DEngine::ObjEffect_SetVertexUV( unsigned HandleIdx, ULONG VIndex, D3DXVECTOR2 Posuv )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].tex = Posuv;
	}
}
void Direct3DEngine::ObjEffect_SetVertexColor( unsigned HandleIdx, ULONG VIndex, D3DCOLOR Color )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.VertexBuffer ) )
			GetVertexBuffer( handle.VertexBuffer )[ VIndex ].color = Color;
	}
}
void Direct3DEngine::ObjEffect_SetRenderState( unsigned HandleIdx, BlendType BlendState )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].BlendState = BlendState;
	}
}
void Direct3DEngine::ObjEffect_SetPrimitiveType( unsigned HandleIdx, D3DPRIMITIVETYPE PrimitiveType )
{
	if( CheckValidIdx( HandleIdx ) )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.MgrIdx ) )
			GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].PrimitiveType = PrimitiveType;
	}
}
void Direct3DEngine::ObjEffect_SetLayer( unsigned HandleIdx, ULONG Layer )
{
	if( CheckValidIdx( HandleIdx ) && Layer < LAYER_COUNT )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( CheckValidIdx( handle.Layer ) && CheckValidIdx( handle.Layer ) && handle.Layer != Layer)
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
			if( Layer == BACKGROUND_LAYER )
			{
				GetLayers()[ Layer ].vObjMgr.back().PShader = GetDefault3DPShader();
				GetLayers()[ Layer ].vObjMgr.back().VShader = GetDefault3DVShader();
			}
			else
			{
				GetLayers()[ Layer ].vObjMgr.back().PShader = GetDefault2DPShader();
				GetLayers()[ Layer ].vObjMgr.back().VShader = GetDefault2DVShader();
			}
		}
	}
}

//ObjShot functions
void Direct3DEngine::ObjShot_SetGraphic( unsigned HandleIdx, ULONG ID )
{
	Object * pObj = GetObject( HandleIdx );
	if( pObj && ID < Bullet_TemplateOffsets.size() && pObj->FlagBullet( -1 ) )
	{
		// create a new shot -> destroy old shot -> swap handle info (except refcount) -> release old handle
		ULONG BufferOffset = Bullet_TemplateOffsets[ ID ];
		ShotData const & shot_data = Bullet_Templates[ BufferOffset ];
		pObj->SetShotDataParams( shot_data, BufferOffset );
		unsigned ResHandle = CreateShot( ID );
		*GetObject( ResHandle ) = *GetObject( HandleIdx );
		DestroyObject( HandleIdx );
		auto refCountHan = vObjHandles[ HandleIdx ].RefCount;
		auto refCountRes = vObjHandles[ ResHandle ].RefCount;
		ObjHandle tmp = vObjHandles[ HandleIdx ];
		vObjHandles[ HandleIdx ] = vObjHandles[ ResHandle ];
		vObjHandles[ HandleIdx ].RefCount = refCountHan;
		vObjHandles[ ResHandle ] = tmp;
		vObjHandles[ ResHandle ].RefCount = refCountRes;
		ReleaseObject( ResHandle );
	}
}

//ObjFont functions
void Direct3DEngine::ObjFont_SetSize( unsigned HandleIdx, ULONG Size )
{
	FontObject * fobj = GetFontObject( HandleIdx );
	if( !fobj )
		return;
	D3DXFONT_DESC desc;
	fobj->pFont->GetDesc( &desc );
	if( desc.Height != Size )
	{
		desc.Height = Size;
		desc.Width = 0;
		fobj->pFont->Release();
		D3DXCreateFontIndirect( GetDevice(), &desc, &(fobj->pFont) );
	}
}
void Direct3DEngine::ObjFont_SetFaceName( unsigned HandleIdx, std::string const & FaceName )
{
	FontObject * fobj = GetFontObject( HandleIdx );
	if( !fobj )
		return;
	D3DXFONT_DESC desc;
	fobj->pFont->GetDesc( &desc );
	if( FaceName != desc.FaceName && FaceName.size() <= sizeof( desc.FaceName ) )
	{
		memset( &desc.FaceName, 0, sizeof( desc.FaceName ) );
		memcpy( &desc.FaceName, FaceName.c_str(), FaceName.size() );
		fobj->pFont->Release();
		D3DXCreateFontIndirect( GetDevice(), &desc, &(fobj->pFont) );
	}
}
void FontObject::SetAlignmentX( int X )
{
	Format = (Format & ~DT_LEFT) & (Format & ~DT_RIGHT) & (Format & ~DT_CENTER);
	switch( X )
	{
		case -1: Format |= DT_LEFT; break;
		case 0: Format |= DT_CENTER; break;
		case 1: Format |= DT_RIGHT; break;
	}
}
void FontObject::SetAlignmentY( int Y )
{
	Format = (Format & ~DT_TOP) & (Format & ~DT_VCENTER) & (Format & ~(DT_BOTTOM | DT_SINGLELINE) );
	switch( Y )
	{
		case -1: Format |= DT_TOP; break;
		case 0: Format |= DT_VCENTER; break;
		case 1: Format |= DT_BOTTOM | DT_SINGLELINE;
	}
}

//misc
unsigned Direct3DEngine::FetchVertexBuffer()
{
	unsigned res;
	if( VertexBuffersGC.size() )
	{
		res = VertexBuffersGC.back();
		VertexBuffersGC.pop_back();
	}
	else
	{
		res = VertexBuffers.size();
		VertexBuffers.resize( res + 1 );
	}
	VertexBuffers[ res ].RefCount = 1;
	return res;
}
vector< Vertex > & Direct3DEngine::GetVertexBuffer( unsigned Idx )
{
	return VertexBuffers[ Idx ].VertexBuffer;
}
void Direct3DEngine::AddRefVertexBuffer( unsigned Idx )
{
	++VertexBuffers[ Idx ].RefCount;
}
void Direct3DEngine::DisposeVertexBuffer( unsigned Idx )
{
	if( VertexBuffers[ Idx ].RefCount == 0 )
		assert( 0 );
	if( !--VertexBuffers[ Idx ].RefCount )
	{
		VertexBuffers[ Idx ].VertexBuffer.resize( 0 );
		VertexBuffersGC.push_back( Idx );
	}
}
void Direct3DEngine::DrawObjects_V2()
{
	D3DXMATRIX world, view, proj;
	D3DXMatrixIdentity( &world );
	D3DXMatrixOrthoLH( &proj, 640.f, -480.f, 0.f, 100.f );
	D3DXMatrixLookAtLH( &view, &D3DXVECTOR3( 0, 0, -1.f ), &D3DXVECTOR3( 0, 0, 0 ), &D3DXVECTOR3( 0, 1, 0 ) );
	D3DXMatrixTranslation( &world, -320.f - 0.5f, -240.f - 0.5f, 0.f );

	for( auto L = GetLayers().begin(); L < GetLayers().end(); ++L )
	{
		/*constant buffers do not own their own constants, they use slots and the names are symbolic*/
		if( L - GetLayers().begin() == BACKGROUND_LAYER )
				pDefault3DConstable->SetMatrix( GetDevice(), "WorldViewProjMat", &( WorldMatrix * ViewMatrix * ProjectionMatrix ) ),
				pDefault3DConstable->SetMatrix( GetDevice(), "WorldViewMat", &( WorldMatrix * ViewMatrix ) );
		else
				pDefault2DConstable->SetMatrix( GetDevice(), "WorldViewProjMat", &( world * view * proj ) );

		GetDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, (L - GetLayers().begin() < ENEMY_LAYER || L - GetLayers().begin() > EFFECT_LAYER )? FALSE : TRUE );
		GetDevice()->SetRenderState( D3DRS_ZENABLE, L - GetLayers().begin() == BACKGROUND_LAYER? TRUE : FALSE );

		for( auto objMgr = L->vObjMgr.begin(); objMgr < L->vObjMgr.end(); ++objMgr )
		{
			DrawObjects( *objMgr );
			AdvanceObjects( *objMgr );
		}
	}
}
void Direct3DEngine::DrawObjects( ObjMgr const & objMgr )
{
	if( CheckValidIdx( objMgr.ObjFontIdx ) )
	{
		FontObject & FontObj = vFontObjects[ objMgr.ObjFontIdx ];
		FontObj.pFont->DrawTextA( NULL, FontObj.String.c_str(), -1, &FontObj.Rect, FontObj.Format, FontObj.Color );
	}

	if( !(CheckValidIdx( objMgr.ObjBufferIdx ) && CheckValidIdx( objMgr.VertexBufferIdx )) )
		return;

	auto & vObjects = vvObjects[ objMgr.ObjBufferIdx ];
	ULONG min_buffersize = vObjects.size() * objMgr.VertexCount * sizeof( Vertex );
	if( PipelineVertexBuffer.BufferSize < min_buffersize )
	{
		PipelineVertexBuffer.Buffer->Release();
		d3ddev->CreateVertexBuffer( PipelineVertexBuffer.BufferSize = min_buffersize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &PipelineVertexBuffer.Buffer, NULL );
	}

	Vertex * ptr = NULL;
	PipelineVertexBuffer.Buffer->Lock( 0, min_buffersize, (void**)&ptr, 0 );
	vector< Vertex > & vb = GetVertexBuffer( objMgr.VertexBufferIdx );

	for( auto pObj = vObjects.begin(); pObj < vObjects.end(); ++pObj )
	{
		D3DXMATRIX mat;
		D3DXMatrixTransformation( &mat, NULL, NULL, &pObj->scale, NULL, &pObj->orient, pObj->FlagPixelPerfect( -1 ) ?
			&D3DXVECTOR3( floor( pObj->position.x + 0.5f), floor( pObj->position.y + 0.5f), floor( pObj->position.z + 0.5f ) ) : &pObj->position );

		Vertex * src = (vb.size() ? &vb[ pObj->VertexOffset ] : NULL );
		for( ULONG v = 0; v < objMgr.VertexCount; ++v )
		{
			*ptr++ = *src++;
			D3DXVec3TransformCoord( &(ptr-1)->pos, &(ptr-1)->pos, &mat );
		}
	}

	PipelineVertexBuffer.Buffer->Unlock();

	GetDevice()->SetTexture( 0, objMgr.pTexture );
	GetDevice()->SetVertexDeclaration( objMgr.VDeclaration );
	GetDevice()->SetVertexShader( objMgr.VShader );
	GetDevice()->SetPixelShader( objMgr.PShader );
	GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	
	switch( objMgr.BlendState )
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
	DWORD VCount = vObjects.size() * objMgr.VertexCount;

	if( VCount )
		if( D3D_OK != GetDevice()->DrawPrimitive( objMgr.PrimitiveType, 0, objMgr.PrimitiveType == D3DPT_TRIANGLELIST ? VCount / 3 :
			objMgr.PrimitiveType == D3DPT_TRIANGLESTRIP || objMgr.PrimitiveType == D3DPT_TRIANGLEFAN ? VCount - 2 : 0 ) ) MessageBox( 0, "", "", 0 );

}
void Direct3DEngine::AdvanceObjects( ObjMgr const & objMgr )
{
	auto & vObjects = vvObjects[ objMgr.ObjBufferIdx ];
	for( auto pObj = vObjects.begin(); pObj < vObjects.end(); ++pObj )
		pObj->Advance();
}
void Direct3DEngine::UpdateObjectCollisions()
{
	auto CollisionCheck = []( vector< Object > & LayerA, vector< Object > & LayerB )
	{
		auto BoundCircleCollisionCheck = []( Object * obj1, Object * obj2 )
		{
			float x = obj2->position.x - obj1->position.x;
			float y = obj2->position.y - obj1->position.y;
			if( obj1->FlagCollidable( -1 ) && obj2->FlagCollidable( -1 ) && obj1->Radius + obj2->Radius <= sqrt( x * x + y * y ) )
			{
				obj1->FlagCollision( 1 );
				obj2->FlagCollision( 1 );
				//grazing?
			}
		};
		auto BoundBoxCollisionCheck = []( Object * obj1, Object * obj2 )
		{
			if( !( obj1->position.y + obj1->Radius < obj2->position.y - obj2->Radius ||
				obj1->position.y - obj1->Radius > obj2->position.y + obj2->Radius ||
				obj1->position.x + obj1->Radius < obj2->position.x - obj2->Radius ||
				obj1->position.x - obj1->Radius > obj2->position.x + obj2->Radius ) )
			{
				obj1->FlagCollision( 1 );
				obj2->FlagCollision( 1 );
				//grazing?
			}
		};
		for( unsigned u = 0; u < LayerA.size(); ++u )
		{
			for( unsigned j = 0; j < LayerB.size(); ++j )
			{
				BoundBoxCollisionCheck( &LayerA[ u ], &LayerB[ j ] );
			}
		}
	};
	for( unsigned u = 0; u < vvObjects.size(); ++u )
	{
		auto & vvec = vvObjects[ u ];
		for( unsigned v = 0; v < vvec.size(); ++v )
			vvec[ v ].FlagCollision( 0 );
	}
	
	auto & EnemyLayer = GetLayers()[ ENEMY_LAYER ];
	auto & PlayerLayer = GetLayers()[ PLAYER_LAYER ];
	auto & BulletLayer = GetLayers()[ BULLET_LAYER ];
	
	for( unsigned u = 0; u < PlayerLayer.vObjMgr.size(); ++u )
	{	
		auto & PlayerObjects = vvObjects[ PlayerLayer.vObjMgr[ u ].ObjBufferIdx ];
		for( unsigned j = 0; j < EnemyLayer.vObjMgr.size(); ++j )
		{
			auto & EnemyObjects = vvObjects[ EnemyLayer.vObjMgr[ j ].ObjBufferIdx ];
			CollisionCheck( PlayerObjects, EnemyObjects );
		}
		for( unsigned j = 0; j < BulletLayer.vObjMgr.size(); ++j )
		{
			auto & BulletObjects = vvObjects[ BulletLayer.vObjMgr[ j ].ObjBufferIdx ];
			CollisionCheck( PlayerObjects, BulletObjects );
		}
	}
}


//camera
void Direct3DEngine::SetLookAtViewMatrix( D3DXVECTOR3 const & eye, D3DXVECTOR3 const & at )
{
	D3DXMatrixLookAtLH( &ViewMatrix, &eye, &at, &D3DXVECTOR3( 0.f, 1.f, 0.f ) );
}
void Direct3DEngine::SetFog( float fognear, float fogfar, float fred, float fgreen, float fblue )
{
	float fog[ 4 ] = { fred / 255.f, fgreen / 255.f, fblue / 255.f, 1.f };
	float fognf[ 2 ] = { fognear, fogfar };
	ClearColor = D3DCOLOR_XRGB( (UCHAR)fred, (UCHAR)fgreen, (UCHAR)fblue );
	GetDefault3DConstable()->SetFloatArray( GetDevice(), "FogRGBA", fog, 4 );
	GetDefault3DConstable()->SetFloatArray( GetDevice(), "FogNF", fognf, 2 );
}

void Direct3DEngine::ToggleWindowed()
{
	D3DPRESENT_PARAMETERS d3dpp;
	D3DSmartPtr< LPDIRECT3DSWAPCHAIN9 > psc;
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
	GetDevice()->SetVertexShader( GetDefault3DVShader() );
	GetDevice()->SetPixelShader( GetDefault3DPShader() );

	RECT rec = { 0, 0, 640, 480 };
	AdjustWindowRect( &rec, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, FALSE );
	if( d3dpp.Windowed )
		MoveWindow( d3dpp.hDeviceWindow, 100, 100, rec.right - rec.left, rec.bottom - rec.top, FALSE );
}
void Direct3DEngine::RenderFrame( MSG const msg )
{
	GetDevice()->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, ClearColor, 1.f, 0 );
	GetDevice()->BeginScene();
	DrawGridTerrain( 100, 100, 1.f );
	DrawObjects_V2();
	DrawFPS();
	GetDevice()->EndScene();
	GetDevice()->Present( NULL, NULL, NULL, NULL );
}
void Direct3DEngine::GenerateGridTerrain( unsigned Rows, unsigned Columns, float Spacing )
{
	D3DSmartPtr< LPDIRECT3DVERTEXBUFFER9 > pvb;
	GetDevice()->CreateVertexBuffer( 2 * sizeof( Vertex ) * 2 * ( Rows * Columns), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pvb, NULL );

	void * ptr;
	Vertex * pverts;
	pvb->Lock( 0, 0, &ptr, 0 );
	pverts = (Vertex*)ptr;

	float FarLeft = Spacing * (float)Columns / -2.f;
	float FarBottom = Spacing * (float)Rows / -2.f;
	for( unsigned i = 0; i < Rows; ++i )
	{
		for( unsigned j = 0; j < Columns; ++j )
		{
			Vertex v1 = { D3DXVECTOR3( FarLeft + (float)j, 0.f, FarBottom + (float)i ), D3DXVECTOR2( 0, 0 ), D3DCOLOR_ARGB( 255, 255, 255, 255 ) };
			Vertex v2 = v1; v2.pos.x += Spacing;
			Vertex v3 = v1; v3.pos.z += Spacing;
			*pverts++ = v1;
			*pverts++ = v2;
			*pverts++ = v1;
			*pverts++ = v3;
		}
	}
	pvb->Unlock();
	pVBGridTerrain = pvb;
}
void Direct3DEngine::DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing )
{
	static unsigned R = -1, C = -1;
	static float S = -1;
	if( !( R == Rows || C == Columns || S == Spacing) )
		GenerateGridTerrain( R = Rows, C = Columns, S = Spacing );
	D3DXMATRIX mat;
	D3DXMatrixIdentity( &mat );
	GetDevice()->SetVertexDeclaration( GetDefaultVDeclaration() );
	GetDevice()->SetStreamSource( 0, pVBGridTerrain, 0, sizeof( Vertex ) );
	GetDefault3DConstable()->SetMatrix( GetDevice(), "WorldViewProjMat", &( WorldMatrix * ViewMatrix * ProjectionMatrix ) );
	GetDefault3DConstable()->SetMatrix( GetDevice(), "WorldViewMat", &( WorldMatrix * ViewMatrix ) );
	GetDevice()->SetVertexShader( GetDefault3DVShader() );
	GetDevice()->SetPixelShader( GetDefault3DPShader() );
	GetDevice()->SetTexture( 0, NULL );
	GetDevice()->DrawPrimitive( D3DPT_LINELIST, 0, 2 * (Rows * Columns) );
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
	D3DXCreateFont( GetDevice(), 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &pFont );
	RECT rec = { 0, 0, 640, 480 };
	std::stringstream ss;
	ss << FrameShow;
	pFont->DrawTextA( NULL, ss.str().c_str(), -1, &rec, DT_BOTTOM | DT_SINGLELINE | DT_RIGHT, D3DCOLOR_ARGB( 100, 255, 255, 255 ) );
	pFont->Release();
	++Frame;
	
}