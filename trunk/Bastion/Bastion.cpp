// Bastion.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Bastion.h"

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
HWND				InitInstance(WindowData& _rConfig);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
HWND				MyCreateWindow(WindowData& _rConfig);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WindowData m_oWindow;
	Application* pApp = new Application;

	// default values
	m_oWindow.m_pCreateWindow = MyCreateWindow;
	m_oWindow.m_hInstance = hInstance;
	m_oWindow.m_hPrevInstance = hPrevInstance;
	m_oWindow.m_lpCmdLine = lpCmdLine;
	m_oWindow.m_nCmdShow = nCmdShow;
	m_oWindow.m_oClientRect.left = 0;
	m_oWindow.m_oClientRect.right = 0;
	m_oWindow.m_oClientRect.right = 640;
	m_oWindow.m_oClientRect.bottom = 480;
	m_oWindow.m_uDXColorFormat = D3DFMT_A8R8G8B8;
	m_oWindow.m_uDXDepthFormat = D3DFMT_D24S8;
	m_oWindow.m_uDXGBufferFormat = D3DFMT_A8R8G8B8;
	m_oWindow.m_uDXGBufferCount = 1;
	m_oWindow.m_fZNear = 1.0f;
	m_oWindow.m_fZFar = 1000.0f;
	m_oWindow.m_bFullScreen = false;

	if (false != pApp->Create(boost::any(&m_oWindow)))
	{
		do
		{
			pApp->Update();
		}
		while (Application::EStateMode_QUIT != pApp->GetStateMode());
	}

	pApp->Release();
	delete pApp;


	return 0;
}

HWND MyCreateWindow(WindowData& _rConfig)
{
	// Initialize global strings
	LoadString(_rConfig.m_hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(_rConfig.m_hInstance, IDC_BASTION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(_rConfig.m_hInstance);

	// Perform application initialization:
	InitInstance (_rConfig);

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
//   FUNCTION: InitInstance(WindowData)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(WindowData& _rConfig)
{
   hInst = _rConfig.m_hInstance; // Store instance handle in our global variable

   WINDOWINFO oDesktopWI;
   GetWindowInfo( GetDesktopWindow(), &oDesktopWI );

   _rConfig.m_oWindowRect = _rConfig.m_oClientRect;

   // Resizes window so that client size has the specified video width/height.
   AdjustWindowRect( &_rConfig.m_oWindowRect, WS_OVERLAPPEDWINDOW, TRUE );
   // Set window positions in the middle of the first monitor.
   EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, LPARAM(&_rConfig.m_oWindowRect) );

   _rConfig.m_hWnd = CreateWindow(szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW,
	   _rConfig.m_oWindowRect.left,
	   _rConfig.m_oWindowRect.top,
	   _rConfig.m_oWindowRect.right,
	   _rConfig.m_oWindowRect.bottom,
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

		pRect->left = ( lprcMonitor->right - pRect->right ) / 2;
		pRect->top = ( lprcMonitor->bottom - pRect->bottom ) / 2;

		return FALSE;
	}

	return TRUE;
}
