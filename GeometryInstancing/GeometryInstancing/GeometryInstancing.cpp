//--------------------------------------------------------------------------------------
// File: GeometryInstancing.cpp
// 
// *************************************************************************************
// Test di performance in una scena ripetitiva utilizzando e non il geometry instancing.
// Usiamo http://spark.developpez.com/ come engine per la fisica del particle system.
// *************************************************************************************
//
// @author: Andrea Casaccia <acasaccia@gmail.com>
// @date: 15/05/2012
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "d3dx11effect.h"
#include <sstream> 

#include "SPK.h"
#include "BaseRenderer.h"
#include "InstanceRenderer.h"

#include "GeometryInstancing.h"

// DXUT Contiene già i seguenti header
//#include <windows.h>
//#include <d3d11.h>
//#include <d3dx11.h>

//--------------------------------------------------------------------------------------
// Dichiarazione variabili globali
//--------------------------------------------------------------------------------------
CDXUTTextHelper *gmTextHelper = NULL;				 // Text Helper Class provided by DXUT
bool gbToggleFps = true;							 // Fps text switch
bool gbToggleInstancing = false;					 // Toggle this to enable instancing awesomeness
CDXUTDialogResourceManager  g_DialogResourceManager; // Manager for shared resources of dialogs (included text)

SPK::System *g_particleSystem;
BaseRenderer *g_baseRenderer;
InstanceRenderer *g_instanceRenderer;

// Matrici di trasformazione
D3DXMATRIX                  g_World;
D3DXMATRIX                  g_View;
D3DXMATRIX                  g_Projection;

// Parametri telecamera
D3DXVECTOR3 g_ViewUpVector; // Vettore Up
D3DXVECTOR3 g_ViewEyeVector; // Posizione
D3DXVECTOR3 g_ViewAtVector; // Punto fissato
D3DXVECTOR3 g_ViewViewVector; // Vettore vista - At = Eye + View
D3DXVECTOR3 g_ViewRightVector; // Vettore dx

ID3DX11Effect* g_pEffect;

ID3DX11EffectTechnique* g_pTechniqueSimple;
ID3DX11EffectTechnique* g_pTechniqueInstanced;

ID3D11ShaderResourceView* g_pTextureRV;

ID3DX11EffectMatrixVariable* g_pWorldVariable;
ID3DX11EffectMatrixVariable* g_pViewVariable;
ID3DX11EffectMatrixVariable* g_pProjectionVariable;

ID3D11Buffer* g_pVertexBuffer;
ID3D11Buffer* g_pIndexBuffer;
ID3D11Buffer* g_pInstanceBuffer;

ID3D11InputLayout* g_pVertexLayout;
ID3D11InputLayout* g_pInstanceLayout;

//--------------------------------------------------------------------------------------
// Dichiarazione costanti
//--------------------------------------------------------------------------------------
#define CAMERASPEED 10.0f
#define CAMERAANGLESPEED 0.1f

//--------------------------------------------------------------------------------------
// Dichiarazione prototipi funzione 
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

void PrintTitleAndFps();

//--------------------------------------------------------------------------------------
// Funzione main del programma. Inizializza la finestra utilizzando DXUT ed entra nel main loop.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Registra Call-back con DXUT
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);

    // Crea/Inizializza finestra e Device con DXUT
	DXUTInit( true, true ); // Parsing linea di comando e mostra dialog box se errore
	DXUTCreateWindow( L"Geometry Instancing" ); // Titolo finestra
	HRESULT hr = DXUTCreateDevice(
		D3D_FEATURE_LEVEL_11_0,
		true,
		640, 480
	);

	DXUTMainLoop();

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Controlla i device disponibili e Rifiuta ogni device non compabile.
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext){
    return true;
}


