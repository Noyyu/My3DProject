#include "Window.h"
//Message handler
//takes the window handle, message id. wParam: UINT_PTR unsigned int 32-bit, lParam
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam)
{

	//Switch on message id
	switch (msg)
	{
		//When the WM_CLOSE is sent, //Function that makes the application close as you close the window
	case WM_CLOSE:
		//post a quit message
		PostQuitMessage(60);
		break;

	case WM_ACTIVATEAPP:

		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		break;


	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		break;

	case WM_SYSKEYDOWN:
		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{

		}
		break;
	}
	
	return DefWindowProc(windowHandle, msg, wParam, lParam);
}

bool setUpWindow(UINT width, UINT height, HWND& window, int nCmdShow, HINSTANCE hInstance)
{
	// When we make a widnow we gotta take care of 2 tihngs: Widnow and Messages. 
	// Window need a class, this is NOT c++

	//Registers mouse device for raw input
	RAWINPUTDEVICE inputDevice;
	inputDevice.usUsagePage = 0x01; //mouse page
	inputDevice.usUsage = 0x02; //mouse usage
	inputDevice.dwFlags = RIDEV_NOLEGACY;
	inputDevice.hwndTarget = nullptr; //Will use whatever window we use at the moment. 
	RegisterRawInputDevices(&inputDevice, 1, sizeof(inputDevice)) == false;

	//Configuration structure for windowClass;
	//Create window class (config)
	const auto pClassName = "hw3dWindowClass";
	WNDCLASSEX windowClass = { 0 }; // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_OWNDC; // Gives every window that we render to its own device context (Usefull for if i wanna spawn more then one window)
	windowClass.lpfnWndProc = WindowProc; //Long pointer to function window proc: pointer to a function that handles all the messages for this window. Determens how the window is going to look basically.
	windowClass.cbClsExtra = 0; // Allocate extra bytes (we dont need that)
	windowClass.cbWndExtra = 0; // Allocate extra bytes for ever instance of window we create (again, we do not need that)
	windowClass.hInstance = hInstance;
	windowClass.hIcon = nullptr; // Custom icon or custom curser. 
	windowClass.hCursor = nullptr; // Custom icon or custom curser. 
	windowClass.hbrBackground = nullptr; // Brush that paints the window, we do not need that since we are going to fill the window with directX tihngs anyway.
	windowClass.lpszMenuName = nullptr; // Menu name, we are not going to use menues.
	windowClass.lpszClassName = pClassName; // ClassName..
	windowClass.hIconSm = nullptr; // Custom icon
	RegisterClassEx(&windowClass);

	//Creates the window
	window = CreateWindowEx
	(
		0, // EX Style, 0 = none
		pClassName, // Class name
		"WindOwO", // Window name
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, // This window will have a caption, a minimizing function and a sysmenu.
		200, 200, width, height, //first two: where to put window, second two: window dimentions. 
		nullptr, nullptr, // Handle to parent and handle to menu.
		hInstance, nullptr //handle to inctense and long pointer to custom paramater.
	);

	//Error check
	if (window == nullptr)
	{
		std::cerr << "Window faild, error code: " << GetLastError() << std::endl;
		return false;
	}

	//Shows the window
	ShowWindow(window, nCmdShow);

	return true;
}

bool setUpCMD()
{
	if (AllocConsole() == 0) {
		std::cerr << "Create console failed, last error: " << GetLastError() << std::endl;
		return false;
	}
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hConsole = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
	FILE* fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout); //Så cout syns i konsol fönstret
	freopen_s(&fp, "CONOUT$", "w", stderr); // Så cerr syns i konsol fönstret
}



