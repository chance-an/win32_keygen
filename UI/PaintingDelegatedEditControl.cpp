#include "stdafx.h"
#include "../stdafx.h"

#include "ui.h"
#include "PaintingDelegatedEditControl.h"

namespace UI {
	PaintingDelegatedEditControl::PaintingDelegatedEditControl(HWND parent, HWND hWnd) :
		DefaultPaintingDelegatedControl(parent, hWnd),
		showPrompt(false){
		_tcscpy_s(tip, _T(""));
	};

	PaintingDelegatedEditControl::~PaintingDelegatedEditControl() {}

	DefaultPaintingDelegatedControl* PaintingDelegatedEditControl::Create(HWND parent, HWND hWnd) {
		TCHAR name[100];
		GetClassName(hWnd, name, 100);
		bool isEdit = _tcscmp(_T("Edit"), name) == 0;

		if (!isEdit) {
			return nullptr;
		}

		return new PaintingDelegatedEditControl(parent, hWnd);
	};

	void PaintingDelegatedEditControl::setTip(const TCHAR* newText) {
		_tcscpy_s(this->tip, newText);
	}
	
	bool PaintingDelegatedEditControl::setShowPrompt(bool ifShowPrompt) {
		auto tmp = this->showPrompt;
		this->showPrompt = ifShowPrompt;
		return tmp;
	};


	LRESULT PaintingDelegatedEditControl::draw(UINT msg, WPARAM wParam, LPARAM lParam) {
		LRESULT result = DefSubclassProc(self, msg, wParam, lParam);

		PaintingDelegatedControl::resetRedirectDC();
		PaintingDelegatedControl::transferDCToBackupDC(GetDC(self));
		HDC redirectDC = getRedirectPaintingDC();
		this->drawCaret(redirectDC);
		if (showPrompt) {
			showTip(redirectDC);
		}

		PaintingDelegatedControl::sendPaintingToParentWindow();

		return 1;
	}

	LRESULT PaintingDelegatedEditControl::onChar(UINT msg, WPARAM wParam, LPARAM lParam){
		this->setShowPrompt(false);
		return 1;
	}

	bool PaintingDelegatedEditControl::drawCaret(HDC hdc) {
		//printf("DefaultPaintingDelegatedControl::EditDrawingStep\n");

		if (self != GetFocus()) {
			return true;
		}

		POINT pt;
		GetCaretPos(&pt);

		// TODO better approach for perceiving how much offset is needed.
		pt.x += 2;
		pt.y += 2;

		Gdiplus::Pen pen(Gdiplus::Color::Black, 1.0f);
		// TODO why 13?
		Gdiplus::Graphics(hdc).DrawLine(&pen, pt.x, pt.y, pt.x, pt.y + 13);
		return true;
	};

	void PaintingDelegatedEditControl::showTip(HDC hdc) {
		std::unique_ptr<Gdiplus::Font> font(nullptr);
		UI::GetCustomFont(font, UI::ScaleX(7));

		RECT drawingContour;

		GetClientRect(self, &drawingContour);

		SetBkMode(hdc, TRANSPARENT);

		TCHAR *text = this->tip;
		int cText = _tcslen(text);

		DPtoLP(hdc, (LPPOINT)&drawingContour, 2);

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::StringFormat format(
			Gdiplus::StringFormatFlags::StringFormatFlagsNoClip);
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		Gdiplus::RectF rect(
			drawingContour.left,
			drawingContour.top,
			drawingContour.right - drawingContour.left,
			drawingContour.bottom - drawingContour.top);

		Gdiplus::SolidBrush brush(Gdiplus::Color(122, 22, 22, 22));

		auto result = graphics.DrawString(text, cText, font.get(), rect, &format, &brush);

		printf("graphics.DrawString:%d\n", result);
	}


}