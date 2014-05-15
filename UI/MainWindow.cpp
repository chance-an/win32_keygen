#include "stdafx.h"
#include "../stdafx.h"
#include <boost/thread/once.hpp>

#define MAINWINDOW_CLASS_NAME _T("MainWindow")

#include "../main.h"
#include "MainWindow.h"
#include "ui.h"
#include "DefaultPaintingDelegatedControl.h"
#include "PaintingDelegatedEditControl.h"
#include "GenerateButton.h"
#include "MusicButton.h"

namespace UI {
	MainWindow::MainWindow() : 
		controlWindow(controlWindow),
		nameEdit(nullptr) {
		static boost::once_flag once = BOOST_ONCE_INIT;
		boost::call_once(&RegisterWindowClass, once);
		
		controlWindow = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL,
			ControlWindowHandler, (LPARAM)this);
		SetWindowLongPtr(controlWindow, GWLP_USERDATA, (LONG_PTR)this);
		SetWindowLongPtr(controlWindow, GWL_EXSTYLE, 
			GetWindowLongPtr(controlWindow, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_LAYERED);

		SetLayeredWindowAttributes(controlWindow, NULL, 1, LWA_ALPHA);

		presentationWindow = CreateWindowEx(
			WS_EX_TRANSPARENT | WS_EX_APPWINDOW | WS_EX_LAYERED,
			MAINWINDOW_CLASS_NAME,
			NULL,
			WS_VISIBLE | WS_POPUP,
			0, 0, 0, 0,
			controlWindow,
			NULL,
			hInst,
			NULL);

		SetWindowLongPtr(presentationWindow, GWLP_USERDATA, (LONG_PTR)this);
		//SetLayeredWindowAttributes(presentationWindow, NULL, 255, LWA_ALPHA);

		retrieveInitialMetrics();

		initializeControls();

		initializeSound();
		
		// Align two windows
		repositionWindow();
		
		// Render PNG on presentation Window
		redirectLayeredWindowDrawing();

		ShowWindow(controlWindow, SW_SHOW);
		ShowWindow(presentationWindow, SW_SHOW);
		UpdateWindow(presentationWindow);

		//SetFocus(mainWindow);
	}

	MainWindow::~MainWindow(){
		if (this->mainWindowPaintingRedirectDC) {
			DeleteDC(this->mainWindowPaintingRedirectDC);
		}
		if (this->backupRedirectDC) {
			DeleteDC(this->backupRedirectDC);
		}
	};

	HWND MainWindow::getControl(INT controlID) {
		return GetDlgItem(controlWindow, controlID);
	};

	void MainWindow::RegisterWindowClass() {
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = PresentationWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		//wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = MAINWINDOW_CLASS_NAME;
		wc.hIconSm = NULL;

		ATOM result = RegisterClassEx(&wc);
	}

	void MainWindow::initializeDialogPosition(HWND hWnd){
		RECT rc, rcDlg, rcOwner;

		HWND hwndOwner = GetDesktopWindow();

		GetWindowRect(hwndOwner, &rcOwner);
		GetWindowRect(hWnd, &rcDlg);
		CopyRect(&rc, &rcOwner);

		// Offset the owner and dialog box rectangles so that right and bottom 
		// values represent the width and height, and then offset the owner again 
		// to discard space taken up by the dialog box. 

		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		OffsetRect(&rc, -rc.left, -rc.top);
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

		// The new position is the sum of half the remaining space and the owner's 
		// original position. 

		SetWindowPos(hWnd,
			HWND_TOP,
			rcOwner.left + (rc.right / 2),
			rcOwner.top + (rc.bottom / 2),
			0, 0,          // Ignores size arguments. 
			SWP_NOSIZE);
	}

