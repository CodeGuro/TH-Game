#include <Direct3DEngine.hpp>
#include <ObjMgr.hpp>
#include <sstream>
#include <cassert>

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


	//0 - background
	//1 - 3D
	//2 - enemy boss
	//3 - player
	//4 - enemies
	//5 - bullets
	//6 - effects
	//7 - sprites/text
	//8 - foreground
	GetLayers().resize( 8 );
	CreateObject( 4 );

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
Battery::vLayer_t & Battery::GetLayers()
{
	return vLayers;
}
unsigned Battery::CreateObject( unsigned short Layer )
{
	unsigned Result;
	if( Layer <= GetLayers().size() )
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
		handle.Layer = Layer;
		handle.RefCount = 1;
		handle.MgrIdx = GetLayers()[ Layer ].vObjMgr.size();
		handle.ObjIdx = 0;
		ObjMgr * objMgr = &(*GetLayers()[ Layer ].vObjMgr.insert( GetLayers()[ Layer ].vObjMgr.end(), ObjMgr() ) );
		objMgr->PushObj( 0 );
		objMgr->SetVertexDeclaration( pDefaultVDeclaration );
		objMgr->SetVertexShader( pDefault3DVShader );
		objMgr->SetPixelShader( pDefault3DPShader );
		objMgr->SetVShaderConstTable( pDefaultConstable );
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
		GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].EraseObj( handle.ObjIdx );
		if( !GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].GetObjCount() )
		{
			GetLayers()[ handle.Layer ].vObjMgr.erase( GetLayers()[ handle.Layer ].vObjMgr.begin() + handle.MgrIdx );
			for( auto it = vObjHandles.begin(); it != vObjHandles.end(); ++it )
				if( it->Layer == handle.Layer && it->MgrIdx > handle.MgrIdx )
					--(it->MgrIdx);
			handle.MgrIdx = -1;
		}
		handle.ObjIdx = -1;
	}
}
Object * Battery::GetObject( unsigned HandleIdx )
{
	if( HandleIdx != -1 )
	{
		ObjHandle & handle = vObjHandles[ HandleIdx ];
		if( handle.ObjIdx != -1 && handle.MgrIdx != -1 )
			return GetLayers()[ handle.Layer ].vObjMgr[ handle.MgrIdx ].GetObjPtr( handle.ObjIdx );
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
void Battery::LoadTexture( std::string const pathname )
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
void Battery::DeleteTexture( std::string const pathname )
{
	auto it = mapTextures.find( pathname );
	if( it != mapTextures.end() )
	{
		it->second->Release();
		mapTextures.erase( it );
	}
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
		handle.MgrIdx = 0;
		handle.ObjIdx = GetLayers()[ 4 ].vObjMgr[ 0 ].PushEmptyObj();
		Object & obj = *GetObject( Result );
		obj.position = D3DXVECTOR3( position.x, position.y, 0.f );
		obj.velocity = D3DXVECTOR3( speed * cos( direction ), speed * sin( direction ), 0.f );
		obj.accel = D3DXVECTOR3( 0, 0, 0 );
		obj.SetRotation( D3DX_PI / 2 );
		obj.SetAngle( direction );
		obj.FlagMotion( 1 );
		obj.FlagCollidable( 1 );
		obj.FlagScreenDeletable( 1 );
		obj.ShotData = (ULONG)graphic;
		return Result;
	}
	return -1;
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
	DrawTexture();
	DrawFPS();
	DrawObjects();
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
		for( auto Obj = L->vObjMgr.begin(); Obj != L->vObjMgr.end(); ++Obj )
			Obj->AdvanceTransformedDraw( this, L - GetLayers().begin() == 4 );
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