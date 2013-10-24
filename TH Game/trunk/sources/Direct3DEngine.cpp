#include <Direct3DEngine.hpp>
#include <ObjMgr.hpp>
#include <sstream>

Direct3DEngine::Battery::Battery() : pDefaultVDeclaration( NULL ), pDefault3DVShader( NULL ), pDefault3DPShader( NULL ), pDefaultConstable( NULL )
{
}
Direct3DEngine::Direct3DEngine() : d3d( Direct3DCreate9( D3D_SDK_VERSION ) ), d3ddev( NULL ), inventory( Battery() )
{
}
Direct3DEngine::Direct3DEngine( Direct3DEngine const & source )
{

	if( source.d3d ) source.d3d->AddRef();
	if( source.d3ddev ) source.d3ddev->AddRef();

	d3d = source.d3d;
	d3ddev = source.d3ddev;
}
Direct3DEngine & Direct3DEngine::operator = ( Direct3DEngine const & source )
{
	D3dRelease( d3d );
	D3dRelease( d3ddev );
	source.d3d->AddRef();
	source.d3ddev->AddRef();
	d3d = source.d3d;
	d3ddev = source.d3ddev;
	return *this;
}
Direct3DEngine::~Direct3DEngine()
{
	if( inventory.pDefault3DPShader ) inventory.pDefault3DPShader->Release();
	if( inventory.pDefault3DVShader ) inventory.pDefault3DVShader->Release();
	if( inventory.pDefaultConstable ) inventory.pDefaultConstable->Release();
	if( inventory.pDefaultVDeclaration ) inventory.pDefaultVDeclaration->Release();
}

