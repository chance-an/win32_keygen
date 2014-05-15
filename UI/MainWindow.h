#include "PaintingDelegatedControl.h"
#include "CustomButton.h"
#include "XMSoundModule.h"

namespace UI {
	class MainWindow{
	public:
		MainWindow::MainWindow();
		MainWindow(HWND presentation);

		virtual ~MainWindow();
		
		HWND controlWindow;
		HWND presentationWindow;

		HWND getControl(INT controlID);

		XMSoundModule* GetMusicModule() {
			return this->_XMSoundModule.get();
		};

		PaintingDelegatedControl* GetNameEdit() {
			return nameEdit;
		};

		friend BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
	protected:
		std::unique_ptr<UI::CustomButton> okButton;
		std::unique_ptr<UI::CustomButton> generateButton;
		std::unique_ptr<UI::CustomButton> musicButton;
	private:
		HDC mainWindowPaintingRedirectDC;
		PaintingDelegatedControl* nameEdit;

		// Records the clean, initial PNG background without controls.
		HDC backupRedirectDC;

		SIZE initialSize;

		std::vector<std::unique_ptr<PaintingDelegatedControl>> paintingDelegatedControls;
		std::unique_ptr<XMSoundModule> _XMSoundModule;
		static void RegisterWindowClass();

		static LRESULT CALLBACK PresentationWndProc(
			_In_  HWND hwnd,
			_In_  UINT uMsg,
			_In_  WPARAM wParam,
			_In_  LPARAM lParam
			);

		static INT_PTR CALLBACK ControlWindowHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		void initializeDialogPosition(HWND hWnd);
		void initializeControls();
		void initializeSound();
		void retrieveInitialMetrics();
		void repositionWindow();
		void redirectLayeredWindowDrawing();
		void renderChildControlToPresentationWindow(HWND child, const PaintingDelegatedControl::PaintStruct* child_hdc);

		LRESULT onPaint();
		void onMoveWindow(LPARAM lParam);
	};
}