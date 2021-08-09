//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

#include <Windows.h>
#include "Window.h"
#include "Graphics.h"
#include "Camera.h"
#include "DeferredRendering.h"
#include "Structs.h"
#include "Mesh.h"
#include "BasicLoader.h"
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
void geomatryPass(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* renderTargetView,
	ID3D11DepthStencilView* depthView, D3D11_VIEWPORT& viewport, ID3D11VertexShader* vertexShader,
	ID3D11PixelShader* pixelShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* vertexBuffer,
	ID3D11Buffer*& pConstantBuffer, ID3D11ShaderResourceView* textureSRV,
	ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, 
	Deferred deferred, Mesh& objObject, Camera* camera, Mesh& objObject2, ID3D11GeometryShader* geomatryShader, 
	ID3D11InputLayout* geomatryInputLayout, Mesh& WaterMesh, Mesh& cubeMesh, ID3D11Buffer*& perFrameConstantBuffer,
	Mesh eyeOne, Mesh platformMesh)
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

	//immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	

	//immediateContext->VSSetShader(vertexShader, nullptr, 0);
	immediateContext->GSSetShader(geomatryShader, nullptr, 0);
	//immediateContext->PSSetShader(pixelShader, nullptr, 0);

	//immediateContext->PSSetShaderResources(0, 1, &textureSRV);
	//immediateContext->PSSetSamplers(0, 1, &sampler);

	//immediateContext->PSSetConstantBuffers(0u, 1, &pConstantBuffer);
	//immediateContext->VSSetConstantBuffers(0u, 1, &pConstantBuffer);

	//immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);

	deferred.setRenderTargets(immediateContext);
	//immediateContext->Draw(6, 0);

	objObject.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
	objObject2.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
	WaterMesh.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
	cubeMesh.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);

	eyeOne.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
	platformMesh.drawObjModel(immediateContext, perFrameConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
}

