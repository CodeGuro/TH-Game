#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <Direct3DEngine.h>
#include <Windows.h>

Direct3DEngine::Direct3DEngine() : d3d( Direct3DCreate9( D3D_SDK_VERSION ) ), d3ddev( NULL )
{
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
	D3dRelease( d3d );
	D3dRelease( d3ddev );
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
		
		d3dpp.BackBufferWidth = windowed ? rec.right - rec.left : d3ddm.Width;
		d3dpp.BackBufferHeight = windowed ? rec.bottom - rec.top : d3ddm.Height;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferCount = 0;
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

		d3ddev->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
		d3ddev->Present( NULL, NULL, NULL, NULL );

		CamSettings.Eye = D3DXVECTOR3( 0, 0, 0 );
		CamSettings.LookAt = D3DXVECTOR3( 0, 0, 1 );
		CamSettings.UpDir = D3DXVECTOR3( 0, 1, 0 );
		
	}
}
void Direct3DEngine::TestDevice()
{
	LPDIRECT3DTEXTURE9 d3dtexture;
	LPDIRECT3DVERTEXDECLARATION9 d3dvertexdecl;
	LPDIRECT3DVERTEXBUFFER9 d3dvertexbuffer;

	D3DXCreateTextureFromFile( d3ddev, "tester.png", &d3dtexture );
	d3ddev->SetTexture( 0, d3dtexture );

	D3DVERTEXELEMENT9 d3dvertelement[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 }, //port position
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, //texture position
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 }, //color
		D3DDECL_END()
	};

	d3ddev->CreateVertexDeclaration( d3dvertelement, &d3dvertexdecl );
	d3ddev->SetVertexDeclaration( d3dvertexdecl );

	struct vertex
	{
		float xpos;
		float ypos;
		float upos;
		float vpos;
		D3DCOLOR color;
	};
	vertex v[4] =
	{
		{ 0, 0, 0, 0, D3DCOLOR_ARGB( 0, 255, 255, 255 ) },
		{ (float)d3dtexture->Width, 0, 1.f, 0, D3DCOLOR_ARGB( 0, 255, 255, 255 ) },
		{ 0, (float)d3dtexture->Height, 0, 1.f, D3DCOLOR_ARGB( 255, 255, 255, 255 ) },
		{ (float)d3dtexture->Width, (float)d3dtexture->Height, 1.f, 1.f, D3DCOLOR_ARGB( 255, 255, 255, 255 ) }
	};
	d3ddev->CreateVertexBuffer( sizeof(v), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &d3dvertexbuffer, NULL );

	void *ptr;
	d3dvertexbuffer->Lock( 0, 0, &ptr, 0 );
	memcpy( ptr, v, sizeof(v) );
	d3dvertexbuffer->Unlock();

	d3ddev->SetStreamSource( 0, d3dvertexbuffer, 0, sizeof(vertex) );
	d3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	d3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	d3ddev->SetRenderState( D3DRS_ZENABLE, TRUE );
	d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	D3DXMATRIX world, view, proj;
	D3DXMatrixLookAtLH( &view, &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,0,1), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixPerspectiveFovLH( &proj, D3DXToRadian(45), 640.f/480, 1.0f, 100.0f );
	D3DXMatrixIdentity( &world );

	d3ddev->SetTransform( D3DTS_WORLD, &world );
	d3ddev->SetTransform( D3DTS_VIEW, &view );
	d3ddev->SetTransform( D3DTS_PROJECTION, &proj );


	d3ddev->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 100, 30, 180 ), 1.f, 0 );
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();
	d3ddev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	d3ddev->EndScene();
	d3ddev->Present( NULL, NULL, NULL, NULL );

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