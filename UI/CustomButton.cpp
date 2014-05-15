#include "stdafx.h"
#include "../stdafx.h"

#include "CustomButton.h"
#include "ui.h"
#define LOGICAL_EXTEND 100

void drawTextOnWindow(HWND hwnd, HDC hdc);

void drawTextOnWindow(HWND hWnd, HDC hdc, RECT drawingContour) {
	std::unique_ptr<Gdiplus::Font> font(nullptr);
	UI::GetCustomFont(font, 6);

	printf("font: result: %d\n", font->GetLastStatus());
	assert(font->GetLastStatus() == 0);

	SetBkMode(hdc, TRANSPARENT);

	TCHAR text[100];
	int cText = Button_GetText(hWnd, text, 100);

	DPtoLP(hdc, (LPPOINT)&drawingContour, 2);

	Gdiplus::Graphics graphics(hdc);
	Gdiplus::StringFormat format(
		Gdiplus::StringFormatFlags::StringFormatFlagsNoClip);
	format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
	format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
	Gdiplus::RectF rect(
		drawingContour.left, 
		drawingContour.top - 1, 
		drawingContour.right - drawingContour.left, 
		drawingContour.bottom - drawingContour.top - 1);

	Gdiplus::SolidBrush brush(Gdiplus::Color(200, 255, 255, 255));

	graphics.DrawString(text, cText, font.get(), rect, &format, &brush);
};

bool ClipRegion(HWND hWnd, HDC hdc, const RECT& rc){
	HRGN clipRegion = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 10, 10);
	SelectClipRgn(hdc, clipRegion);
	DeleteObject(clipRegion);
	return true;
}

namespace UI{
	CustomButton::CustomButton(HWND parent, int controlId, HWND paintingRedirectWindow) :
		PaintingDelegatedControl(parent, GetDlgItem(parent, controlId), paintingRedirectWindow),
		controlId(controlId),
		_buttonState((BFS_GRADE0 << 16) | BS_NORMAL),
		_animationTimer(NULL),
		onMouseDown([=](){}),
		onMouseUp([=](){})
	{
		backupParentBackground = nullptr;
		// printf("Before Button_SetStyle: %x\n", self);
		// Button_SetStyle(self, BS_PUSHBUTTON, FALSE);
		// printf("Button_SetStyle: %d\n", GetLastError());

		SetWindowSubclass(self, SUBCLASSPROC, controlId, (DWORD_PTR)this);
		SetWindowLongPtr(self, GWLP_USERDATA, (LONG_PTR)this);
	}

	CustomButton::~CustomButton()
	{
		::DeleteDC(this->backupParentBackground);
		::DeleteObject(backupParentBackgroundBitMap);
	}

	bool CustomButton::isOpaque() {
		return false;
	}

	RECT CustomButton::getDrawingContour() {
		RECT rect;

		GetClientRect(self, &rect);
		rect.left++;
		rect.top++;
		rect.bottom--;
		rect.right--;

		printf("BUTTON_STATE: %d\n", getButtonState() & 0xFFFF);

		switch (getButtonState() & 0xFFFF) {
		case BS_NORMAL:
			break;
		case BS_HOVER:
			// move down a bit
			rect.top++;
			rect.bottom++;
			break;
		case BS_CLICKED:
			// move down 2 bits
			rect.top+=2;
			rect.bottom+=2;
			break;
		}

		return rect;
	}

