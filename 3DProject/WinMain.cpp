#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

#include <Windows.h>
#include "Window.h"
#include "Graphics.h"
#include "Camera.h"
#include "DeferredRendering.h"
#include "Structs.h"
#include "Mesh.h"
#include "HeightMap.h"
#include "ShadowMap.h"
#include "ParticleSystem.h"

void setPerFrameMatrixes(PerFrameMatrixes& object, Camera* camera, ID3D11DeviceContext* immediateContext, ID3D11Buffer*& perFrameConstantBuffer)
{
	DirectX::XMStoreFloat4(&object.CameraPosition, camera->getCameraPos());
	DirectX::XMStoreFloat4x4(&object.ViewMatrix, camera->getCameraView());
	DirectX::XMStoreFloat4x4(&object.ProjectionMatrix, camera->getCameraProjection());

	immediateContext->UpdateSubresource(perFrameConstantBuffer, 0, nullptr, &object, 0, 0);
}

//Sending everytihng to the immidiate context (device context) ((screen))
void geomatryPass(ID3D11DeviceContext*& immediateContext, D3D11_VIEWPORT& viewport, ID3D11VertexShader*& vertexShader,
	ID3D11PixelShader*& pixelShader, ID3D11InputLayout*& inputLayout, ID3D11Buffer*& pPixelConstantBuffer, 
	Deferred& deferred, Mesh& objObject, Camera* camera, Mesh& objObject2, ID3D11GeometryShader*& geomatryShader, Mesh& WaterMesh, 
	Mesh& cubeMesh, ID3D11Buffer*& perFrameConstantBuffer, Mesh& eyeOne, Mesh& platformMesh)
{
	//immidiate context is the link or "adapter" to the hardwere. This is the thing that makes you see shit on the screen. 
	float clearColor[4] = { 1,1,1,1 };
	deferred.clearRenderTargets(immediateContext);
	deferred.unbindShaderResourceView(immediateContext);


	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	immediateContext->RSSetViewports(1, &viewport);
	immediateContext->IASetInputLayout(inputLayout);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->GSSetShader(geomatryShader, nullptr, 0);

	deferred.setRenderTargets(immediateContext);

	objObject.DrawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
	//objObject2.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
	WaterMesh.DrawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
	cubeMesh.DrawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
	eyeOne.DrawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
	platformMesh.DrawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, pPixelConstantBuffer, camera);
}

void lightPass(ID3D11DeviceContext*& immediateContext, ID3D11RenderTargetView*& renderTargetView, ID3D11VertexShader*& lightPassVertexShader,
	ID3D11PixelShader*& lightPassPixelShader, ID3D11InputLayout*& inputLayout, ID3D11Buffer*& fullScreenVertexBuffer,
	ID3D11Buffer*& pConstantBuffer,ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, Deferred& deferred, 
	Light& light, ShadowMap shadowObject, ID3D11Buffer*& perFrameConstantBuffer, ID3D11Buffer*& pShadowConstantBuffer)
{
	MatrixFunctions matrixFunction;
	float clearColor[4] = { 1,1,1,1 };
	deferred.unbindShaderResourceView(immediateContext);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	immediateContext->IASetVertexBuffers(0, 1, &fullScreenVertexBuffer, &stride, &offset);
	immediateContext->IASetInputLayout(inputLayout);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deferred.setLightPassRenderTarget(renderTargetView, immediateContext); //geting the back buffer to putt the stuff in it
	deferred.setShaderResourceView(immediateContext, shadowObject.depthMap.shaderResourceView.Get());

	immediateContext->VSSetShader(lightPassVertexShader, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	immediateContext->PSSetShader(lightPassPixelShader, nullptr, 0);

	immediateContext->UpdateSubresource(pPixelConstantBuffer, 0, NULL, &light, 0, 0);
	immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);

	immediateContext->VSSetConstantBuffers(0u, 1, &pConstantBuffer);
	immediateContext->PSSetConstantBuffers(0u, 1, &pPixelConstantBuffer);   // B0
	immediateContext->PSSetConstantBuffers(1u, 1, &pShadowConstantBuffer);  // B1
	immediateContext->PSSetConstantBuffers(2u, 1, &perFrameConstantBuffer); // B2

	immediateContext->Draw(6, 0);

	deferred.unbindShaderResourceView(immediateContext);
	deferred.clearRenderTargets(immediateContext);

}

