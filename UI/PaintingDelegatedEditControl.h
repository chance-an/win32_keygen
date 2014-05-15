#pragma once

#include "DefaultPaintingDelegatedControl.h"

namespace UI {
	class PaintingDelegatedEditControl : public DefaultPaintingDelegatedControl {
	public:
		~PaintingDelegatedEditControl();
		static DefaultPaintingDelegatedControl* Create(HWND parent, HWND hWnd);

		void setTip(const TCHAR*);
		bool setShowPrompt(bool);
	private:
		TCHAR tip[100];
		bool showPrompt;

		PaintingDelegatedEditControl(HWND parent, HWND hWnd);

		LRESULT draw(UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT onChar(UINT msg, WPARAM wParam, LPARAM lParam);
		bool drawCaret(HDC hdc);
		void showTip(HDC hdc);
	};
}