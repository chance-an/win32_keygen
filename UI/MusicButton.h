#pragma once
#include "../stdafx.h"
#include "CustomButton.h"

namespace UI {
	class MusicButton : public UI::CustomButton{
	public:
		MusicButton(HWND parent, int controlId, HWND paintingRedirectWindow) :
			CustomButton(parent, controlId, paintingRedirectWindow),
			music_state(1),
			pBitmap(nullptr),
			pGrayedBitmap(nullptr){

			initialize();
		}

		~MusicButton() {
			// Note that pBitmap will be recycled by the UI framework. Don't let unique_ptr delete the managed by itself.
			pBitmap.release();
		};
	
	private:
		int music_state;

		std::unique_ptr<Gdiplus::Bitmap> pBitmap;
		std::unique_ptr<Gdiplus::Bitmap> pGrayedBitmap;

		void initialize();

		LRESULT onPaint(WPARAM wParam, LPARAM lParam);
		LRESULT onClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	};
}