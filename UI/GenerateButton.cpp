#include "stdafx.h"
#include "../stdafx.h"

#include "boost/algorithm/string/trim.hpp"
#include "GenerateButton.h"
#include "PaintingDelegatedEditControl.h"
#include "../KeyGen.h"
#include "../main.h"
#include "MainWindow.h"

const LRESULT alwaysSuccess = 1;
extern std::unique_ptr<UI::MainWindow> mainWindow;

LRESULT GenerateButton::onClick(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	printf("Generate clicked\n");

	/*InvalidateRect(GetDlgItem(parent, IDOK), NULL, true);
	UpdateWindow(GetDlgItem(parent, IDOK));
*/
	TCHAR name[100];
	TCHAR tmp[100];
	TCHAR output[100];

	HWND controlName = mainWindow->getControl(IDC_EDIT_NAME);
	HWND controlSn = mainWindow->getControl(IDC_EDIT_SN);

	[&](){
		if (!controlName) {
			printf("CTR_EDIT_NAME doesn't exit. %d\n", 0);
			return;
		}

		int getLength = Edit_GetText(controlName, name, 100);

		std::wcout << getLength << ": " << name << std::endl;

		if (!getLength) {
			//Edit_SetText(controlName, INVALID);
			// TODO: notify control state
			// TODO: more sanity check.
			static_cast<UI::PaintingDelegatedEditControl*>(mainWindow->GetNameEdit())->setShowPrompt(true);
			PostMessage(mainWindow->GetNameEdit()->GetHWND(), WM_PAINT, NULL, NULL);
			return;
		}

		// tirm
		_tcscpy_s(name, 100, boost::algorithm::trim_copy(std::wstring(name)).c_str());
		if (_tcscmp(name, INVALID) == 0) {
			return ;
		}

		//std::wcout << "trim" << ": " << name << std::endl;

		int result = KeyGen::generate(name, getLength, output, 100);
		if (result == 0) {
			Edit_SetText(controlSn, output);
		}

	}();
	
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
};