void Direct3DEngine::InitEng( HWND hWnd, bool windowed )
{
	if( hWnd && !d3ddev )
	{
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
		d3dpp.Windowed = windowed;
		d3dpp.EnableAutoDepthStencil= TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.FullScreen_RefreshRateInHz = windowed? 0 : d3ddm.RefreshRate;
		d3dpp.Flags = 0;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		
		d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd , D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev );
		
		D3DXMatrixIdentity( &CamSetting.WorldMat );
		D3DXMatrixLookAtLH( &CamSetting.ViewMat, &D3DXVECTOR3(0, 1,-10), &D3DXVECTOR3(0,1,0), &D3DXVECTOR3(0,1,0) );
		D3DXMatrixPerspectiveFovLH( &CamSetting.ProjMat, D3DXToRadian(45), 640.f/480.f, 1.0f, 100.0f );
		
		d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

		InitBattery();
		InitLayers();

		d3ddev->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
		d3ddev->Present( NULL, NULL, NULL, NULL );
	}
}
void Direct3DEngine::InitBattery()
{
	LPD3DXBUFFER pshaderbuff = NULL;
	LPD3DXBUFFER pshadererrbuff = NULL;
	if( !inventory.pDefaultVDeclaration )
	{
		D3DVERTEXELEMENT9 ve[] = 
		{
			{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};
		d3ddev->CreateVertexDeclaration( ve, &inventory.pDefaultVDeclaration );
	}

	if( !inventory.pDefault3DVShader && !inventory.pDefaultConstable )
	{
		if( D3D_OK != D3DXCompileShaderFromFile( "Default3D.vs", NULL, NULL, "vs_main", "vs_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, &inventory.pDefaultConstable ) )
			MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Vertex Shader Compiler Error", "DX Shader Error", NULL );
		d3ddev->CreateVertexShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &inventory.pDefault3DVShader );
		pshaderbuff->Release();
		if(pshadererrbuff ) pshadererrbuff->Release();
	}
	if( !inventory.pDefault3DPShader )
	{
		if( D3D_OK != D3DXCompileShaderFromFile( "Default3D.ps", NULL, NULL, "ps_main", "ps_2_0", D3DXSHADER_DEBUG, &pshaderbuff, &pshadererrbuff, NULL ) )
			MessageBox( NULL, pshadererrbuff? (LPCSTR)pshadererrbuff->GetBufferPointer() : "Pixel Shader Compiler Error", "DX Shader Error", NULL );	
		d3ddev->CreatePixelShader( (DWORD const*)pshaderbuff->GetBufferPointer(), &inventory.pDefault3DPShader );
		pshaderbuff->Release();
		if(pshadererrbuff ) pshadererrbuff->Release();
	}
}
void Direct3DEngine::InitLayers()
{
	//0 - background
	//1 - 3D
	//2 - enemy boss
	//3 - player
	//4 - enemies
	//5 - bullets
	//6 - effects
	//7 - sprites/text
	//8 - foreground
	inventory.vLayers.resize( 8 );
}
void Direct3DEngine::ToggleWindowed()
{
	D3DPRESENT_PARAMETERS d3dpp;
	LPDIRECT3DSWAPCHAIN9 psc;
	D3DDISPLAYMODE d3ddm;

	d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, d3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8 ) - 1, &d3ddm );
	d3ddev->GetSwapChain( 0, &psc );
	psc->GetPresentParameters( &d3dpp );
	d3dpp.Windowed = (BOOL)!d3dpp.Windowed;
	d3dpp.FullScreen_RefreshRateInHz = (d3dpp.Windowed? 0 : d3ddm.RefreshRate);
	d3ddev->Reset( &d3dpp );
	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	d3ddev->SetVertexShader( inventory.pDefault3DVShader );
	d3ddev->SetPixelShader( inventory.pDefault3DPShader );

	RECT rec = { 0, 0, 640, 480 };
	AdjustWindowRect( &rec, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, FALSE );
	if( d3dpp.Windowed )
		MoveWindow( d3dpp.hDeviceWindow, 100, 100, rec.right - rec.left, rec.bottom - rec.top, FALSE );
	
}
void Direct3DEngine::LoadTexture( std::string const pathname )
{
	auto it = inventory.mapTextures.find( pathname );
	if( it == inventory.mapTextures.end() )
	{
		inventory.mapTextures[ pathname ];
		D3DXCreateTextureFromFile( d3ddev, pathname.c_str(), &inventory.mapTextures[ pathname ] );
	}
}
LPDIRECT3DTEXTURE9 Direct3DEngine::GetTexture( std::string const & pathname )
{
	auto it = inventory.mapTextures.find( pathname );
	if( it == inventory.mapTextures.end() )
		return 0;
	return it->second;
}
void Direct3DEngine::DeleteTexture( std::string const pathname )
{
	auto it = inventory.mapTextures.find( pathname );
	if( it != inventory.mapTextures.end() )
	{
		it->second->Release();
		inventory.mapTextures.erase( it );
	}
}
void Direct3DEngine::RenderFrame( MSG const msg )
{
	d3ddev->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
	d3ddev->BeginScene();
	DrawGridTerrain( 1000, 1000, 1.f );
	DrawTexture();
	DrawFPS();
	DrawObjects();
	d3ddev->EndScene();
	d3ddev->Present( NULL, NULL, NULL, NULL );
	ProcUserInput( msg );
}
void Direct3DEngine::DrawGridTerrain( unsigned Rows, unsigned Columns, float Spacing )
{
	d3ddev->SetTransform( D3DTS_WORLD, &CamSetting.WorldMat );
	d3ddev->SetTransform( D3DTS_VIEW, &CamSetting.ViewMat );
	d3ddev->SetTransform( D3DTS_PROJECTION, &CamSetting.ProjMat );
	LPDIRECT3DVERTEXBUFFER9 pvb;
	d3ddev->CreateVertexBuffer( 2 * sizeof( Vertex ) * ( Rows + Columns), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pvb, NULL );

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
	d3ddev->SetVertexDeclaration( inventory.pDefaultVDeclaration );
	d3ddev->SetStreamSource( 0, pvb, 0, sizeof( Vertex ) );
	inventory.pDefaultConstable->SetMatrix( d3ddev, "WorldViewProjMat", &( CamSetting.WorldMat * CamSetting.ViewMat * CamSetting.ProjMat ) );
	d3ddev->SetVertexShader( inventory.pDefault3DVShader );
	d3ddev->SetPixelShader( inventory.pDefault3DPShader );
	d3ddev->SetTexture( 0, NULL );
	d3ddev->DrawPrimitive( D3DPT_LINELIST, 0, Rows + Columns );
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
	d3ddev->CreateVertexBuffer( sizeof( verts ), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pvb, 0 );
	
	void * ptr;
	pvb->Lock( 0, NULL, &ptr, D3DLOCK_DISCARD );
	memcpy( ptr, verts, sizeof( verts ) );
	pvb->Unlock();

	D3DXMATRIX world, view, proj;
	D3DXMatrixIdentity( &world );
	D3DXMatrixOrthoLH( &proj, 640.f, -480.f, 0.f, 100.f );
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0,0,-1.f), &D3DXVECTOR3(0,0,0 ), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixTranslation( &world, -640.f/4 - 0.5f, 0.f - 0.5f, 0.f );
	inventory.pDefaultConstable->SetMatrix( d3ddev, "WorldViewProjMat", &(world*view*proj) );
	//d3ddev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	d3ddev->SetTexture( 0, inventory.mapTextures[ csTexture ] );
	d3ddev->SetStreamSource( 0, pvb, 0, sizeof( Vertex ) );
	d3ddev->SetVertexDeclaration( inventory.pDefaultVDeclaration );
	d3ddev->SetVertexShader( inventory.pDefault3DVShader );
	d3ddev->SetPixelShader( inventory.pDefault3DPShader );
	d3ddev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	d3ddev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	d3ddev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	d3ddev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
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
	D3DXCreateFont( d3ddev, 30, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Lucida", &pFont );
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
	inventory.pDefaultConstable->SetMatrix( d3ddev, "WorldViewProjMat", &(world*view*proj) );
	for( auto L = inventory.vLayers.begin(); L != inventory.vLayers.end(); ++L )
	{
		for( auto Obj = L->vObjMgr.begin(); Obj != L->vObjMgr.end(); ++Obj )
			Obj->AdvanceTransformedDraw( this );
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
		CamSetting.ViewMat *= DisplaceMat;
		
	}
}
LPDIRECT3DDEVICE9 Direct3DEngine::GetDevice()
{
	return d3ddev;
}
template<typename T>
void Direct3DEngine::D3dRelease( T *& RefPtr )
{
	if( RefPtr )
	{
		RefPtr->Release();
		RefPtr = NULL;
	}
}

