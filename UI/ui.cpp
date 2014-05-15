#include "stdafx.h"
#include "../stdafx.h"
#include "../Resource.h"

#include "ui.h"

extern HINSTANCE hInst;


void setTrivertex(TRIVERTEX* vertex, COLORREF color, LONG x, LONG y){
	vertex->x = x;
	vertex->y = y;
	vertex->Red = (COLOR16_)GetRValue(color);
	vertex->Green = (COLOR16_)GetGValue(color);
	vertex->Blue = (COLOR16_)GetBValue(color);
	vertex->Alpha = (COLOR16_)GetAValue(color);
};


namespace UI{
	ULONG_PTR m_gdiplusToken;
	bool _intialized;
	int _dpiX, _dpiY;

	// A PrivateFontCollection must stay live before all associating FontFamily is destroyed.
	// Also these two things guys need to be initialized after Gdi+ startup.
	std::unique_ptr<Gdiplus::PrivateFontCollection> m_fontcollection;
	std::unique_ptr<Gdiplus::FontFamily> customFontFamily;

	std::vector<HGLOBAL> memoryObjects;
	std::vector<Gdiplus::Bitmap*> bitmaps;


	void initializeFont();

	// The function owns pBitmap.
	bool _getPNGBitMap(LPCTSTR lpName, Gdiplus::Bitmap* & pBitmap);

	void initialize(){
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

		initializeFont();
		GetUIScale();

		_intialized = true;
	}

