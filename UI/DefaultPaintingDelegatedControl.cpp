#include "stdafx.h"
#include "../stdafx.h"

#include "PaintingDelegatedControl.h"
#include "DefaultPaintingDelegatedControl.h"
#include "ui.h"

namespace UI{
	
	DefaultPaintingDelegatedControl::DefaultPaintingDelegatedControl(HWND parent, int controlId) :
		DefaultPaintingDelegatedControl(parent, GetDlgItem(parent, controlId)){
	}

	DefaultPaintingDelegatedControl::DefaultPaintingDelegatedControl(HWND parent, HWND self) :
		 PaintingDelegatedControl(parent, self),
		controlId(controlId){
		initialize();
	}

	void DefaultPaintingDelegatedControl::initialize(){
		SetWindowSubclass(self, SUBCLASSPROC, controlId, (DWORD_PTR)this);
	}

	LRESULT CALLBACK DefaultPaintingDelegatedControl::SUBCLASSPROC(
		HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {

		DefaultPaintingDelegatedControl* instance = (DefaultPaintingDelegatedControl*)dwRefData;

		DRAWITEMSTRUCT* drawitem;
		LRESULT result;

		//printf("DPDC RCVD: uIdSubclass: %d, 0x%04x\n", uIdSubclass, uMsg);
		//printf("DefSubclassProc LastError: %d\n", GetLastError());

		switch (uMsg)
		{
		case WM_CREATE:
			std::cout << "DPDC: Create" << std::endl;
			break;
		case WM_CHAR:
			instance->onChar(uMsg, wParam, lParam);
		case WM_LBUTTONUP:
		case WM_CAPTURECHANGED:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_MOUSEFIRST:
		case WM_IME_NOTIFY:
		case WM_PAINT:
			//std::cout << "DPDC: WM_PAINT" << std::endl;
			return instance->draw(uMsg, wParam, lParam);		
		default:
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		return result;
	};

	LRESULT DefaultPaintingDelegatedControl::draw(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		LRESULT result = DefSubclassProc(self, uMsg, wParam, lParam);

		PaintingDelegatedControl::resetRedirectDC();
		PaintingDelegatedControl::transferDCToBackupDC(GetDC(self));
		PaintingDelegatedControl::sendPaintingToParentWindow();

		return 1;
	};

	LRESULT DefaultPaintingDelegatedControl::onChar(UINT msg, WPARAM wParam, LPARAM lParam){
		return 1;
	}
}