	void MainWindow::initializeControls() {
		// Combine two windows
		[this](){
			if (!controlWindow) {
				return;
			}

			// Survey controls
			EnumChildWindows(controlWindow, (WNDENUMPROC)[](HWND hWnd, LPARAM lParam) -> BOOL {
				TCHAR name[100];
				GetClassName(hWnd, name, 100);
				bool isEdit = _tcscmp(_T("Edit"), name) == 0;
				if (isEdit) {
					MainWindow* mainWindow = ((MainWindow*)lParam);
					HWND parent = mainWindow->presentationWindow;
					// For recycle later
					mainWindow->paintingDelegatedControls.push_back(						
						std::unique_ptr<DefaultPaintingDelegatedControl>(
							PaintingDelegatedEditControl::Create(parent, hWnd))
					);
				}
				return TRUE;
			}, (LPARAM)this);

			// Instantiate "OK" button
			okButton = std::make_unique<UI::CustomButton>(controlWindow, IDOK, presentationWindow);
			generateButton = std::make_unique<GenerateButton>(controlWindow, IDGENERATE, presentationWindow);
			musicButton = std::make_unique<UI::MusicButton>(controlWindow, IDC_MUSIC, presentationWindow);

			// Keep reference of nameEdit
			for (std::unique_ptr<PaintingDelegatedControl>& pControl : paintingDelegatedControls) {
				if (pControl.get() == nullptr) {
					continue;
				}

				if (pControl.get()->GetHWND() != GetDlgItem(controlWindow, IDC_EDIT_NAME)) {
					continue;
				}
				nameEdit = pControl.get(); break;
			}
			assert(nameEdit);
			PaintingDelegatedEditControl* edit = static_cast<PaintingDelegatedEditControl*>(nameEdit);
			edit->setTip(INVALID);
			edit->setShowPrompt(true);
		}();

		// Set window region
		[this](){
			if (!controlWindow) {
				return;
			}
			// make the control window completely invisible.
			//SetLayeredWindowAttributes(controlWindow, NULL, 0, LWA_ALPHA);

			int radius = 50;
			HRGN hitTestArea = CreateRoundRectRgn(0, 0, initialSize.cx, initialSize.cy, radius, radius);
			SetWindowRgn(controlWindow, hitTestArea, TRUE);
		}();
	};
	
	void MainWindow::initializeSound() {
		_XMSoundModule = std::make_unique<UI::XMSoundModule>(controlWindow);
		_XMSoundModule->initialize();
		_XMSoundModule->play();
	};

	void MainWindow::repositionWindow() {
		printf("MainWindow::repositionWindow\n");
		if (!controlWindow) {
			return;
		}

		RECT location;
		GetWindowRect(controlWindow, &location);

		SetWindowPos(presentationWindow,
			HWND_TOP,
			location.left,
			location.top,
			location.right - location.left, 
			location.bottom - location.top,
			SWP_SHOWWINDOW);
		
		SetFocus(presentationWindow);
	};

	void MainWindow::retrieveInitialMetrics() {
		RECT rect;
		GetWindowRect(controlWindow, &rect);
		this->initialSize = UI::RECT2SIZE(rect);
	}
	
	void MainWindow::redirectLayeredWindowDrawing() {
		std::unique_ptr<Gdiplus::Bitmap> pBitmap(nullptr);

		UI::GetPNGBitMap(MAKEINTRESOURCE(IDB_PNG1), pBitmap);

		UI::CreateRedirectDC(
			this->presentationWindow, 
			this->initialSize, 
			&this->mainWindowPaintingRedirectDC);

		RECT rect;
		GetClientRect(this->presentationWindow, &rect);
		SIZE dialogSize = UI::RECT2SIZE(rect);

		Gdiplus::Graphics graphics(this->mainWindowPaintingRedirectDC);
		// Transfer Bitmap to redirectedDC.
		graphics.DrawImage(pBitmap.get(), 0, 0, dialogSize.cx, dialogSize.cy);

		POINT ptSrc = { 0, 0 };
		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		HDC destDC = GetDC(this->presentationWindow);

		bool result = UpdateLayeredWindow(this->presentationWindow, destDC, NULL, &dialogSize,
			this->mainWindowPaintingRedirectDC, &ptSrc, NULL, &blend, ULW_ALPHA);

		ReleaseDC(this->presentationWindow, destDC);

		// Clone the DC of the initial state to backup DC.
		CloneHDC(this->mainWindowPaintingRedirectDC, &this->backupRedirectDC);
	};