//--------------------------------------------------------------------------------------
// Crea i layout di input
//--------------------------------------------------------------------------------------
HRESULT CALLBACK CreateInputLayouts( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext ){

	// Creiamo e configuriamo il layout di input base
	D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( vertexLayout ) / sizeof( vertexLayout[0] );

    D3DX11_PASS_DESC PassDesc;
    g_pTechniqueSimple->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    HRESULT hr = pd3dDevice->CreateInputLayout(vertexLayout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pVertexLayout );
    if( FAILED( hr ) )
        return hr;

	// Creiamo e configuriamo il layout di input per le istanze
	// Offset di posizione e colore sono passati a livello di istanza!
	D3D11_INPUT_ELEMENT_DESC instanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	numElements = sizeof( instanceLayout ) / sizeof( instanceLayout[0] );

	// Creiamo e configuriamo il layout di input con instancing
	g_pTechniqueInstanced->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	hr = pd3dDevice->CreateInputLayout(instanceLayout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &g_pInstanceLayout );

    if( FAILED( hr ) )
        return hr;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Crea i buffers
//--------------------------------------------------------------------------------------
HRESULT CALLBACK CreateBuffers( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext ){

	// Settiamo il descrittore per il vertex buffer
    D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DYNAMIC; // DEFAULT - gpu read-write - IMMUTABLE gpu read-only -  DYNAMIC - scritta e letta anche dalla cpu spesso (ad ogni frame) 
    bd.ByteWidth = sizeof( VertexWithColor ) * ( MAX_PARTICLES_NUMBER * 4 ); // Dimensione
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // Identifica dove deve essere associata nella pipeline
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // La cpu ha accesso in scrittura.
    bd.MiscFlags = 0;

	// Creiamo il buffer nel device.
    HRESULT hr = pd3dDevice->CreateBuffer( &bd, NULL, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

	// Settiamo il descrittore per l'instance buffer
    bd.Usage = D3D11_USAGE_DYNAMIC; // DEFAULT - gpu read-write - IMMUTABLE gpu read-only -  DYNAMIC - scritta e letta anche dalla cpu spesso (ad ogni frame) 
    bd.ByteWidth = sizeof( Instance ) * ( MAX_PARTICLES_NUMBER ); // Dimensione
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // Identifica dove deve essere associata nella pipeline
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // La cpu ha accesso in scrittura.
    bd.MiscFlags = 0;

	// Creiamo il buffer nel device.
    hr = pd3dDevice->CreateBuffer( &bd, NULL, &g_pInstanceBuffer );
    if( FAILED( hr ) )
        return hr;

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Inizializza il particle system
// @see http://spark.ftp-developpez.com/files/tutos/SPARK_tuto_1_en.pdf
//--------------------------------------------------------------------------------------
void InitializeParticleSystem(){
										  
	// Inizializzazione del particle system
	SPK::Model* particleModel = SPK::Model::create (
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
		SPK::FLAG_ALPHA,
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE
	);

	particleModel->setParam(SPK::PARAM_ALPHA,1.0f,0.0f);
	particleModel->setLifeTime(PARTICLE_LIFETIME_MIN,PARTICLE_LIFETIME_MAX);

	// Creates the emitter
	SPK::SphericEmitter* emitter = SPK::SphericEmitter::create( SPK::Vector3D(0.0f,1.0f,0.0f), 0.5f, 0.5f );
	emitter->setForce(PARTICLE_FORCE_MIN,PARTICLE_FORCE_MAX);
	emitter->setTank(PARTICLE_TANK);
	emitter->setFlow(PARTICLE_FLOW);

	SPK::Group* particleGroup = SPK::Group::create(particleModel,MAX_PARTICLES_NUMBER);
	particleGroup->addEmitter(emitter);

	g_baseRenderer = new BaseRenderer(g_pVertexLayout, g_pTechniqueSimple, g_pVertexBuffer);
	g_instanceRenderer = new InstanceRenderer(g_pInstanceLayout, g_pTechniqueInstanced, g_pVertexBuffer, g_pInstanceBuffer);

	particleGroup->setRenderer(g_baseRenderer);

	particleGroup->setGravity(SPK::Vector3D(0.0f,PARTICLE_GRAVITY,0.0f));
	particleGroup->setFriction(PARTICLE_FRICTION);
	particleGroup->enableDistanceComputation(true);
	particleGroup->enableSorting(true);

	g_particleSystem = SPK::System::create();
	g_particleSystem->addGroup(particleGroup);

}


//--------------------------------------------------------------------------------------
// Inizializza le risorse da utilizzare. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext )
{
	g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, DXUTGetD3D11DeviceContext());

	// Inizializza helper per scrittura di testo
	gmTextHelper = new CDXUTTextHelper( pd3dDevice, DXUTGetD3D11DeviceContext(),&g_DialogResourceManager,15 );

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	#if defined( DEBUG ) || defined( _DEBUG )
    // Il settaggio di questo flag permette di eseguire il debug diretto dello shader, senza
	// impattare in modo marcato sulle performance.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    #endif

	ID3D10Blob*	pBlob = NULL;
	ID3D10Blob*	pErrorBlob = NULL;

	// Creazione effetto in g_pEffect, caricandolo dal file fx
	HRESULT hr = D3DX11CompileFromFile(L"Instancing.fx", NULL, NULL, NULL, "fx_5_0", dwShaderFlags, 0, NULL, &pBlob, &pErrorBlob, NULL);

    if( FAILED( hr ) )
    {
        MessageBox( NULL, L"Problema nel caricamento di basic_effect.fx.", L"Error", MB_OK );

		if (pErrorBlob)	 {
			MessageBoxA(0, (char*)pErrorBlob->GetBufferPointer(), 0, 0);
		}

        return hr;
    }

	D3DX11CreateEffectFromMemory(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pd3dDevice ,&g_pEffect);

	// Trova le tecniche definita nel .fx
    g_pTechniqueSimple = g_pEffect->GetTechniqueByName( "SimpleRender" );
	g_pTechniqueInstanced = g_pEffect->GetTechniqueByName( "InstancedRender" );

	// Otteniamo le variabili
	g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
	g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();	
	g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();

	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, L"Particle.png", NULL, NULL, &g_pTextureRV, &hr);
	if( FAILED( hr ) )
		return hr;

	g_pEffect->GetVariableByName("Particle")->AsShaderResource()->SetResource(g_pTextureRV);

	hr = CreateInputLayouts(pd3dDevice, pBufferSurfaceDesc, pUserContext);
	if( FAILED( hr ) )
		return hr;

	hr = CreateBuffers(pd3dDevice, pBufferSurfaceDesc, pUserContext);
	if( FAILED( hr ) )
		return hr;

	// World
	D3DXMatrixIdentity(&g_World);

	// View
	g_ViewEyeVector = D3DXVECTOR3( 0.0f, 1.0f, -10.0f );
	g_ViewUpVector = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	g_ViewAtVector = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

	g_ViewViewVector = (g_ViewAtVector - g_ViewEyeVector); 
	D3DXVec3Normalize(&g_ViewViewVector, &g_ViewViewVector);
	D3DXVec3Cross(&g_ViewRightVector,&g_ViewUpVector,&g_ViewViewVector);

	D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );

	// Projection
	D3DXMatrixPerspectiveFovLH( &g_Projection, ( float )D3DX_PI * 0.2f,((float)DXUTGetWindowWidth() / (float)DXUTGetWindowHeight()), 0.1f, 100.0f);

	DXUTSetCursorSettings(false, true);

	InitializeParticleSystem();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Crea ogni risorsa dipendendente dal back buffer.
// Resetta l'aspectratio modificando viewport o matrice di proiezione.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext ){

	g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBufferSurfaceDesc );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Effettua il rendering
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext){

    // Pulisci back buffer
    float ClearColor[4] = { 0.0f, 0.0f, 0.1f, 1.0f }; // red, green, blue, alpha
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView(); // Ottieni il render target in cui stiamo disegnando
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );

	// Setta e pulisci lo Z-Buffer (depth stencil)
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	// Aggiorna matrici
	g_pWorldVariable->SetMatrix( ( float* )&g_World );
	g_pViewVariable->SetMatrix( ( float* )&g_View );
	g_pProjectionVariable->SetMatrix( ( float* )&g_Projection );

	// Aggiorna la posizione della camera per il sorting delle particelle quindi esegue l'update del sistema
	g_particleSystem->setCameraPosition(SPK::Vector3D(g_ViewEyeVector.x,g_ViewEyeVector.y,g_ViewEyeVector.z));
	g_particleSystem->update(fElapsedTime);
	g_particleSystem->render();

	PrintTitleAndFps();
}

