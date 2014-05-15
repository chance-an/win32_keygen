#include "stdafx.h"
#include "../stdafx.h"
#include "XMSoundModule.h"
#include "../dsufmod.h"
#include "../Resource.h"

namespace UI{
	const TCHAR szDSOUND[] = _T("dsound");

	DSBUFFERDESC dsbuffdesc = {
		sizeof(DSBUFFERDESC),
		DSBCAPS_PRIMARYBUFFER,
	};

	WAVEFORMATEX pcm = {
		WAVE_FORMAT_PCM, /* linear PCM */
		2,               /* stereo     */
		uFMOD_MixRate,
		uFMOD_MixRate * 4,
		4, 16             /* 16-bit     */
	};

	XMSoundModule::XMSoundModule(HWND assoicateHWND) :
		_initialized(false),
		_assoicateHWND(assoicateHWND),
		hDSoundLib(NULL),
		lpDS(nullptr),
		lpDSBufferPrimary(nullptr),
		lpDSBuffer(nullptr){
	};

	XMSoundModule::~XMSoundModule() {
		finalize();
	}

	void XMSoundModule::initialize() {
		DS_init(_assoicateHWND);
		_initialized = true;
	}

	void XMSoundModule::finalize() {
		if (lpDS) {
			stop();
			IDirectSound_Release(lpDS);
		}
	}

	void XMSoundModule::play() {
		assert(_initialized);
		uFMOD_DSPlaySong((VOID *)XM_RRLD1, 0, XM_RESOURCE, lpDSBuffer);
	}

	void XMSoundModule::stop() {
		assert(_initialized);
		uFMOD_StopSong();
	}

	void XMSoundModule::pause() {
		assert(_initialized);
		uFMOD_Pause(); 
	}

	void XMSoundModule::resume() {
		assert(_initialized);
		uFMOD_Resume();
	}

	int XMSoundModule::DS_init(HWND hWnd){
		typedef long int(__stdcall *LPDSCREATE)(void*, void*, void*);

		LPDSCREATE DS_Create;
		/* Get a pointer to IDirectSound. */
		hDSoundLib = LoadLibrary(szDSOUND);
		if (!hDSoundLib) return -1;
		DS_Create = (LPDSCREATE)GetProcAddress(hDSoundLib, "DirectSoundCreate8");
		if (!DS_Create){
			DS_Create = (LPDSCREATE)GetProcAddress(hDSoundLib, (char*)1);
			if (!DS_Create) return -1;
			/* For compatibility with DirectX 7 and earlier. */
			dsbuffdesc.dwSize = 20;
			isDX_old++;
		}
		if (DS_Create(0, &lpDS, 0) < 0) return -1;
		/*
		It is important to set the cooperative level to at least
		DSSCL_PRIORITY prior to creating 16-bit stereo buffers.
		*/
		//if (IDirectSound_SetCooperativeLevel(lpDS, hWnd, DSSCL_PRIORITY) < 0)
		if (lpDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY) < 0)
			return -1;
		/* Get the primary buffer. */
		if (IDirectSound_CreateSoundBuffer(lpDS, &dsbuffdesc, &lpDSBufferPrimary, 0) < 0)
			return -1;
		/* Set format to PCM / 48 KHz / 16-bit / stereo. */
		if (IDirectSoundBuffer_SetFormat(lpDSBufferPrimary, &pcm) < 0)
			return -1;
		/* Create the secondary buffer. */
		dsbuffdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 |
			/*
			Some earlier DirectX versions didn't
			track the current position correctly
			when this flag is not specified.
			*/
			DSBCAPS_STICKYFOCUS;
		/*
		Enable playback even if the main window
		is not active, but currently active
		window don't have exclusive access to
		DirectSound.
		*/
		/* Enable sound effects. */
		if (!isDX_old) dsbuffdesc.dwFlags |= DSBCAPS_CTRLFX;
		dsbuffdesc.dwBufferBytes = uFMOD_BUFFER_SIZE;
		dsbuffdesc.lpwfxFormat = &pcm;
		if (IDirectSound_CreateSoundBuffer(lpDS, &dsbuffdesc, &lpDSBuffer, 0) < 0)
			return -1;
		return 0;
	}
}