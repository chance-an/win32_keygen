#include "stdafx.h"
#include "../stdafx.h"

#include "PaintingDelegatedControl.h"
#include "ui.h"
#include "MainWindow.h"

extern std::unique_ptr<UI::MainWindow> mainWindow;

namespace UI{
	PaintingDelegatedControl::PaintingDelegatedControl(HWND parent, HWND self) :
		PaintingDelegatedControl(parent, self, nullptr) {}

	PaintingDelegatedControl::PaintingDelegatedControl(HWND parent, HWND self, HWND paintingRedirectWindow) :
		backup_drawing_section(nullptr),
		_delegated_dc(nullptr),
		replaced_bitmap(nullptr),
		_initialized(false),
		parent(parent),
		self(self),
		paintingRedirectWindow(paintingRedirectWindow)
	{		
		initialize();
	}

	PaintingDelegatedControl& PaintingDelegatedControl::operator= (
		const PaintingDelegatedControl& rhs) {
		this->_delegated_dc = rhs._delegated_dc;
		this->backup_drawing_section = rhs.backup_drawing_section;
		return *this;
	}

	bool PaintingDelegatedControl::isOpaque() {
		return true;
	}

	void PaintingDelegatedControl::initialize() {
		RECT rect;
		GetWindowRect(self, &rect);

		int width = rect.right - rect.left, height = rect.bottom - rect.top;

		SIZE size = { width, height};
		CreateRedirectDC(self, size, &_delegated_dc);

		backup_drawing_section = (HBITMAP)GetCurrentObject(_delegated_dc, OBJ_BITMAP);

		_initialized = true;
	}

	void PaintingDelegatedControl::resetRedirectDC(){
		assert(_initialized);
		BITMAP bitmap;
		if (!GetObject(backup_drawing_section, sizeof(BITMAP), &bitmap)){
			printf("backup_drawing_section has no info.\n");
			return;
		};

		int size_of_bytes = bitmap.bmWidthBytes * bitmap.bmHeight;
		BYTE * bits_addr = (BYTE*)bitmap.bmBits;
		memset(bits_addr, 0, size_of_bytes);
	}

	void PaintingDelegatedControl::sendPaintingToParentWindow(){
		assert(_initialized);
		// Fall back to the presentationWindow of global variable "mainWindow" if necessary.
		HWND presentationWindow = this->paintingRedirectWindow ? this->paintingRedirectWindow :
			mainWindow->presentationWindow;
		PaintStruct paintStruct = { _delegated_dc, !isOpaque()};
		SendMessage(presentationWindow, WM_CHILDWINDOW_PAINT, (WPARAM)self, (LPARAM)&paintStruct);
	}

	bool PaintingDelegatedControl::transferDCToBackupDC(const HDC& hDC) {
		assert(_initialized);
		// SetLastError(0);
		auto result = ::SendMessage(self, WM_PRINT, (WPARAM)_delegated_dc,
			(LPARAM)PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN | PRF_CHECKVISIBLE | PRF_ERASEBKGND | PRF_OWNED);

		// printf("PrintWindow(self, hdcMemory, 0): %d, %d\n", result, GetLastError());
		return true;
	};


}