//--------------------------------------------------------------------------------------
// Stampa nella parte alta dello schermo titolo e frame per secondo
//--------------------------------------------------------------------------------------
void PrintTitleAndFps(){
	// Utilizza la classe gmTextHelper e le routine di DXUT per stampare a schermo i FPS
	gmTextHelper->Begin();
	gmTextHelper->SetInsertionPos( 1, 1 );
	gmTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );		

	std::wostringstream s;
	s << "(A)/(R): Add/Remove particles: " << g_particleSystem->getGroup(0)->getEmitter(0)->getFlow() << " per second";
	const std::wstring tmp = s.str();
	const WCHAR* cstr = tmp.c_str();

	gmTextHelper->DrawTextLine(cstr);

	if(gbToggleInstancing){
		gmTextHelper->DrawTextLine(L"(I): toggle Geometry Instancing: ENABLED");
	} else {
		gmTextHelper->DrawTextLine(L"(I): toggle Geometry Instancing: DISABLED");
	}

	// Stampa la stringa contenente i FPS
	if(gbToggleFps){
		gmTextHelper->DrawTextLine(DXUTGetFrameStats(true));
	}
	
	gmTextHelper->End();
}

//--------------------------------------------------------------------------------------
// Rilascia eventuali risorse create in OnD3D11ResizedSwapChain
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Rilascia eventuali risorse create in OnD3D11CreateDevice 
// Device-Swapbuffer e rendertarget sono rilasciate in automatico.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	if(gmTextHelper)
		delete gmTextHelper;
	gmTextHelper = 0;

	g_DialogResourceManager.OnD3D11DestroyDevice();

	if( g_pVertexBuffer ) g_pVertexBuffer->Release();
	if( g_pVertexLayout ) g_pVertexLayout->Release();
	if( g_pEffect ) g_pEffect->Release();
	if( g_pIndexBuffer ) g_pIndexBuffer->Release();
	if( g_pInstanceBuffer ) g_pInstanceBuffer->Release();
	if( g_pInstanceLayout ) g_pInstanceLayout->Release();
	if( g_pTextureRV ) g_pTextureRV->Release();

}