	void MainWindow::renderChildControlToPresentationWindow(HWND child, 
		const PaintingDelegatedControl::PaintStruct* paint_struct) {
		HDC& hdc_child = paint_struct->hdc;

		RECT childLocation;
		GetWindowRect(child, &childLocation);
		MapWindowPoints(HWND_DESKTOP, this->presentationWindow, (LPPOINT)&childLocation, 2);
		SIZE childSize = UI::RECT2SIZE(childLocation);

		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		// Refill the background of the child window(control) with backed up background.
		BitBlt(this->mainWindowPaintingRedirectDC, 
			childLocation.left, childLocation.top, childSize.cx, childSize.cy,
			this->backupRedirectDC, childLocation.left, childLocation.top, SRCCOPY);

		// Paint child control on the redirected DC
		// Currently if the control renders a DC that contains alpha channel, it has to be rendered differently
		if (paint_struct->isTransparent) {
			auto r = AlphaBlend(this->mainWindowPaintingRedirectDC,
				childLocation.left, childLocation.top, childSize.cx, childSize.cy,
				hdc_child, 0, 0, childSize.cx, childSize.cy, blend);

			//printf("AlphaBlend() result: %d, error: %d\n", r, GetLastError());
		} else {
			HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hdc_child, OBJ_BITMAP);
			Gdiplus::Bitmap bitmap(hBitmap, NULL);

			Gdiplus::Graphics graphics(this->mainWindowPaintingRedirectDC);
			graphics.DrawImage(&bitmap, childLocation.left, childLocation.top);
		}
		
		/*
		BitBlt(this->mainWindowPaintingRedirectDC,
			childLocation.left, childLocation.top, childSize.cx, childSize.cy,
			hdc_child, 0, 0, SRCCOPY);*/

		// Update Layered window
		HDC destDC = GetDC(this->presentationWindow);
		POINT ptSrc = { 0, 0 };
		bool result = UpdateLayeredWindow(this->presentationWindow, destDC, NULL, &initialSize,
			this->mainWindowPaintingRedirectDC, &ptSrc, NULL, &blend, ULW_ALPHA);

		ReleaseDC(this->presentationWindow, destDC);
	};

	void MainWindow::onMoveWindow(LPARAM lParam){
		int xPos = LOWORD(lParam);  // horizontal position of cursor 
		int yPos = HIWORD(lParam);  // vertical position of cursor 

		// Drag window by clicking anywhere on 
		PostMessage(this->controlWindow, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(xPos, yPos));
	};

	LRESULT MainWindow::PresentationWndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam){
		//printf("MainWindow::WndProc message rcvd: %d\n", uMsg);

		MainWindow * windowInstance = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		int xPos, yPos;
		RECT rect;
		SIZE size;

		switch (uMsg)
		{
		case WM_ACTIVATE:
			break;
		case WM_CREATE:
			//printf("PresentationWnd: WM_CREATE\n");
			//windowInstance->redirectLayeredWindowDrawing();
			break;
		// a custom message
		case WM_CHILDWINDOW_PAINT:
			windowInstance->renderChildControlToPresentationWindow((HWND)wParam, 
				(PaintingDelegatedControl::PaintStruct*)lParam);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	INT_PTR CALLBACK MainWindow::ControlWindowHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
		HWND hwndOwner;
		RECT rc, rcDlg, rcOwner;
		PAINTSTRUCT ps;
		HDC hdc;
		int previous;
		RECT rect;
		SIZE size;
		int xPos, yPos;

		MainWindow * windowInstance = (MainWindow*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		//printf("control window msg received: %d, %4x\n", message, message);

		switch (message)
		{
		case WM_INITDIALOG:
			((MainWindow *)lParam)->initializeDialogPosition(hDlg);

			if (GetDlgCtrlID((HWND)wParam) != IDD_MAIN_DIALOG)
			{
				SetFocus(GetDlgItem(hDlg, IDD_MAIN_DIALOG));
				return FALSE;
			}
			return TRUE;
		case WM_MOVE:
			// Also update presentation window location
			GetWindowRect(windowInstance->controlWindow, &rect);
			size = UI::RECT2SIZE(rect);
			MoveWindow(windowInstance->presentationWindow, rect.left, rect.top, size.cx, size.cy, true);
			break;
		case WM_LBUTTONDOWN:
		{
			windowInstance->onMoveWindow(lParam);
			break;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				// EndDialog(hDlg, LOWORD(wParam));
				DestroyWindow(hDlg);
				return (INT_PTR)TRUE;
			}
			break;
		case WM_DRAWITEM:
			std::cout << " -- MAIN: WM_DRAWITEM" << std::endl;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

		return (INT_PTR)FALSE;
	};

}