#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include "camera.h"
#include "Model.h"
#include "Input.h"
#include "scene_node.h"
#include <dinput.h>


//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;
ID3D11DepthStencilView* g_pZBuffer;      
ID3D11ShaderResourceView*	g_pTexture0;
ID3D11SamplerState* g_pSampler0;
ObjFileModel *pObject;
Camera* player;
Model* enemy;
Model* enemy2;
Model* obstacle;
Model* pick_up;
Model* pick_up2;
Model* pick_up3;
Model* cam_mod;
Model* goal;
Input* input;
scene_node* g_enemy_root_node;
scene_node* g_enemy_node1;
scene_node* g_enemy_node2;
scene_node* g_obstacle_root;
scene_node* g_obstacle_node1;
scene_node* g_pickup_root;
scene_node* g_pickup_node1;
scene_node* g_pickup2_root;
scene_node* g_pickup2_node1;
scene_node* g_pickup3_root;
scene_node* g_pickup3_node1;
scene_node* g_goal_root;
scene_node* g_goal_node1;
scene_node* cam_node;


// Rename for each tutorial
char		g_TutorialName[100] = "SWD304 - Gary Tizzard Assessment Member 2\0";

ID3D11Buffer*			g_pConstantBuffer0; 
ID3D11Buffer*			g_pVertexBuffer;                
ID3D11VertexShader*		g_pVertexShader;         
ID3D11PixelShader*		g_pPixelShader;          
ID3D11InputLayout*		g_pInputLayout;   

float r = 0;
float d = 0.1;


//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);

