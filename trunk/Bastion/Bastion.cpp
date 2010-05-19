// Bastion.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <vld.h>

//#ifdef _DEBUG
//#include <crtdbg.h>
//#endif

#include "../Bastion/Bastion.h"
#include "../Application/Application.h"

using namespace BastionGame;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HACCEL hAccelTable;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(GraphicConfigDataRef _rConfig);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
HWND				MyCreateWindow(GraphicConfigDataRef _rConfig);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

//#ifdef _DEBUG
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//	//_CrtSetBreakAlloc(227);
//#endif

	GraphicConfigData m_oGraphicConfig;
	Application* pApp = new Application;

	// default values
	m_oGraphicConfig.m_pCreateWindow = MyCreateWindow;
	m_oGraphicConfig.m_hInstance = hInstance;
	m_oGraphicConfig.m_hPrevInstance = hPrevInstance;
	m_oGraphicConfig.m_lpCmdLine = lpCmdLine;
	m_oGraphicConfig.m_nCmdShow = nCmdShow;
	m_oGraphicConfig.m_oClientRect.left = 0;
	m_oGraphicConfig.m_oClientRect.right = 0;
	m_oGraphicConfig.m_oClientRect.right = 640;
	m_oGraphicConfig.m_oClientRect.bottom = 480;
	m_oGraphicConfig.m_uDXColorFormat = D3DFMT_A8R8G8B8;
	m_oGraphicConfig.m_uDXDepthFormat = D3DFMT_D24S8;
	m_oGraphicConfig.m_uDXGBufferFormat = D3DFMT_A8R8G8B8;
	m_oGraphicConfig.m_uDXGBufferCount = 1;
	m_oGraphicConfig.m_fZNear = 1.0f;
	m_oGraphicConfig.m_fZFar = 1000.0f;
	m_oGraphicConfig.m_bFullScreen = false;

	if (false != pApp->Create(boost::any(&m_oGraphicConfig)))
	{
		do
		{
			pApp->Update();
		}
		while (Application::EStateMode_QUIT != pApp->GetStateMode());
	}

	CoreObject::ReleaseDeleteReset(pApp);

	return 0;
}

HWND MyCreateWindow(GraphicConfigDataRef _rConfig)
{
	// Initialize global strings
	LoadString(_rConfig.m_hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(_rConfig.m_hInstance, IDC_BASTION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(_rConfig.m_hInstance);

	// Perform application initialization:
	InitInstance(_rConfig);

	if (NULL != _rConfig.m_hWnd)
	{
		_rConfig.m_hAccelTable = LoadAccelerators(_rConfig.m_hInstance, MAKEINTRESOURCE(IDC_BASTION));
	}

	return _rConfig.m_hWnd;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BASTION));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_BASTION);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(GraphicConfigData)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(GraphicConfigDataRef _rConfig)
{
   hInst = _rConfig.m_hInstance; // Store instance handle in our global variable

   WINDOWINFO oDesktopWI;
   GetWindowInfo( GetDesktopWindow(), &oDesktopWI );

   _rConfig.m_oGraphicConfigRect = _rConfig.m_oClientRect;

   // Resizes window so that client size has the specified video width/height.
   AdjustWindowRect( &_rConfig.m_oGraphicConfigRect, WS_OVERLAPPEDWINDOW, TRUE );
   _rConfig.m_oGraphicConfigRect.right -= _rConfig.m_oGraphicConfigRect.left;
   _rConfig.m_oGraphicConfigRect.bottom -= _rConfig.m_oGraphicConfigRect.top;
   // Set window positions in the middle of the first monitor.
   EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, LPARAM(&_rConfig.m_oGraphicConfigRect) );

   _rConfig.m_hWnd = CreateWindow(szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW,
	   _rConfig.m_oGraphicConfigRect.left,
	   _rConfig.m_oGraphicConfigRect.top,
	   _rConfig.m_oGraphicConfigRect.right,
	   _rConfig.m_oGraphicConfigRect.bottom,
	   NULL,
	   NULL,
	   _rConfig.m_hInstance,
	   NULL);

   if (!_rConfig.m_hWnd)
   {
      return NULL;
   }

   ShowWindow(_rConfig.m_hWnd, _rConfig.m_nCmdShow);
   UpdateWindow(_rConfig.m_hWnd);

   return _rConfig.m_hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		{
			ApplicationPtr pApp = (ApplicationPtr)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (NULL != pApp)
			{
				pApp->SetMousePos(float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	// search for the first monitor
	if ( ( 0 == lprcMonitor->left ) && ( 0 == lprcMonitor->top ) )
	{
		LPRECT pRect = LPRECT(dwData);

		pRect->left += ( lprcMonitor->right - pRect->right ) / 2;
		pRect->top += ( lprcMonitor->bottom - pRect->bottom ) / 2;

		return FALSE;
	}

	return TRUE;
}
