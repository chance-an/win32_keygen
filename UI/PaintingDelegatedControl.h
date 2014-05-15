#pragma once

namespace UI{
	class PaintingDelegatedControl{
		public:
			PaintingDelegatedControl(HWND parent, HWND self);
			PaintingDelegatedControl(HWND parent, HWND self, HWND paintRedirectWindow);
			PaintingDelegatedControl& operator=(const PaintingDelegatedControl& rhs);

			virtual ~PaintingDelegatedControl() {
				if (backup_drawing_section != nullptr) {
					::DeleteObject(backup_drawing_section);
				}

				if (_delegated_dc != nullptr) {
					::DeleteDC(_delegated_dc);
				}

				if (replaced_bitmap != nullptr) {
					::DeleteObject(replaced_bitmap);
				}
			}

			HWND GetHWND() {
				return self;
			}

			struct PaintStruct {
				HDC& hdc;
				const bool isTransparent;
			};
		protected:
			HWND self;
			HWND parent;
			HWND paintingRedirectWindow;

			void resetRedirectDC();
			void sendPaintingToParentWindow();
			bool transferDCToBackupDC(const HDC&);

			virtual bool isOpaque();
			
			HDC getRedirectPaintingDC() {
				return _delegated_dc;
			}

			HBITMAP replaced_bitmap;
		private:
			bool _initialized;
			HDC _delegated_dc;
			HBITMAP backup_drawing_section;
			void initialize();
	};
}