//--------------------------------------------------------------------------------------
// Chiamata prima di inizializzare un device, ci permette di specificare parametri personalizzati per il device
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Gestisce modifiche alla scena.  Le animazioni andranno inserite qui. Chiamata prima di OnD3D11FrameRender nel main loop
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Gestione movimenti telecamera
	// Non gestiamo l'orientamento delle particelle, quindi spostiamoci solo sull'asse z

	//if(DXUTIsKeyDown(VK_LEFT)){
	//	// Spostamento negativo rispetto direzione del right vector (sinistra)
	//	g_ViewEyeVector -= DXUTGetElapsedTime() * CAMERASPEED * g_ViewRightVector;
	//	g_ViewAtVector -= DXUTGetElapsedTime() * CAMERASPEED * g_ViewRightVector;
	//	D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	//}

	//if(DXUTIsKeyDown(VK_RIGHT)){
	//	// Spostamento positivo rispetto direzione del right vector (destra)
	//	g_ViewEyeVector += fElapsedTime * CAMERASPEED * g_ViewRightVector;
	//	g_ViewAtVector += fElapsedTime * CAMERASPEED * g_ViewRightVector;
	//	D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	//}

	if(DXUTIsKeyDown(VK_UP)){
		// Spostamento positivo rispetto alla direzione di vista (avanti)
		g_ViewEyeVector += fElapsedTime * CAMERASPEED * g_ViewViewVector;
		g_ViewAtVector += fElapsedTime * CAMERASPEED * g_ViewViewVector;
		D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	}

	if(DXUTIsKeyDown(VK_DOWN)){
		// Spostamento negativo rispetto alla direzione di vista (indietro)
		g_ViewEyeVector -= fElapsedTime * CAMERASPEED * g_ViewViewVector;
		g_ViewAtVector -= fElapsedTime * CAMERASPEED * g_ViewViewVector;
		D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	}

	//if(DXUTIsKeyDown('q') || DXUTIsKeyDown('Q')){
	//	// Spostamento positivo rispetto alla direzione dell'Up Vector (Su)
	//	g_ViewEyeVector += fElapsedTime * CAMERASPEED * g_ViewUpVector;
	//	g_ViewAtVector += fElapsedTime * CAMERASPEED * g_ViewUpVector;
	//	D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	//}

	//if(DXUTIsKeyDown('a') || DXUTIsKeyDown('A')){
	//	// Spostamento negativo rispetto alla direzione dell'Up Vector (Giù)
	//	g_ViewEyeVector -= fElapsedTime * CAMERASPEED * g_ViewUpVector;
	//	g_ViewAtVector -= fElapsedTime * CAMERASPEED * g_ViewUpVector;
	//	D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );
	//}

	//static bool bMoving = false;
	//static POINT prevCoord;
	//if(DXUTIsMouseButtonDown(VK_LBUTTON))
	//{
	//	POINT coord;
	//	GetCursorPos(&coord);
	//	if(bMoving){

	//		// Moto relativo a X e Y
	//		float motionX = (float)coord.x - (float)prevCoord.x;
	//		float motionY = (float)coord.y - (float)prevCoord.y;

	//		// Costruiamo la matrice di Yaw (rotazione rispetto a Y) con un angolo dipendente dalla direzione relativa rispetto a X
	//		D3DXMATRIX yawMatrix;
	//		D3DXMatrixRotationAxis(&yawMatrix, &g_ViewUpVector, DEG2RAD(motionX * CAMERAANGLESPEED) ); 
	//		// Applichiamo la trasformazione ai vettori view e right.
	//		// Dato che la matrice di rotazione è ortogonale non abbiamo bisogno di rinormalizzare i vettori dopo la trasformazione
	//		D3DXVec3TransformCoord(&g_ViewViewVector, &g_ViewViewVector, &yawMatrix);
	//		D3DXVec3TransformCoord(&g_ViewRightVector, &g_ViewRightVector, &yawMatrix); 

	//		// Costruiamo la matrice di Pitch (rotazione rispetto a X) con un angolo dipendente dalla direzione relativa rispetto a Y
	//		D3DXMATRIX pitchMatrix;
	//		D3DXMatrixRotationAxis(&pitchMatrix, &g_ViewRightVector, DEG2RAD(motionY * CAMERAANGLESPEED) ); 
	//		// Applichiamo la trasformazione ai vettori view e Up.
	//		// Dato che la matrice di rotazione è ortogonale non abbiamo bisogno di rinormalizzare i vettori dopo la trasformazione
	//		D3DXVec3TransformCoord(&g_ViewViewVector, &g_ViewViewVector, &pitchMatrix);
	//		D3DXVec3TransformCoord(&g_ViewUpVector, &g_ViewUpVector, &pitchMatrix); 

	//		// Aggiorniamo la posizione osservata e ricostruiamo la matrice di view
	//		g_ViewAtVector = g_ViewEyeVector + g_ViewViewVector;
	//		D3DXMatrixLookAtLH( &g_View, &g_ViewEyeVector, &g_ViewAtVector, &g_ViewUpVector );

	//	}
	//	prevCoord = coord;
	//	bMoving = true;
	//}
	//else
	//{
	//	bMoving = false;
	//}

}


//--------------------------------------------------------------------------------------
// Gestisce coda messaggi
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    return 0;
}

//--------------------------------------------------------------------------------------
// Gestisce eventi da tastiera
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if( bKeyDown )
    {
        switch( nChar )
        {
			case 'i': 
            case 'I':
				gbToggleInstancing = !gbToggleInstancing;
				if (gbToggleInstancing)
					g_particleSystem->getGroup(0)->setRenderer(g_instanceRenderer);
				else
					g_particleSystem->getGroup(0)->setRenderer(g_baseRenderer);
				break;
            case 'f': 
            case 'F': 
				gbToggleFps = !gbToggleFps;
				break;
			case 'a':
			case 'A':
				g_particleSystem->getGroup(0)->getEmitter(0)->changeFlow(+10.0f);
				break;
			case 'r':
			case 'R':
				g_particleSystem->getGroup(0)->getEmitter(0)->changeFlow(-10.0f);
				break;
			case VK_F10:
			    DXUTToggleFullScreen();
				break;	
        }
    }
}

