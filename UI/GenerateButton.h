#pragma once
#include "../stdafx.h"
#include "CustomButton.h"

const TCHAR INVALID[] = _T("[Please Input Your Name]");

class GenerateButton : public UI::CustomButton{
public:
	GenerateButton() : CustomButton(){};
	GenerateButton(HWND parent, int controlId, HWND paintingRedirectWindow) : 
		CustomButton(parent, controlId, paintingRedirectWindow){}
private:
	LRESULT onClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
