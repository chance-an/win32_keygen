#pragma once

#include "PaintingDelegatedControl.h"

#define CUSTOM_BUTTON_ANIMATION_TIMER_ID 1

namespace UI{

	class CustomButton : public UI::PaintingDelegatedControl
	{
	public:
		CustomButton() : PaintingDelegatedControl(nullptr, nullptr){
			backupParentBackground = nullptr;
		};
		CustomButton(HWND parent, int controlId, HWND paintingRedirectWindow);
		~CustomButton();

	protected:
		enum ButtonState{
			BS_NORMAL,
			BS_HOVER,
			BS_CLICKED
		};

		enum ButtonFlashingState{
			BFS_GRADE0,
			BFS_GRADE1,
			BFS_GRADE2,
			BFS_GRADE3
		};

		virtual LRESULT onClick(HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam);

		virtual LRESULT onPaint(WPARAM wParam, LPARAM lParam);

		RECT getDrawingContour();

		__int32 getButtonState(){
			return _buttonState;
		}

	private:
		int controlId;
		HDC backupParentBackground;
		HBITMAP backupParentBackgroundBitMap;
		__int32 _buttonState;
		UINT_PTR _animationTimer;

		std::function<void()> onMouseDown;
		std::function<void()> onMouseUp;

		bool isOpaque();


		static LRESULT CALLBACK SUBCLASSPROC(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			UINT_PTR uIdSubclass,
			DWORD_PTR dwRefData
			);

		static int getAnimationFrameRate(){
			// TODO use global configuration
			return 100; // milliseconds
		}


		LRESULT onMouseEnter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT onMouseLeave(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void CALLBACK buttonAnimationFadeIn(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		static void CALLBACK buttonAnimationFadeOut(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	};

}