void lightPass(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* renderTargetView,
	ID3D11DepthStencilView* depthView, D3D11_VIEWPORT& viewport, ID3D11VertexShader* lightPassVertexShader,
	ID3D11PixelShader* lightPassPixelShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* fullScreenVertexBuffer,
	ID3D11Buffer*& pConstantBuffer, ID3D11ShaderResourceView* textureSRV,
	ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, Deferred& deferred, Light light, Mesh &objObject, ShadowMap shadowObject, Camera* camera, ID3D11Buffer*& perFrameConstantBuffer, ID3D11Buffer*& pShadowConstantBuffer)
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

	immediateContext->VSSetShader(lightPassVertexShader, nullptr, 0);
	immediateContext->GSSetShader(nullptr, nullptr, 0);
	deferred.setShaderResourceView(immediateContext, shadowObject.depthMap.shaderResourceView.Get());
	immediateContext->PSSetShader(lightPassPixelShader, nullptr, 0);

	immediateContext->UpdateSubresource(pPixelConstantBuffer, 0, NULL, &light, 0, 0);
	immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);


	//DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1, 1, 1);
	//DirectX::XMMATRIX translate = DirectX::XMMatrixTranslation(0, 0, 0); // X+ = >, Z+ = ^
	//DirectX::XMMATRIX world = scale * translate;

	//matrixes.World = matrixFunction.setWorld(world);
	//matrixes.WorldViewProjection = matrixFunction.setWVP(world * camera->getCameraView() * camera->getCameraProjection());

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
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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
	ID3D11RasterizerState*   rasState            = nullptr;
	ID3D11RasterizerState*   rasStateNoCulling   = nullptr;
	ID3D11Device*            pDevice             = nullptr;
	IDXGISwapChain*          pSwapChain          = nullptr;
	ID3D11DeviceContext*     immediateContext    = nullptr;
	ID3D11RenderTargetView*  renderTargetView    = nullptr;
	ID3D11Texture2D*         depthTexture        = nullptr;
	ID3D11DepthStencilView*  depthView           = nullptr;
	ID3D11VertexShader*      vertexShader        = nullptr;
	ID3D11PixelShader*       pixelShader         = nullptr;
	ID3D11GeometryShader*    geomatryShader      = nullptr;  // Back face culling
	ID3D11VertexShader*      ShadowVertexShader  = nullptr;  // Shadow shader
	
	ID3D11InputLayout*       inputLayout         = nullptr;
	ID3D11InputLayout*		 shadowInputLayout   = nullptr;
	ID3D11InputLayout*       geomatryInputLayout = nullptr;  // Back face culling
	ID3D11Buffer*            vertexBuffer        = nullptr;  // For the spinning quad
	D3D11_VIEWPORT           viewPort;
	std::string              vertexShaderByteCode;
	std::string              vertexShadowShaderByteCode;       // Shadow map
	ID3D11Buffer*            pConstantBuffer        = nullptr; // Interface pointer
	ID3D11Buffer*            pPixelConstantBuffer   = nullptr;
	ID3D11Buffer*            fullScreenVertexBuffer = nullptr; // Fullscreen quad
	ID3D11Buffer*			 pShadowConstantBuffer  = nullptr; // Shadow map
	ID3D11Buffer*            pPerFrameConstantBuffer = nullptr;

	//Texture
	ID3D11Texture2D*          texture                   = nullptr;
	ID3D11ShaderResourceView* textureShaderResourceView = nullptr;
	ID3D11SamplerState*       sampler                   = nullptr;

	//Deferred things
	ID3D11Texture2D*          fullScreentexture                   = nullptr;
	ID3D11ShaderResourceView* fullScreenTextureShaderResourceView = nullptr;
	ID3D11SamplerState*       fullScreenSampler                   = nullptr;

	ID3D11VertexShader*       lightPassVertexShader = nullptr;
	ID3D11PixelShader*        lightPassPixelShader  = nullptr;

	ID3D11InputLayout*        lightPassInputLayout  = nullptr;
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
	//I'll use this as my shadow light. 
	Light light;
	light.position       = DirectX::XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); // X+ = >, Z+ = ^
	light.attenuation    = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	light.ambient        = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.diffuse        = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.cameraPosition = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.0f);
	light.direction      = DirectX::XMFLOAT4(0.0f, -0.99995f, 0.009999f, 0.0f);
	light.range = 100.0f;


	//-----------------------------------------------------------------//
	// Spinning quad stuff
	//-----------------------------------------------------------------//

	//Geomatry Stuff
	float currentRotation = 0.001;
	DirectX::XMConvertToRadians(currentRotation);

	//The offset for the spin
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f);
	DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.5f);

	//Rotation matrix
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(currentRotation));


	//-----------------------------------------------------------------//
	// Load grapgics class
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
		depthTexture,
		depthView,
		viewPort,
		vertexShader,
		pixelShader,
		vertexShaderByteCode,
		vertexBuffer,
		inputLayout,
		pConstantBuffer,
		texture,
		textureShaderResourceView,
		sampler,
		light,
		pPixelConstantBuffer,
		matrixes,
		fullScreenVertexBuffer,
		lightPassVertexShader,
		lightPassPixelShader,
		lightPassInputLayout,
		lightPassVertexShaderByteCode,
		rasState,
		rasStateNoCulling,
		geomatryShader,
		geomatryInputLayout,
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
	// OBJ Matrixes
	//-----------------------------------------------------------------//

	//objMatrixes objMatrixes = {}; //Jag tror inte detta behövs
	//objMatrixes.objRotation = DirectX::XMMatrixRotationY(3.14f);
	//objMatrixes.objScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	//objMatrixes.objTranslation = DirectX::XMMatrixTranslation(0.0f, 5.0f, 0.0f);
	//objMatrixes.cameraPosition = DirectX::XMMatrixIdentity();
	//objMatrixes.cameraProjection = DirectX::XMMatrixIdentity();
	//objMatrixes.cameraView = DirectX::XMMatrixIdentity();


	//-----------------------------------------------------------------//
	// Load Models
	//-----------------------------------------------------------------//

	//Mesh Objects
	ID3D11SamplerState* objSampler;
	Mesh heightPlaneMesh(pDevice); 
	Mesh houseMesh(pDevice);
	Mesh waterMesh(pDevice);
	Mesh cubeMesh(pDevice);
	Mesh eyeBall1(pDevice);
	Mesh platformMesh(pDevice);

	//File Paths
	std::wstring fileName = L"Objects/House.obj";
	std::wstring fileName2 = L"Objects/HightPlane.obj";
	std::wstring waterMeshPath = L"Objects/WaterMesh.obj";
	std::wstring cubePath = L"Objects/Boll.obj";
	std::wstring eyeOneFile = L"Objects/EyeBall1.obj";
	std::wstring platFormFile = L"Objects/Platform.obj";

	std::wstring filePath = L"Objects/";

	//Set file paths
	heightPlaneMesh.setFilePath(filePath);
	houseMesh.setFilePath(filePath);
	waterMesh.setFilePath(filePath);
	cubeMesh.setFilePath(filePath);
	eyeBall1.setFilePath(filePath);
	platformMesh.setFilePath(filePath);

	//Load models
	heightPlaneMesh.loadObjModel(pDevice, fileName2, false, true);
	houseMesh.loadObjModel(pDevice, fileName, false, true); //False = normalerna hamnar på fel håll men specular funkar??
	waterMesh.loadObjModel(pDevice, waterMeshPath, false, true);
	cubeMesh.loadObjModel(pDevice, cubePath, false, true);
	eyeBall1.loadObjModel(pDevice, eyeOneFile, false, true);
	platformMesh.loadObjModel(pDevice, platFormFile, false, true);

	waterMesh.Animation(true);
	cubeMesh.Animation(true);
	cubeMesh.FollowMe(true);

	HeightMap heightMap("Objects/HeightMap.png");

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

	if (windowHandle)
	{

	}
	while (msg.message != WM_QUIT) //event queue
	{
		//TranslateMessage(&msg);
		if (PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		else
		{
			//-----------------------------------------------------------------//
			// Update Camera Y Position in respect to Height Map
			//-----------------------------------------------------------------//


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

			auto start = timer.now();

			//Matrix and time stuff
			rotationMatrix = translation * DirectX::XMMatrixRotationY(currentRotation * deltaTime) * translation2;

			//rotationMatrix = DirectX::XMMatrixTranslation(0.0f, 0.01f, 0.0f);

			World = rotationMatrix * World;
			WorldViewProjection = World * walkingCamera->getCameraView() * walkingCamera->getCameraProjection();

			time += 1;
			matrixes.World = matrixFunction.setWorld(World);
			matrixes.WorldViewProjection = matrixFunction.setWVP(WorldViewProjection);
			matrixes.time = time;

			immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);

			//-----------------------------------------------------------------//
			// Geomatry pass and Light pass Update
			//-----------------------------------------------------------------//

			immediateContext->RSSetViewports(1, &viewPort); //Is needed at top always. 
			immediateContext->PSSetSamplers(1,1,shadowMap.depthMap.samplerState.GetAddressOf());
			shadowMap.depthMap.samplerState;


			geomatryPass
			(
				immediateContext,
				renderTargetView,
				depthView,
				viewPort,
				vertexShader,
				pixelShader,
				inputLayout,
				vertexBuffer,
				pConstantBuffer,
				textureShaderResourceView,
				sampler,
				pPixelConstantBuffer,
				matrixes,
				deffered,
				heightPlaneMesh,
				walkingCamera,
				houseMesh,
				geomatryShader,
				geomatryInputLayout,
				waterMesh,
				cubeMesh,
				pPerFrameConstantBuffer,
				eyeBall1,
				platformMesh

			);
			

			//Draw shadow here maybe?
			shadowMap.shadowPass(&light, pShadowConstantBuffer, ShadowVertexShader, shadowInputLayout); //Stuff that happenes in the ShadowMap class.
			//Stuff that happenes in the Mesh class.
			cubeMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			houseMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			//waterMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			//heightPlaneMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			eyeBall1.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			platformMesh.DrawShadow(immediateContext, walkingCamera, pPerFrameConstantBuffer);
			
			lightPass
			(
				immediateContext,
				renderTargetView,
				depthView,
				viewPort,
				lightPassVertexShader,
				lightPassPixelShader,
				inputLayout,
				fullScreenVertexBuffer,
				pConstantBuffer,
				textureShaderResourceView,
				sampler,
				pPixelConstantBuffer,
				matrixes,
				deffered,
				light,
				heightPlaneMesh,
				shadowMap,
				walkingCamera, 
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
	deffered.shutDownDeferredObjects();
	shadowMap.ShutDownShadows();
	heightPlaneMesh.shutDownMesh();
	houseMesh.shutDownMesh();
	waterMesh.shutDownMesh();
	cubeMesh.shutDownMesh();

	rasState->Release();
	rasStateNoCulling->Release();
	pDevice->Release();
	pSwapChain->Release();
	immediateContext->Release();
	renderTargetView->Release();
	depthTexture->Release();
	depthView->Release();
	vertexShader->Release();
	pixelShader->Release();
	geomatryShader->Release();
	ShadowVertexShader->Release();

	inputLayout->Release();
	shadowInputLayout->Release();
	geomatryInputLayout->Release();

	vertexBuffer->Release();
	pConstantBuffer->Release();
	pPixelConstantBuffer->Release();
	fullScreenVertexBuffer->Release();
	pShadowConstantBuffer->Release();
	pPerFrameConstantBuffer->Release();
	
	texture->Release();
	textureShaderResourceView->Release();
	sampler->Release();

	fullScreentexture->Release();
	fullScreenTextureShaderResourceView->Release();
	fullScreenSampler->Release();

	lightPassVertexShader->Release();
	lightPassPixelShader->Release();
	lightPassInputLayout->Release();

	//----

	rasState = 0;
	rasStateNoCulling = 0;
	pDevice = 0;
	pSwapChain = 0;
	immediateContext = 0;
	renderTargetView = 0;
	depthTexture = 0;
	depthView = 0;
	vertexShader = 0;
	pixelShader = 0;
	geomatryShader = 0;
	ShadowVertexShader = 0;

	inputLayout = 0;
	shadowInputLayout = 0;
	geomatryInputLayout = 0;

	vertexBuffer = 0;
	pConstantBuffer = 0;
	pPixelConstantBuffer = 0;
	fullScreenVertexBuffer = 0;
	pShadowConstantBuffer = 0;
	pPerFrameConstantBuffer = 0;

	texture = 0;
	textureShaderResourceView = 0;
	sampler = 0;

	fullScreentexture = 0;
	fullScreenTextureShaderResourceView = 0;
	fullScreenSampler = 0;

	lightPassVertexShader = 0;
	lightPassPixelShader = 0;
	lightPassInputLayout = 0;
	


	delete walkingCamera;


	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	pDebug->Release();

	return 0;
}