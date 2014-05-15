// main.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

#include "main.h"

#include "ui/ui.h"
#include "ui/MainWindow.h"
#include "ui/CustomButton.h"
#include "ui/GenerateButton.h"
#include "ui/DefaultPaintingDelegatedControl.h"
#include "dev/debug_console.h"


#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")

#pragma comment(lib, "ComCtl32.lib")


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE, int);
BOOL				Finalize(HINSTANCE);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MessageHandler(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<UI::MainWindow> mainWindow;

SIZE dialogSize;

HWND CTR_EDIT_NAME;
HWND CTR_EDIT_SN;

struct CleanUpItems {
	HDC redirectHDC;
	HDC background_backup_HDC;
	HBITMAP hBmp;
} cleanUpItems;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Open debug console.
	//dev::SetStdOutToNewConsole();
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MAIN, szWindowClass, MAX_LOADSTRING);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAIN));


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	Finalize(hInstance);
	return (int) msg.wParam;
}



//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   tagINITCOMMONCONTROLSEX icex;
   icex.dwICC = ICC_STANDARD_CLASSES;
   icex.dwSize = sizeof icex;

   if (!InitCommonControlsEx(&icex)){   
	   return FALSE;
   }

   UI::initialize();

   mainWindow = std::make_unique<UI::MainWindow>();

   return TRUE;
}

BOOL Finalize(HINSTANCE hInstance) {
	mainWindow.reset(nullptr);

	UI::cleanup();
	return TRUE;
}

