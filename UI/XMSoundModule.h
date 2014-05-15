#pragma once
#include "DSound.h"

#define uFMOD_MixRate 48000

namespace UI{

	class XMSoundModule {
	public:
		XMSoundModule(HWND assoicateHWND);
		virtual ~XMSoundModule();

		void initialize();
		void play();
		void stop();
		void pause();
		void resume();
		void finalize();

	private:
		bool _initialized;
		HWND _assoicateHWND;
		HMODULE  hDSoundLib;
		IDirectSound *lpDS;
		IDirectSoundBuffer* lpDSBufferPrimary;
		IDirectSoundBuffer *lpDSBuffer;
		int isDX_old = 0;

		int DS_init(HWND hWnd);
	};
}