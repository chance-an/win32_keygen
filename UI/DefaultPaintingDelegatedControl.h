#pragma once

#include "PaintingDelegatedControl.h"

namespace UI{
	class DefaultPaintingDelegatedControl : public PaintingDelegatedControl{
	public:
		DefaultPaintingDelegatedControl(HWND parent, int controlId);
		DefaultPaintingDelegatedControl(HWND parent, HWND self);
		
	protected:
		virtual LRESULT draw(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT onChar(UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		int controlId;
		void initialize();

		static LRESULT CALLBACK SUBCLASSPROC(
			HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
			DWORD_PTR dwRefData);

	};
}