	// This is a default implementation
	LRESULT CustomButton::onClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
		printf("Default CustomButton::onClick invoked.\n");
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	};

	LRESULT CALLBACK CustomButton::SUBCLASSPROC(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
		){

		// printf("MSG RCVD: uIdSubclass: %d, 0x%04x\n", uIdSubclass, uMsg);

		CustomButton* instance = (CustomButton*)dwRefData;
		LRESULT result;
		static bool mousemoveFlag = false;
		TRACKMOUSEEVENT tme;

		switch (uMsg)
		{
		case WM_CREATE:
			std::cout << "WTH: Create" << std::endl;
			break;
		case WM_PAINT:
			std::cout << "WTH: WM_PAINT" << std::endl;
			return instance->onPaint(wParam, lParam);
			break;
		case WM_DRAWITEM:
			std::cout << "WTH: WM_DRAWITEM" << std::endl;
			break;
		case  WM_LBUTTONDOWN:
			instance->onMouseDown();
			PostMessage(hWnd, WM_PAINT, NULL, NULL);
			break;
		case WM_LBUTTONUP:
			std::cout << "WTH: Clicked" << std::endl;		
			instance->onMouseUp();
			PostMessage(hWnd, WM_PAINT, NULL, NULL);
			return instance->onClick(hWnd, uMsg, wParam, lParam);
			// break;
		case WM_MOUSEMOVE:
			if (!mousemoveFlag) {
				mousemoveFlag = true;
				PostMessage(hWnd, WM_MOUSEENTER, NULL, NULL);
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.dwHoverTime = NULL;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
				break;
			}
			break;
		case WM_MOUSEENTER:
			instance->onMouseEnter(hWnd, uMsg, wParam, lParam);
			break;
		case WM_MOUSELEAVE:
			mousemoveFlag = false;
			instance->onMouseLeave(hWnd, uMsg, wParam, lParam);
			break;
		default:
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	};

	void CALLBACK CustomButton::buttonAnimationFadeIn(
		HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
		CustomButton* instance = (CustomButton*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		INT32 bfs = (instance->_buttonState & 0xFFFF0000) >> 16;
		INT32 bs = (instance->_buttonState & 0xFFFF);

		if (bfs == BFS_GRADE3) {
			KillTimer(hwnd, idEvent);
			return;
		}
		instance->_buttonState = (++bfs << 16) | bs;
		PostMessage(hwnd, WM_PAINT, NULL, NULL);

		//printf("BFS: %d\n", bfs);
	}

	void CALLBACK CustomButton::buttonAnimationFadeOut(
		HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
		CustomButton* instance = (CustomButton*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		INT32 bfs = (instance->_buttonState & 0xFFFF0000) >> 16;
		INT32 bs = (instance->_buttonState & 0xFFFF);

		if (bfs == BFS_GRADE0) {			
			KillTimer(hwnd, idEvent);
			return;
		}
		instance->_buttonState = (--bfs << 16) | bs;
		PostMessage(hwnd, WM_PAINT, NULL, NULL);

		//printf("BFS: %d\n", bfs);
	}
	
	LRESULT CustomButton::onPaint(WPARAM wParam, LPARAM lParam){
		PAINTSTRUCT ps;
		HDC hdc;
		HWND hWnd = this->self;

		RECT rc;

		//hdc = BeginPaint(hWnd, &ps);
		BeginPaint(hWnd, &ps);

		resetRedirectDC();
		hdc = getRedirectPaintingDC();

		GetClientRect(hWnd, &rc);
		RECT drawingContour = getDrawingContour();

		// This is for coordinate transformation
		// SetMapMode(hdc, MM_ANISOTROPIC);
		// SetWindowExtEx(hdc, 100, 100, NULL);
		// SetViewportExtEx(hdc, rc.right, rc.bottom, NULL);

		// Draw background
		ClipRegion(hWnd, hdc, drawingContour);
		// Calculate transparency according to current flashing state.
		float bfs = (getButtonState() & 0xFFFF0000) >> 16;
		float grade = bfs / BFS_GRADE3;
		UI::RenderGradient(hWnd, hdc, {
			{ RGBA(30, 30, 210, (int)(180 + (255 - 180) * grade)), 0 },
			{ RGBA(123, 123, 150, 255), 78 }
		});

		HRGN fullRegion = ::CreateRectRgn(0, 0, rc.right, rc.bottom);
		SelectClipRgn(hdc, fullRegion);
		
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		// Draw bounding box
		Gdiplus::GraphicsPath path;
		UI::MakeRoundRect(Gdiplus::Point(drawingContour.left, drawingContour.top), 
			Gdiplus::Point(drawingContour.right - 1, drawingContour.bottom - 2), 10, &path);
		Gdiplus::Pen pen(Gdiplus::Color::Black, 1.5);
		Gdiplus::Graphics(hdc).DrawPath(&pen, &path);		

		// Draw Text.	
		drawTextOnWindow(hWnd, hdc, drawingContour);

		// printf("child control hdc %x\n", hdc);
		sendPaintingToParentWindow();

		EndPaint(hWnd, &ps);		
		return 1L;
	};

	LRESULT CustomButton::onMouseEnter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//std::cout << "WTH: Mouse Enter" << std::endl;
		this->_buttonState = (this->_buttonState & 0xFFFF0000) | BS_HOVER;
		PostMessage(hWnd, WM_PAINT, NULL, NULL);

		this->onMouseDown = [=](){
			this->_buttonState = (this->_buttonState & 0xFFFF0000) | BS_CLICKED;
		};
		this->onMouseUp = [=](){
			this->_buttonState = (this->_buttonState & 0xFFFF0000) | BS_HOVER;
		};

		// fade in 
		_animationTimer = SetTimer(hWnd, CUSTOM_BUTTON_ANIMATION_TIMER_ID, getAnimationFrameRate(), 
			&CustomButton::buttonAnimationFadeIn);
		return 1;
	};

	LRESULT CustomButton::onMouseLeave(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//std::cout << "WTH: Mouse Leave" << std::endl;
		this->_buttonState = (this->_buttonState & 0xFFFF0000) | BS_NORMAL;
		PostMessage(hWnd, WM_PAINT, NULL, NULL);

		this->onMouseDown = [](){};
		this->onMouseUp = [](){};

		// fade out 
		_animationTimer = SetTimer(hWnd, CUSTOM_BUTTON_ANIMATION_TIMER_ID, getAnimationFrameRate(),
			&CustomButton::buttonAnimationFadeOut);
		return 1;
	};

}