//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if(FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if(FAILED(InitialiseD3D()))
    {
 		DXTRACE_MSG("Failed to create Device");
		return 0;
    }

	if(FAILED(InitialiseGraphics())) 
	{
		DXTRACE_MSG( "Failed to initialise graphics" );
		return 0;
	}

	input = new Input(g_hInst, g_hWnd);
	input->InitInput();

	if(FAILED(input->InitInput()))
	{
		DXTRACE_MSG( "Failed to initialise input" );
		return 0;
	}
	
	// Main message loop
	MSG msg = {0};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// do something
			RenderFrame();
		}
	}

	ShutdownD3D();

	return (int) msg.wParam;
}
//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Gary Tizzard\0";

	// Register class
	WNDCLASSEX wcex={0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if(!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = {0, 0, 640, 480}; 
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(	Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, 
				rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if(!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch(message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
	
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, 
			createDeviceFlags, featureLevels, numFeatureLevels, 
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, 
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if(SUCCEEDED(hr))
			break;
	}

	if(FAILED(hr))
		return hr;

	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                      (LPVOID*)&pBackBufferTexture);

	if(FAILED(hr)) return hr;
	
	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
                                                    &g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if(FAILED(hr)) return hr;

	// Create a Z buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D *pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if(FAILED(hr)) return hr;

	// Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();


	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if(g_pVertexBuffer) g_pVertexBuffer->Release();
	if(g_pInputLayout) g_pInputLayout->Release();
	if(g_pVertexShader) g_pVertexShader->Release();
	if(g_pPixelShader) g_pPixelShader->Release();

	if(g_pConstantBuffer0) g_pConstantBuffer0->Release();
	if(g_pSwapChain) g_pSwapChain->Release();
	if(g_pTexture0) g_pTexture0->Release();	
	if(g_pSampler0) g_pSampler0->Release();
	if(g_pImmediateContext) g_pImmediateContext->Release();
	if(g_pD3DDevice) g_pD3DDevice->Release();
	if(g_pBackBufferRTView) g_pBackBufferRTView->Release();

	if(g_pZBuffer) g_pZBuffer->Release();

	input->~Input();

	delete player;
	delete pObject;
	delete enemy;
	delete enemy2;
	delete obstacle;
	delete pick_up;
	delete pick_up2;
	delete pick_up3;
	delete cam_mod;
	delete goal;
	delete g_enemy_root_node;
	delete g_enemy_node1;
	delete g_enemy_node2;
	delete g_obstacle_root;
	delete g_obstacle_node1;
	delete g_pickup_root;
	delete g_pickup_node1;
	delete g_pickup2_root;
	delete g_pickup2_node1;
	delete g_pickup3_root;
	delete g_pickup3_node1;
	delete g_goal_root;
	delete g_goal_node1;
	delete cam_node;
	
}

HRESULT InitialiseGraphics()
{
    HRESULT hr = S_OK;

	//create the objects inside the game
	enemy = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	enemy->LoadObjModel("assets/sphere.obj");
	enemy->setSampler(g_pSampler0);
	enemy->setTexture(g_pTexture0);

	enemy2 = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	enemy2->LoadObjModel("assets/sphere.obj");
	enemy2->setSampler(g_pSampler0);
	enemy2->setTexture(g_pTexture0);

	obstacle = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	obstacle->LoadObjModel("assets/cube.obj");
	obstacle->setSampler(g_pSampler0);
	obstacle->setTexture(g_pTexture0);

	pick_up = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	pick_up->LoadObjModel("assets/pyramid.obj");
	pick_up->setSampler(g_pSampler0);
	pick_up->setTexture(g_pTexture0);

	pick_up2 = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	pick_up2->LoadObjModel("assets/pyramid.obj");
	pick_up2->setSampler(g_pSampler0);
	pick_up2->setTexture(g_pTexture0);

	pick_up3 = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	pick_up3->LoadObjModel("assets/pyramid.obj");
	pick_up3->setSampler(g_pSampler0);
	pick_up3->setTexture(g_pTexture0);

	cam_mod = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	cam_mod->LoadObjModel("assets/cube.obj");
	cam_mod->setSampler(g_pSampler0);
	cam_mod->setTexture(g_pTexture0);

	goal = new Model(g_pD3DDevice, g_pImmediateContext, NULL, NULL);
	goal->LoadObjModel("assets/PointySphere.obj");
	goal->setSampler(g_pSampler0);
	goal->setTexture(g_pTexture0);

	//creating camera object
	player = new Camera(0.0, 0.0, -20, 0);

	// Creating the scene nodes
	g_enemy_root_node = new scene_node();
	g_enemy_node1 = new scene_node();
	g_enemy_node2 = new scene_node();
	g_obstacle_root = new scene_node();
	g_obstacle_node1 = new scene_node();
	g_pickup_root = new scene_node();
	g_pickup_node1 = new scene_node();
	g_pickup2_root = new scene_node();
	g_pickup2_node1 = new scene_node();
	g_pickup3_root = new scene_node();
	g_pickup3_node1 = new scene_node();
	g_goal_root = new scene_node();
	g_goal_node1 = new scene_node();
	cam_node = new scene_node();

	// Associate the nodes with their models
	g_enemy_node1->setModel(enemy);
	g_enemy_node2->setModel(enemy2);
	g_obstacle_node1->setModel(obstacle);
	g_pickup_node1->setModel(pick_up);
	g_pickup2_node1->setModel(pick_up2);
	g_pickup3_node1->setModel(pick_up3);
	g_goal_node1->setModel(goal);
	cam_node->setModel(cam_mod);

	// Create the enemy graph
	g_enemy_root_node->addChildNode(g_enemy_node1);
	g_enemy_node1->addChildNode(g_enemy_node2);
	
	// attach the camera node to the enemy graph 
	g_enemy_root_node->addChildNode(cam_node);

	// Create obstacle graph
	g_obstacle_root->addChildNode(g_obstacle_node1);

	// create the pickup graphs
	g_pickup_root->addChildNode(g_pickup_node1);
	
	g_pickup2_root->addChildNode(g_pickup2_node1);

	g_pickup3_root->addChildNode(g_pickup3_node1);

	//create the goal graph
	g_goal_root->addChildNode(g_goal_node1);
	
	// Initialise start positions 
	g_enemy_node1->setXPos(-5);
	g_enemy_node1->setYPos(0);
	g_enemy_node2->setYPos(4);

	g_obstacle_node1->setXPos(10);
	g_obstacle_node1->setYPos(0);
	g_obstacle_node1->setScale(1.5);

	g_pickup_node1->setYPos(-2);
	g_pickup_node1->setXPos(20);
	g_pickup_node1->setScale(0.1);

	g_pickup2_node1->setYPos(-2);
	g_pickup2_node1->setXPos(30);
	g_pickup2_node1->setScale(0.1);

	g_pickup3_node1->setYPos(-2);
	g_pickup3_node1->setXPos(40);
	g_pickup3_node1->setScale(0.1);

	cam_node->setScale(0.6);
	cam_node->setZPos(-20);
	cam_node->setYPos(3);

	g_goal_node1->setXPos(40);
	g_goal_node1->setZPos(40);
	
	
    return S_OK;

 }

void Movement(void)
{
	if(input->IsKeyPressed(DIK_A))
	{
		r = (r - 0.15);
		player->Rotate(r);
	}

	if(input->IsKeyPressed(DIK_D))
	{
		r = (r + 0.15);
		player->Rotate(r);
	}

	if(input->IsKeyPressed(DIK_W))
	{
		player->Forward(0.1);

		// set camera node to the position of the camera
		cam_node->setXPos(player->getX());
		cam_node->setYPos(player->getY());
		cam_node->setZPos(player->getZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update trees to reflect new camera position
		g_enemy_root_node->update_collision_tree(&identity, 1.0);
		g_obstacle_root->update_collision_tree(&identity, 1.0);
		g_pickup_root->update_collision_tree(&identity, 1.0);
		g_pickup2_root->update_collision_tree(&identity, 1.0);
		g_pickup3_root->update_collision_tree(&identity, 1.0);
		g_goal_root->update_collision_tree(&identity, 1.0);

		// Test collisions with enemies
		if(cam_node->check_collision(g_enemy_root_node) == true) 
		{
			// if there is a collision, restore camera and camera node positions
			player->Forward(-0.1);
			player->loselife(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
		}
		// Test collisions with obstacles in the level
		if(cam_node->check_collision(g_obstacle_root) == true)
		{
			player->Forward(-0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
		}
		// Test collisions with the keys
		if(cam_node->check_collision(g_pickup_root) == true)
		{
			player->Forward(-0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup_node1->setXPos(50);
			g_pickup_node1->setYPos(100);
		}
		if(cam_node->check_collision(g_pickup2_root) == true)
		{
			player->Forward(-0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup2_node1->setXPos(60);
			g_pickup2_node1->setYPos(100);
		}
		if(cam_node->check_collision(g_pickup3_root) == true)
		{
			player->Forward(-0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup3_node1->setXPos(70);
			g_pickup3_node1->setYPos(100);
		}
		if(cam_node->check_collision(g_goal_root) == true)
		{
			player->Forward(-0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			
			// Check to see if the player has all of the keys
			if(player->getKeys() >= 3)
			{
				// operations to be performed when all keys are collected
				DestroyWindow(g_hWnd);
			}
			
		}
	}

	if(input->IsKeyPressed(DIK_S))
	{
		player->Forward(-0.1);

		// set camera node to the position of the camera
		cam_node->setXPos(player->getX());
		cam_node->setYPos(player->getY());
		cam_node->setZPos(player->getZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_enemy_root_node->update_collision_tree(&identity, 1.0);
		g_obstacle_root->update_collision_tree(&identity, 1.0);
		g_pickup_root->update_collision_tree(&identity, 1.0);
		g_pickup2_root->update_collision_tree(&identity, 1.0);
		g_pickup3_root->update_collision_tree(&identity, 1.0);
		g_goal_root->update_collision_tree(&identity, 1.0);

		if(cam_node->check_collision(g_enemy_root_node) == true) 
		{
			// if there is a collision, restore camera and camera node positions
			player->Forward(0.1);	
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
		}
		if(cam_node->check_collision(g_obstacle_root) == true)
		{
			player->Forward(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
		}
		if(cam_node->check_collision(g_pickup_root) == true)
		{
			player->Forward(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup_node1->setXPos(40);
			g_pickup_node1->setYPos(150);
		}
		if(cam_node->check_collision(g_pickup2_root) == true)
		{
			player->Forward(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup2_node1->setXPos(20);
			g_pickup2_node1->setYPos(150);
		}
		if(cam_node->check_collision(g_pickup3_root) == true)
		{
			player->Forward(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			 
			player->addKey();
			// move the object out of the playable space to ensure it isn't picked up multiple times
			g_pickup3_node1->setXPos(0);
			g_pickup3_node1->setYPos(150);
		}
		if(cam_node->check_collision(g_goal_root) == true)
		{
			player->Forward(0.1);
			cam_node->setXPos(player->getX());
			cam_node->setYPos(player->getY());
			cam_node->setZPos(player->getZ());
			
			// Check to see if the player has all of the keys
			if(player->getKeys() >= 3)
			{
				// operations to be performed when all keys are collected
				DestroyWindow(g_hWnd);
			}
			
		}
	}


}

// Render frame
void RenderFrame(void)
{
		
	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.0f, 0.5f, 0.1f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	XMMATRIX projection, view;


	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0/480.0, 1.0, 100.0);
	view = player->GetViewMatrix();

	//Input states
	input->ReadInputStates();

	// Escape to close game
	if(input->IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);

	// Check to see if the player has any life left
	if(player->getLives() <= 0)
	{	
		// operations to be performed when the player has no lives left
		DestroyWindow(g_hWnd);
	}

	Movement();


	// draw the different scenes
	g_enemy_root_node->execute(&XMMatrixIdentity(), &view, &projection);
	g_obstacle_root->execute(&XMMatrixIdentity(), &view, &projection);
	g_pickup_root->execute(&XMMatrixIdentity(), &view, &projection);
	g_pickup2_root->execute(&XMMatrixIdentity(), &view, &projection);
	g_pickup3_root->execute(&XMMatrixIdentity(), &view, &projection);
	g_goal_root->execute(&XMMatrixIdentity(), &view, &projection);



	// Select which primtive type to use
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}
