#pragma once

#define RGBA(r,g,b,a) (RGB(r,g,b) | (a << 24)) 
#define GetAValue(rgb) (LOBYTE((rgb)>>24))
#define RGB2LUMA(r,g,b) ((INT32)(0.2126* r + 0.7152* g + 0.0722*b) & 0xFF)

#define WM_CHILDWINDOW_PAINT (WM_USER + 0x0001)
#define WM_MOUSEENTER (WM_USER + 0x0002)

struct COLOR16_{
	COLOR16_(BYTE v) : v((unsigned short)(v << 8)){};
	operator unsigned short() const
	{
		return v;
	}
private:
	unsigned short v;
};

namespace UI{
	static int GradientRenderCount = 0;
	extern bool _intialized;
	extern int _dpiX, _dpiY;

	struct GrandientInfoEntry{
		COLORREF color;
		float position;
	};

	void initialize();
	void cleanup();

	SIZE RECT2SIZE(const RECT& rect);

	void GetCustomFont(std::unique_ptr<Gdiplus::Font>& font, float size);

	void GetLocalCoordinates(HWND hWnd, RECT* Rect);	

	void CloneHDC(const HDC & from, HDC * to);
	SIZE GetHDCSize(const HDC& hdc);

	float GetUIScale();

	inline float  ScaleX(float x) { assert(_intialized); return x * _dpiX / 96; }
	inline float  ScaleY(float y) { assert(_intialized); return y * _dpiX / 96; }


	/* 
	Example:
	UI::RenderGradient(hWnd, hdc, {
		// {Color, Position_as_percent}
		{ RGBA(100, 20, 30, 33), 0},
		{ RGBA(40, 123, 111, 255), 78 },
		{ RGBA(40, 20, 244, 255), 100 }
	});
	*/
	void RenderGradient(HWND hWnd, HDC hdc, const std::vector<GrandientInfoEntry>& colors);

	bool GetPNGBitMap(LPCTSTR lpName, std::unique_ptr<Gdiplus::Bitmap> & out_pBitmap);

	void convertToGrayedImage(Gdiplus::Bitmap * bitmap);

	// size is required in case the window is shrunk due to PNG rendering.
	bool CreateRedirectDC(HWND hWnd, const SIZE& size, HDC * out_hdc);

	void MakeRoundRect(Gdiplus::Point topLeft, Gdiplus::Point bottomRight, INT percentageRounded,
		Gdiplus::GraphicsPath* path) ;
}