unsigned Direct3DEngine::CreateObject( unsigned short Layer )
{
	unsigned Result;
	if( Layer <= inventory.vLayers.size() )
	{
		if( inventory.vObjHandlesGC.size() )
		{
			Result = inventory.vObjHandlesGC.back();
			inventory.vObjHandlesGC.pop_back();
		}
		else
			Result = inventory.vObjHandles.size();
		if( inventory.vObjHandles.size() <= Result )
			inventory.vObjHandles.resize( 1 + Result );
		ObjHandle & handle = inventory.vObjHandles[ Result ];
		handle.Layer = Layer;
		handle.RefCount = 1;
		handle.MgrIdx = inventory.vLayers[ Layer ].vObjMgr.size();
		handle.ObjIdx = 0;
		ObjMgr * objMgr = &(*inventory.vLayers[ Layer ].vObjMgr.insert( inventory.vLayers[ Layer ].vObjMgr.end(), ObjMgr() ) );
		objMgr->PushObj( 0 );
		objMgr->SetVertexDeclaration( inventory.pDefaultVDeclaration );
		objMgr->SetVertexShader( inventory.pDefault3DVShader );
		objMgr->SetPixelShader( inventory.pDefault3DPShader );
		objMgr->SetVShaderConstTable( inventory.pDefaultConstable );
	}
	else
		Result = -1;
	return Result;
}
void Direct3DEngine::AddRefObjHandle( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
		++inventory.vObjHandles[ HandleIdx ].RefCount;
}
void Direct3DEngine::ReleaseObjHandle( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
		if( !--inventory.vObjHandles[ HandleIdx ].RefCount )
			inventory.vObjHandlesGC.push_back( HandleIdx );
}
void Direct3DEngine::ReleaseObject( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = inventory.vObjHandles[ HandleIdx ];
		inventory.vLayers[ handle.Layer ].vObjMgr[ handle.MgrIdx ].EraseObj( handle.ObjIdx );
		if( !inventory.vLayers[ handle.Layer ].vObjMgr[ handle.MgrIdx ].GetObjCount() )
		{
			inventory.vLayers[ handle.Layer ].vObjMgr.erase( inventory.vLayers[ handle.Layer ].vObjMgr.begin() + handle.MgrIdx );
			for( auto it = inventory.vObjHandles.begin(); it != inventory.vObjHandles.end(); ++it )
				if( it->Layer == handle.Layer && it->MgrIdx > handle.MgrIdx )
					--(it->MgrIdx);
			handle.MgrIdx = -1;
		}
		handle.ObjIdx = -1;
	}
}
Object * Direct3DEngine::GetObject( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = inventory.vObjHandles[ HandleIdx ];
		if( handle.ObjIdx != -1 && handle.MgrIdx != -1 )
			return inventory.vLayers[ handle.Layer ].vObjMgr[ handle.MgrIdx ].GetObjPtr( handle.ObjIdx );
	}
	return 0;
}
ObjMgr * Direct3DEngine::GetObjMgr( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = inventory.vObjHandles[ HandleIdx ];
		if( handle.MgrIdx != -1 )
			return &inventory.vLayers[ handle.Layer ].vObjMgr[ handle.MgrIdx ];
	}
	return 0;
}