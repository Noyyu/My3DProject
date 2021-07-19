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

//Sending everytihng to the immidiate context (device context) ((screen))
void geomatryPass(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* renderTargetView,
	ID3D11DepthStencilView* depthView, D3D11_VIEWPORT& viewport, ID3D11VertexShader* vertexShader,
	ID3D11PixelShader* pixelShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* vertexBuffer,
	ID3D11Buffer*& pConstantBuffer, ID3D11ShaderResourceView* textureSRV,
	ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, 
	Deferred deferred, Mesh& objObject, objMatrixes objMats, Camera* camera, Meshs testobject, Mesh& objObject2, ID3D11GeometryShader* geomatryShader, ID3D11InputLayout* geomatryInputLayout)
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

	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	

	immediateContext->VSSetShader(vertexShader, nullptr, 0);
	immediateContext->GSSetShader(geomatryShader, nullptr, 0);
	immediateContext->PSSetShader(pixelShader, nullptr, 0);

	//immediateContext->IASetInputLayout(geomatryInputLayout);
	

	//immediateContext->IASetInputLayout(inputLayout); //Go back to the original input layout. 

	immediateContext->PSSetShaderResources(0, 1, &textureSRV);
	immediateContext->PSSetSamplers(0, 1, &sampler);

	immediateContext->PSSetConstantBuffers(0u, 1, &pConstantBuffer);
	immediateContext->VSSetConstantBuffers(0u, 1, &pConstantBuffer);

	//matrixes.setHasTexture(true);

	immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);


	deferred.setRenderTargets(immediateContext);
	immediateContext->Draw(6, 0);

	//Test object
	immediateContext->IASetIndexBuffer(testobject.ib.Get(), DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetVertexBuffers(0, 1, testobject.vb.GetAddressOf(), &stride, &offset);
	immediateContext->Draw(testobject.ib.getIndexCount(), 0);

	

	objObject.drawObjModel(immediateContext, pConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
	objObject2.drawObjModel(immediateContext, pConstantBuffer, deferred, vertexShader, pixelShader, sampler, pPixelConstantBuffer, camera);
}

void lightPass(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* renderTargetView,
	ID3D11DepthStencilView* depthView, D3D11_VIEWPORT& viewport, ID3D11VertexShader* lightPassVertexShader,
	ID3D11PixelShader* lightPassPixelShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* fullScreenVertexBuffer,
	ID3D11Buffer*& pConstantBuffer, ID3D11ShaderResourceView* textureSRV,
	ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, Deferred& deferred, Light light, Mesh &objObject, objMatrixes objMats)
{
	
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
	deferred.setShaderResourceView(immediateContext);
	immediateContext->PSSetShader(lightPassPixelShader, nullptr, 0);

	immediateContext->UpdateSubresource(pPixelConstantBuffer, 0, NULL, &light, 0, 0);
	immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &matrixes, 0, 0);

	immediateContext->VSSetConstantBuffers(0u, 1, &pConstantBuffer);
	immediateContext->PSSetConstantBuffers(0u, 1, &pPixelConstantBuffer);
	immediateContext->PSSetConstantBuffers(1u, 1, &pConstantBuffer);

	
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

	//-----------------------------------------------------------------//
	// Setup window
	//-----------------------------------------------------------------//

	const UINT WIDTH = 640;
	const UINT HEIGHT = 480;

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

	Camera* walkingCamera = new Camera({ 1.0f,0.0f, 2.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, WIDTH, HEIGHT);
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
	ID3D11Buffer*            vertexBuffer        = nullptr;
	D3D11_VIEWPORT           viewPort;
	std::string              vertexShaderByteCode;
	std::string              vertexShadowShaderByteCode;       // Shadow map
	ID3D11Buffer*            pConstantBuffer        = nullptr; // Interface pointer
	ID3D11Buffer*            pPixelConstantBuffer   = nullptr;
	ID3D11Buffer*            fullScreenVertexBuffer = nullptr; // Fullscreen quad
	ID3D11Buffer*			 pShadowConstantBuffer  = nullptr; // Shadow map

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

	//Gaussian blur stuff
	ID3D11UnorderedAccessView* backBuffer = nullptr; // Delete
	ID3D11ComputeShader* computeShader    = nullptr; // Delete

	//matrixsstuff
	constantBufferMatrixes matrixes;//used to send the world matrix and worldviewprojection matrixes to the shader later
	DirectX::XMMATRIX WorldViewProjection = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	//-----------------------------------------------------------------//
	// Light attributes
	//-----------------------------------------------------------------//
	//I'll use this as my shadow light. 
	Light light;
	light.position = DirectX::XMFLOAT4(2.0f, 5.0f, 10.0f, 1.0f);
	light.ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	light.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.attenuation = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	light.cameraPosition = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.0f);
	light.range = 5.0f;


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
		geomatryInputLayout

	);

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

	objMatrixes objMatrixes = {}; //Jag tror inte detta beh�vs
	objMatrixes.objRotation = DirectX::XMMatrixRotationY(3.14f);
	objMatrixes.objScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	objMatrixes.objTranslation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	objMatrixes.cameraPosition = DirectX::XMMatrixIdentity();
	objMatrixes.cameraProjection = DirectX::XMMatrixIdentity();
	objMatrixes.cameraView = DirectX::XMMatrixIdentity();


	//-----------------------------------------------------------------//
	// Load Models
	//-----------------------------------------------------------------//

	ID3D11SamplerState* objSampler;
	Mesh objObject(pDevice); 
	Mesh objObject2(pDevice);
	Meshs testObject;//Remove basic import

	std::wstring fileName = L"C:/Users/Nikki/OneDrive/Skrivbord/Skolan/Flowie.obj";
	std::wstring fileName2 = L"C:/Users/Nikki/OneDrive/Skrivbord/Skolan/HightPlane.obj";
	std::wstring filePath = L"C:/Users/Nikki/OneDrive/Skrivbord/Skolan/";
	
	objObject.setFilePath(filePath);
	objObject2.setFilePath(filePath);
	objObject.loadObjModel(pDevice, fileName2, true, true);
	objObject2.loadObjModel(pDevice, fileName, false, true);

	HeightMap heightMap("C:/Users/Nikki/OneDrive/Skrivbord/Skolan/HeightMap.png");

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

			float x = DirectX::XMVectorGetX(walkingCamera->getCameraPos());
			float y = walkingCamera->getGameraYPos();
			float z = DirectX::XMVectorGetZ(walkingCamera->getCameraPos());
			float newY = heightMap.getHegihtOnPosition(x, z);

			walkingCamera->setCameraY(newY);


			//-----------------------------------------------------------------//
			// Update matrixes
			//-----------------------------------------------------------------//
			
			light.updateCameraPosition(DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()),1.0f));

			walkingCamera->moveCameraWithInput();
			/*RedirectIOToConsole(walkingCamera->getMouseState());*/

			auto start = timer.now();

			//Matrix and time stuff
			rotationMatrix = translation * DirectX::XMMatrixRotationY(currentRotation * deltaTime) * translation2;

			World = rotationMatrix * World;
			WorldViewProjection = World * walkingCamera->getCameraView() * walkingCamera->getCameraProjection();

			time += 1;

			matrixes.setWorld(World);
			matrixes.setWVP(WorldViewProjection);
			matrixes.setTime(time);

			//-----------------------------------------------------------------//
			// Geomatry pass and Light pass Update
			//-----------------------------------------------------------------//

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
				objObject,
				objMatrixes, 
				walkingCamera,
				testObject,
				objObject2,
				geomatryShader,
				geomatryInputLayout
			);

			//Draw shadow here maybe?
			//shadowMap.shadowPass(&light, pShadowConstantBuffer, ShadowVertexShader, shadowInputLayout);

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
				objObject,
				objMatrixes
			);
			

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

	//Get rid of some of them memoryleaks
	deffered.shutDownDeferredObjects();

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
	pPixelConstantBuffer->Release();
	pShadowConstantBuffer->Release();
	
	texture->Release();
	textureShaderResourceView->Release();
	sampler->Release();
	fullScreentexture->Release();
	fullScreenTextureShaderResourceView->Release();
	fullScreenSampler->Release();

	lightPassVertexShader->Release();
	lightPassPixelShader->Release();
	lightPassInputLayout->Release();
	
	ID3D11Debug* pDebug = nullptr;
	pDevice->QueryInterface(IID_PPV_ARGS(&pDebug));
	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	pDebug->Release();

	delete walkingCamera;

	return 0;
}