	void initializeFont(){
		HRSRC fontResource = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT1), RT_FONT);
		HGLOBAL mem = LoadResource(NULL, fontResource);
		void *data = LockResource(mem);
		size_t len = SizeofResource(NULL, fontResource);

		DWORD nFonts = 0;

		m_fontcollection = std::make_unique<Gdiplus::PrivateFontCollection>();
		customFontFamily = std::make_unique<Gdiplus::FontFamily>();

		Gdiplus::Status status = m_fontcollection->AddMemoryFont(data, len);
		assert(status == 0);

		int nNumFound = 0;
		m_fontcollection->GetFamilies(1, customFontFamily.get(), &nNumFound);
		printf("GetFamilies num of fonts found: %d\n", nNumFound);
	}

	float GetUIScale() {
		HDC hdc = GetDC(NULL);
		_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		return 0;
	}

	void cleanup(){
		// Free memory objects
		for (HGLOBAL& memoryObject : memoryObjects) {
			::GlobalUnlock(memoryObject);
			::GlobalFree(memoryObject);
		}

		for (Gdiplus::Bitmap* bitmap : bitmaps) {
			if (bitmap) {
				delete bitmap;
			}
		}

		customFontFamily.reset();
		m_fontcollection.reset();
		// Gdi+ related
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
	};

	SIZE RECT2SIZE(const RECT& rect){
		return{ rect.right - rect.left, rect.bottom - rect.top };
	}

	void GetCustomFont(std::unique_ptr<Gdiplus::Font>& font, float size){
		//Gdiplus::FontFamily fontFamily(L"superstition", NULL);		
		font = std::make_unique<Gdiplus::Font>(
			customFontFamily.get(),
			(Gdiplus::REAL)size,
			Gdiplus::FontStyle::FontStyleRegular,
			Gdiplus::Unit::UnitPixel);
	};

	bool GetPNGBitMap(LPCTSTR lpName, std::unique_ptr<Gdiplus::Bitmap> & pBitmap){

		HRSRC hResource = ::FindResource(hInst, lpName, _T("PNG"));
		if (!hResource)
			return false;

		DWORD imageSize = ::SizeofResource(hInst, hResource);
		if (!imageSize)
			return false;

		const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
		if (!pResourceData)
			return false;

		HGLOBAL m_hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (!m_hBuffer) {
			return false;
		}
		memoryObjects.push_back(m_hBuffer); // to be released later

		void* pBuffer = ::GlobalLock(m_hBuffer);
		if (!pBuffer) {
			return false;
		}

		CopyMemory(pBuffer, pResourceData, imageSize);

		IStream* pStream = NULL;
		Gdiplus::Bitmap* m_pBitmap;

		if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) != S_OK) {
			return false;
		}

		m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
		pStream->Release();

		if (!m_pBitmap) {
			return false;
		}

		if (m_pBitmap->GetLastStatus() != Gdiplus::Ok) {
			bitmaps.push_back(m_pBitmap); // to be released later
			return false;
		}

		pBitmap.reset(m_pBitmap);
		return true;
	};

	void convertToGrayedImage(Gdiplus::Bitmap * bitmap) {
		Gdiplus::RectF rectf;
		Gdiplus::Unit unit;
		Gdiplus::BitmapData bitmapData;

		bitmap->GetBounds(&rectf, &unit);
		Gdiplus::Rect rect(rectf.X, rectf.Y, rectf.Width, rectf.Height);

		auto status = bitmap->LockBits(
			&rect,
			Gdiplus::ImageLockMode::ImageLockModeRead | Gdiplus::ImageLockMode::ImageLockModeWrite,
			PixelFormat32bppARGB,
			&bitmapData);

		int num_pixels = bitmapData.Width * bitmapData.Height;
		INT32 *pixelPtr = (INT32*)bitmapData.Scan0;
		while (num_pixels--) {
			INT32 pixel = *pixelPtr;
			INT32 a = pixel & 0xFF000000, R = (pixel & 0xFF0000) >> 16, G = (pixel & 0xFF00) >> 8, B = pixel & 0xFF;
			INT32 Y = RGB2LUMA(R,G,B);

			*pixelPtr = a | (Y << 16) | (Y << 8) | Y;
			pixelPtr++;
		}

		bitmap->UnlockBits(&bitmapData);
	};

	bool CreateRedirectDC(HWND hWnd, const SIZE& size, HDC * out_hdc) {
		BITMAPINFOHEADER bih;
		ZeroMemory(&bih, sizeof(bih));

		bih.biSize = sizeof(bih);
		bih.biWidth = size.cx;
		bih.biHeight = -size.cy;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		void * pvImageBits = NULL;

		*out_hdc = ::CreateCompatibleDC(GetDC(hWnd));
		HBITMAP hbitmap = CreateDIBSection(*out_hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
		::SelectObject(*out_hdc, hbitmap);

		return true;
	}

	void GetLocalCoordinates(HWND hWnd, RECT* Rect)
	{
		GetWindowRect(hWnd, Rect);
		MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)Rect, 2);
	}

	void CloneHDC(const HDC & from, HDC * to){
		*to = CreateCompatibleDC(NULL);

		HBITMAP from_bitmap = (HBITMAP)GetCurrentObject(from, OBJ_BITMAP);
		BITMAP bitmap;
		GetObject(from_bitmap, sizeof(bitmap), &bitmap);

		BITMAPINFOHEADER bih;
		ZeroMemory(&bih, sizeof(bih));

		bih.biSize = sizeof(bih);
		bih.biWidth = bitmap.bmWidth;
		bih.biHeight = bitmap.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		void * pvImageBits = NULL;

		HBITMAP clone = CreateDIBSection(*to, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
		
		SIZE coordinates = { bitmap.bmWidth, bitmap.bmHeight };
		DPtoLP(*to, (LPPOINT)&coordinates, 1);
		memcpy(pvImageBits, bitmap.bmBits, bitmap.bmWidthBytes * bitmap.bmHeight);
		SelectObject(*to, clone);
	};

	void RenderGradient(HWND hWnd, HDC hdc, const std::vector<GrandientInfoEntry>& _colors){
		if (_colors.size() < 1){
			return;
		}
		std::vector<GrandientInfoEntry> colors(_colors.begin(), _colors.end());

		// If only one color is provided, make sure the info entry has size of 2 with the same color,
		// then redo the operation.
		if (_colors.size() == 1){
			colors.push_back(colors[0]);
			RenderGradient(hWnd, hdc, colors);
			return;
		}

		// Get device logical units
		RECT rcDevice, rcLogical;;
		GetClientRect(hWnd, &rcDevice);
		rcLogical = rcDevice;
		DPtoLP(hdc, (LPPOINT)&rcLogical, 2);
		SIZE sizeLogical = *((SIZE*)(LPPOINT)&rcLogical + 1);
		SIZE sizeDevice = *((SIZE*)(LPPOINT)&rcDevice + 1);

		GrandientInfoEntry & first = colors[0];

		TRIVERTEX *vertexes = new TRIVERTEX[_colors.size()];
		int index = 0, x = 0;
		for (GrandientInfoEntry& grandient_info_entry : colors){
			setTrivertex(&vertexes[index], grandient_info_entry.color, x, 
				(LONG)floor(grandient_info_entry.position / 100.0 * sizeDevice.cy));
			index++;
			x = sizeDevice.cx - x; // 0 or size.cx alternatively
		}
		vertexes[0].y = 0; // First one is always 0%
		vertexes[_colors.size() - 1].y = sizeDevice.cy; // Last one is always 100%

		GRADIENT_RECT * rects = new GRADIENT_RECT[_colors.size() - 1];
		for (unsigned int i = 0; i < _colors.size() - 1; i++){
			rects[i].UpperLeft = i;
			rects[i].LowerRight = i + 1;
		}

		// Paint gradient
		HDC inMemoryDC = ::CreateCompatibleDC(hdc);
		// Must expand to proper size, the initial state is 0 x 0.
		HBITMAP hBmp = ::CreateCompatibleBitmap(hdc, sizeDevice.cx, sizeDevice.cy);
		::SelectObject(inMemoryDC, hBmp);

		int resultGdiGradientFill = ::GdiGradientFill(inMemoryDC, vertexes, _colors.size(), rects, 
			_colors.size() - 1, GRADIENT_FILL_RECT_V);

		BLENDFUNCTION bfn = { 0 };
		bfn.BlendOp = AC_SRC_OVER;
		bfn.BlendFlags = 0;
		bfn.SourceConstantAlpha = 255;
		bfn.AlphaFormat = AC_SRC_ALPHA;

		int alphaBlend = GdiAlphaBlend(hdc, 0, 0, sizeLogical.cx, sizeLogical.cy,
			inMemoryDC, 0, 0, sizeDevice.cx, sizeDevice.cy, bfn); // Display bitmap	

		GradientRenderCount++;
		printf("RenderGradient[%05d] result: %d, %d\n", GradientRenderCount, resultGdiGradientFill, alphaBlend);
		
	finalizing:
		delete vertexes;
		delete rects;
		::DeleteDC(inMemoryDC);
		::DeleteObject(hBmp);
	};

	SIZE GetHDCSize(const HDC& hdc) {
		HBITMAP bitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
		if (!bitmap) {
			return{ 0, 0 };
		}

		BITMAP bitmapInfo;
		if (!GetObject(bitmap, sizeof(BITMAP), &bitmapInfo)) {
			return{ 0, 0 };
		};
		return{ bitmapInfo.bmWidth, bitmapInfo.bmHeight };
	};

	void MakeRoundRect(Gdiplus::Point topLeft, Gdiplus::Point bottomRight, INT roundRadius,
		Gdiplus::GraphicsPath* path) {

		//assert(percentageRounded >= 0 && percentageRounded <= 100);

		// Like they said in the X Files - Trust No One.  Rather than crash and burn,
		// I just pull out the correct edges from the points given, regardless of if they were wrong.

		INT left = fmin(topLeft.X, bottomRight.X);
		INT right = fmax(topLeft.X, bottomRight.X);

		INT top = fmin(topLeft.Y, bottomRight.Y);
		INT bottom = fmax(topLeft.Y, bottomRight.Y);

		/*INT offsetX = (right - left)*percentageRounded / 100;
		INT offsetY = (bottom - top)*percentageRounded / 100;*/

		INT offsetX = roundRadius;
		INT offsetY = roundRadius;

		//GraphicsPath * path = new GraphicsPath;

		path->AddArc(right - offsetX, top, offsetX, offsetY, 270, 90);

		path->AddArc(right - offsetX, bottom - offsetY, offsetX, offsetY, 0, 90);

		path->AddArc(left, bottom - offsetY, offsetX, offsetY, 90, 90);

		path->AddArc(left, top, offsetX, offsetY, 180, 90);

		path->AddLine(left + offsetX, top, right - offsetX / 2, top);
	}

	/*
	void MakeUpperLeftRoundRect(Gdiplus::Point topLeft, Gdiplus::Point bottomRight, INT roundRadius,
		Gdiplus::GraphicsPath* path) {

		//assert(percentageRounded >= 0 && percentageRounded <= 100);

		// Like they said in the X Files - Trust No One.  Rather than crash and burn,
		// I just pull out the correct edges from the points given, regardless of if they were wrong.

		INT left = fmin(topLeft.X, bottomRight.X);
		INT right = fmax(topLeft.X, bottomRight.X);

		INT top = fmin(topLeft.Y, bottomRight.Y);
		INT bottom = fmax(topLeft.Y, bottomRight.Y);

		//INT offsetX = (right - left)*percentageRounded / 100;
		//INT offsetY = (bottom - top)*percentageRounded / 100;

		INT offsetX = roundRadius;
		INT offsetY = roundRadius;

		//GraphicsPath * path = new GraphicsPath;

		path->AddArc(right - offsetX, top, offsetX, offsetY, 270, 90);

		path->AddArc(right - offsetX, bottom - offsetY, offsetX, offsetY, 0, 90);

		path->AddArc(left, bottom - offsetY, offsetX, offsetY, 90, 90);

		path->AddArc(left, top, offsetX, offsetY, 180, 90);

		path->AddLine(left + offsetX, top, right - offsetX / 2, top);
	}
	*/
}