//add _In_ and _In_opt_ to get rid of the C28251 Inconsistent annotation 
int	CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
// hInstance: is a "handle to instance" that is used to indentify a executable (EXE).
// hPrevInstance: Has no longer meaning.
// pCmdLine: contains the command-line arguments as a Unicode string.
// nCmdShow: flag that says in what format the window should be shown (minimized, maximized, or shown normally)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//-----------------------------------------------------------------//
	// Setup window
	//-----------------------------------------------------------------//

	const UINT WIDTH = 640;
	const UINT HEIGHT = 640;

	HWND windowHandle;
	if (!setUpWindow(WIDTH, HEIGHT, windowHandle, nCmdShow, hInstance))
	{
		std::cerr << "Faild to set up window" << std::endl;
		return -1;
	}
	setUpCMD();


	//-----------------------------------------------------------------//
	// Camera
	//-----------------------------------------------------------------//

	Camera* walkingCamera = new Camera({ -6.0f, 5.0f, 4.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, WIDTH, HEIGHT);
	walkingCamera->setWindowForMouse(windowHandle);


	//-----------------------------------------------------------------//
	// Engine Objects
	//-----------------------------------------------------------------//

	//All the tihngs needed
	ID3D11RasterizerState*   rasStateNoCulling   = nullptr; // Back face culling with geomatry shader
	ID3D11Device*            pDevice             = nullptr;
	IDXGISwapChain*          pSwapChain          = nullptr;
	ID3D11DeviceContext*     immediateContext    = nullptr;
	ID3D11RenderTargetView*  renderTargetView    = nullptr;
	ID3D11VertexShader*      vertexShader        = nullptr;
	ID3D11PixelShader*       pixelShader         = nullptr;
	ID3D11GeometryShader*    geomatryShader      = nullptr;  // Back face culling
	ID3D11VertexShader*      ShadowVertexShader  = nullptr;  // Shadow shader
	
	ID3D11InputLayout*       inputLayout         = nullptr;
	ID3D11InputLayout*		 shadowInputLayout   = nullptr;
	D3D11_VIEWPORT           viewPort;
	std::string              vertexShaderByteCode;
	std::string              vertexShadowShaderByteCode;       // Shadow map
	ID3D11Buffer*            pConstantBuffer          = nullptr; // Interface pointer
	ID3D11Buffer*            pPixelConstantBuffer     = nullptr; // Light buffer
	ID3D11Buffer*            fullScreenVertexBuffer   = nullptr; // Fullscreen quad
	ID3D11Buffer*			 pShadowConstantBuffer    = nullptr; // Shadow map
	ID3D11Buffer*            pPerFrameConstantBuffer  = nullptr;
	ID3D11SamplerState*      sampler                  = nullptr; // Using the repeat thing

	//Deferred things

	ID3D11VertexShader*       lightPassVertexShader = nullptr;
	ID3D11PixelShader*        lightPassPixelShader  = nullptr;

	std::string               lightPassVertexShaderByteCode;



	//matrixsstuff for the quad
	constantBufferMatrixes matrixes;//used to send the world matrix and worldviewprojection matrixes to the shader later for 
	DirectX::XMMATRIX WorldViewProjection = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	World = DirectX::XMMatrixTranslation(0.0f, 5.0f, 0.0f);
	matrixes.hasNormal  = false;
	matrixes.hasTexture = true;
	MatrixFunctions matrixFunction;

	//per frame matrix stuff
	PerFrameMatrixes perframeMatrixes;
	perframeMatrixes.CameraPosition   = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.0f);
	perframeMatrixes.ProjectionMatrix = {};
	perframeMatrixes.ViewMatrix       = {};

	//-----------------------------------------------------------------//
	// Light attributes
	//-----------------------------------------------------------------//

	Light light;
	light.position       = DirectX::XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); // X+ = >, Z+ = ^
	light.attenuation    = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	light.ambient        = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.diffuse        = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.cameraPosition = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.0f);
	light.direction      = DirectX::XMFLOAT4(0.0f, -0.99995f, 0.009999f, 0.0f);
	light.range = 100.0f;

	//-----------------------------------------------------------------//
	// Load Graphics class
	//-----------------------------------------------------------------//
	//Make sure to put in a "finished" window. When i declared this one above the setUpWindow the pSwapChain become nullptr.
	//Does all the gapthic stuff like creating the interface, create render target view, creating depth stencil, setting the viewport, load vertex and pixel shader, create input layer, creating vertex buffer and so on. 
	Graphics setUpTheGraphics
	(
		WIDTH, HEIGHT,
		windowHandle,
		pDevice,
		immediateContext,
		pSwapChain,
		renderTargetView,
		viewPort,
		vertexShader,
		pixelShader,
		vertexShaderByteCode,
		inputLayout,
		pConstantBuffer,
		sampler,
		light,
		pPixelConstantBuffer,
		matrixes,
		fullScreenVertexBuffer,
		lightPassVertexShader,
		lightPassPixelShader,
		lightPassVertexShaderByteCode,
		rasStateNoCulling,
		geomatryShader,
		pPerFrameConstantBuffer, 
		perframeMatrixes

	);

	//-----------------------------------------------------------------//
	// Particle things
	//-----------------------------------------------------------------//

	//Particle system stuff
	DirectX::XMFLOAT4 particlePosition(-30, 10, -30, 1);
	Particle particleList[MAX_PARTICLES];
	ParticleSystem particles;
	particles.InitializeParticles(pDevice, particleList, particlePosition);


	//-----------------------------------------------------------------//
	// Shadow mapping
	//-----------------------------------------------------------------//

	ShadowMap shadowMap
	(
		immediateContext,
		pDevice,
		WIDTH, HEIGHT,
		pShadowConstantBuffer,
		shadowInputLayout,
		vertexShadowShaderByteCode,
		ShadowVertexShader
	);

	shadowMap.SetProjectionMatrix(&light, pShadowConstantBuffer);


	//-----------------------------------------------------------------//
	// Load Models
	//-----------------------------------------------------------------//
	std::cout << " " << std::endl;
	std::cout << "Models are loading, this might take a while depending on your hardwere" << std::endl;

	//Mesh Objects
	Mesh heightPlaneMesh; 
	Mesh houseMesh;
	Mesh waterMesh;
	Mesh cubeMesh;
	Mesh eyeBall1;
	Mesh platformMesh;

	//File Paths
	std::wstring fileName = L"Objects/House.obj";
	std::wstring fileName2 = L"Objects/HightPlane.obj";
	std::wstring waterMeshPath = L"Objects/WaterMesh.obj";
	std::wstring cubePath = L"Objects/Boll.obj";
	std::wstring eyeOneFile = L"Objects/EyeBall1.obj";
	std::wstring platFormFile = L"Objects/Platform.obj";

	std::wstring filePath = L"Objects/";

	//Set file paths
	heightPlaneMesh.SetFilePath(filePath);
	houseMesh.SetFilePath(filePath);
	waterMesh.SetFilePath(filePath);
	cubeMesh.SetFilePath(filePath);
	eyeBall1.SetFilePath(filePath);
	platformMesh.SetFilePath(filePath);

	//Load models
	heightPlaneMesh.LoadObjModel(pDevice, fileName2, true);
	//houseMesh.loadObjModel(pDevice, fileName, true);
	waterMesh.LoadObjModel(pDevice, waterMeshPath, true);
	cubeMesh.LoadObjModel(pDevice, cubePath, true);
	eyeBall1.LoadObjModel(pDevice, eyeOneFile, true);
	platformMesh.LoadObjModel(pDevice, platFormFile, true);

	waterMesh.Animation(true);
	cubeMesh.Animation(true);
	cubeMesh.FollowMe(true);

	HeightMap heightMap("Objects/HeightMap.png");


	std::cout << "Models loaded" << std::endl;

	//-----------------------------------------------------------------//

	//Message pump
	MSG msg = {};
	BOOL gResult = {};

	//Deferred
	Deferred deffered;
	deffered.instalize(pDevice, WIDTH, HEIGHT, 0.001f, 15.0f);

	//Delta
	using ms = std::chrono::duration<float, std::milli>;
	std::chrono::high_resolution_clock clock;
	std::chrono::steady_clock timer;

	float deltaTime = 0.0f;
	auto start = clock.now();
	auto stop = clock.now();
	float time = 0;

	//-----------------------------------------------------------------//
	// Update
	//-----------------------------------------------------------------//

	std::cout << " " << std::endl;
	std::cout << "Move around: W A S D. SHIFT/SPACE to ascend/dscend" << std::endl;
	std::cout << "Move camera: Hold down LEFT CLICK on the MOUSE" << std::endl;
	std::cout << "While above/below the green terrain you'll get stuck on the ground. " << std::endl;
	std::cout << "You may close the application by closing the render window. " << std::endl;

	while (msg.message != WM_QUIT) //event queue
	{
		//TranslateMessage(&msg);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}	

		else
		{
			//-----------------------------------------------------------------//
			// Update Camera Y Position in respect to Height Map
			//-----------------------------------------------------------------//
			auto start = timer.now();

			setPerFrameMatrixes(perframeMatrixes, walkingCamera, immediateContext, pPerFrameConstantBuffer);

			float x = DirectX::XMVectorGetX(walkingCamera->getCameraPos());
			float y = walkingCamera->getGameraYPos();
			float z = DirectX::XMVectorGetZ(walkingCamera->getCameraPos());
			float newY = heightMap.getHegihtOnPosition(x, z);

			walkingCamera->setCameraY(newY);

			//-----------------------------------------------------------------//
			// Update matrixes
			//-----------------------------------------------------------------//
			
			light.cameraPosition = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.0f);
			walkingCamera->moveCameraWithInput();

			//-----------------------------------------------------------------//
			// Geomatry pass, Shadow pass, Light pass and Particle Update
			//-----------------------------------------------------------------//

			immediateContext->RSSetViewports(1, &viewPort); //Is needed at top always. 
			immediateContext->PSSetSamplers(1,1,shadowMap.depthMap.samplerState.GetAddressOf());
			shadowMap.depthMap.samplerState;
			immediateContext->PSSetSamplers(0, 1, &sampler);

			geomatryPass
			(
				immediateContext,
				viewPort,
				vertexShader,
				pixelShader,
				inputLayout,
				pPixelConstantBuffer,
				deffered,
				heightPlaneMesh,
				walkingCamera,
				houseMesh,
				geomatryShader,
				waterMesh,
				cubeMesh,
				pConstantBuffer,
				eyeBall1,
				platformMesh

			);
			

			//Stuff that happenes in the ShadowMap class.
			shadowMap.shadowPass(&light, pShadowConstantBuffer, ShadowVertexShader, shadowInputLayout); 

			//Stuff that happenes in the Mesh class.
			cubeMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			//houseMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			eyeBall1.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			platformMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);

			
			lightPass
			(
				immediateContext,
				renderTargetView,
				lightPassVertexShader,
				lightPassPixelShader,
				inputLayout,
				fullScreenVertexBuffer,
				pConstantBuffer,
				pPixelConstantBuffer,
				matrixes,
				deffered,
				light,
				shadowMap,
				pPerFrameConstantBuffer,
				pShadowConstantBuffer

			);
			


			particles.particlePass(immediateContext, walkingCamera);

			//Shows the front buffer
			pSwapChain->Present(1, 0);

			auto stop = timer.now();
			using ms = std::chrono::duration<float, std::milli>;
			deltaTime = std::chrono::duration_cast<ms>(stop - start).count();
		}
	}

	//-----------------------------------------------------------------//
	// Shut down
	//-----------------------------------------------------------------//

	ID3D11Debug* pDebug = nullptr;
	pDevice->QueryInterface(IID_PPV_ARGS(&pDebug));

	//Get rid of some of them memoryleaks

	rasStateNoCulling->Release();
	pSwapChain->Release();
	immediateContext->Release();
	renderTargetView->Release();
	vertexShader->Release();
	pixelShader->Release();
	geomatryShader->Release();
	ShadowVertexShader->Release();

	inputLayout->Release();
	shadowInputLayout->Release();

	pConstantBuffer->Release();
	pPixelConstantBuffer->Release();
	fullScreenVertexBuffer->Release();
	pShadowConstantBuffer->Release();
	pPerFrameConstantBuffer->Release();
	//sampler->Release();

	lightPassVertexShader->Release();
	lightPassPixelShader->Release();

	//----

	rasStateNoCulling = 0;
	
	pSwapChain = 0;
	immediateContext = 0;
	renderTargetView = 0;
	vertexShader = 0;
	pixelShader = 0;
	geomatryShader = 0;
	ShadowVertexShader = 0;

	inputLayout = 0;
	shadowInputLayout = 0;

	pConstantBuffer = 0;
	pPixelConstantBuffer = 0;
	fullScreenVertexBuffer = 0;
	pShadowConstantBuffer = 0;
	pPerFrameConstantBuffer = 0;
	//sampler = 0;

	lightPassVertexShader = 0;
	lightPassPixelShader = 0;

	delete walkingCamera;

	pDevice->Release();
	pDevice = 0;

	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	pDebug->Release();
	pDebug = 0;


	if (gResult == -1)
	{
		return -1;
	}
	else
	{
		return msg.wParam;
	}
}