#include "stdafx.h"
#include "../stdafx.h"

#include "../Resource.h"
#include "MusicButton.h"
#include "UI.h"
#include "MainWindow.h"
extern std::unique_ptr<UI::MainWindow> mainWindow;

namespace UI {

	void MusicButton::initialize() {
		// Load symbol
		UI::GetPNGBitMap(MAKEINTRESOURCE(IDB_PNG2), pBitmap);

		Gdiplus::Bitmap * cloneBitmap = static_cast<Gdiplus::Bitmap *>(static_cast<Gdiplus::Image*>(pBitmap.get())->Clone());
		pGrayedBitmap.reset(cloneBitmap);

		UI::convertToGrayedImage(pGrayedBitmap.get());
	}

	LRESULT MusicButton::onPaint(WPARAM wParam, LPARAM lParam) {
		PAINTSTRUCT ps;
		HWND hWnd = this->self;

		BeginPaint(hWnd, &ps);

		RECT drawingContour = getDrawingContour();
		SIZE size = UI::RECT2SIZE(drawingContour);

		resetRedirectDC();
		HDC hdc = getRedirectPaintingDC();

		// Draw symbol
		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Bitmap* bitmapToUse = music_state == 1 ? pBitmap.get() : pGrayedBitmap.get();
		graphics.DrawImage(bitmapToUse, drawingContour.left, drawingContour.top, size.cx, size.cy);

		sendPaintingToParentWindow();

		EndPaint(hWnd, &ps);

		return 1;
	}

	LRESULT MusicButton::onClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		std::function<void()> actions[] = {
			[](){
				mainWindow->GetMusicModule()->resume();
			},
			[](){
				mainWindow->GetMusicModule()->pause();
			}
		};
		actions[music_state]();

		music_state = 1 - music_state;
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	};

}

