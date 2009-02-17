/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "popup_dialog.h"
#include <windows.h>
#include <windowsx.h>
#include <vector>

#define ID_INPUT_FIELD 101

namespace ti {
	char Win32PopupDialog::textEntered[MAX_INPUT_LENGTH];
	int Win32PopupDialog::result = 0;

	// TODO there must be a better way to do this
	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int) s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	class DialogTemplate {
		public:
			LPCDLGTEMPLATE Template()
			{
				return (LPCDLGTEMPLATE)&v[0];
			}

			void AlignToDword()
			{
				if (v.size() % 4) Write(NULL, 4 - (v.size() % 4));
			}

			void Write(LPCVOID pvWrite, DWORD cbWrite)
			{
				v.insert(v.end(), cbWrite, 0);
				if (pvWrite)
				{
					CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
				}
			}

			template<typename T> void Write(T t)
			{
				Write(&t, sizeof(T));
			}

			void WriteString(LPCWSTR psz)
			{
				Write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR));
			}

		private:
			std::vector<BYTE> v;
	};

	Win32PopupDialog::Win32PopupDialog(HWND _windowHandle) : windowHandle(_windowHandle)
	{
		this->showInputText = false;
	}

	Win32PopupDialog::~Win32PopupDialog()
	{
	}

	int Win32PopupDialog::Show()
	{
		textEntered[0] = '\0';
		ShowMessageBox(windowHandle);

		this->inputText.clear();
		this->inputText.append(textEntered);

		return result;
	}

	INT_PTR CALLBACK Win32PopupDialog::Callback(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (iMsg)
		{
			case WM_INITDIALOG: return TRUE;
			case WM_COMMAND:
				if(GET_WM_COMMAND_ID(wParam, lParam) == IDOK)
				{
					GetDlgItemText(hDlg, ID_INPUT_FIELD, textEntered, MAX_INPUT_LENGTH);
					result = IDYES;
					EndDialog(hDlg, 0);
				}
				else if(GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL)
				{
					textEntered[0] = '\0';
					result = IDNO;
					EndDialog(hDlg, 0);
				}
				break;
		}

		return FALSE;
	}

	BOOL Win32PopupDialog::ShowMessageBox(HWND hwnd)
	{
		BOOL fSuccess = FALSE;
		HDC hdc = GetDC(hwnd);

		if (hdc)
		{
			NONCLIENTMETRICSW ncm = { sizeof(ncm) };
			if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0)) {
				DialogTemplate tmp;

				int controlCount = 2;	// at minimum, static label and OK button
				if(this->showCancelButton) controlCount++;
				if(this->showInputText) controlCount++;

				int labelHeight = 14;
				int width = 300;
				int height = 90;
				int margin = 10;
				int buttonWidth = 50;
				int buttonHeight = 14;
				int inputHeight = 14;


				// Write out the extended dialog template header
				tmp.Write<WORD>(1); // dialog version
				tmp.Write<WORD>(0xFFFF); // extended dialog template
				tmp.Write<DWORD>(0); // help ID
				tmp.Write<DWORD>(0); // extended style
				tmp.Write<DWORD>(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME);
				tmp.Write<WORD>(controlCount); // number of controls
				tmp.Write<WORD>(32); // X
				tmp.Write<WORD>(32); // Y
				tmp.Write<WORD>(width); // width
				tmp.Write<WORD>(height); // height
				tmp.WriteString(L""); // no menu
				tmp.WriteString(L""); // default dialog class
				//tmp.WriteString(pszTitle); // title
				tmp.WriteString(s2ws(this->title).c_str()); // title

				// Next comes the font description.
				// See text for discussion of fancy formula.
				if (ncm.lfMessageFont.lfHeight < 0)
				{
					ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
				}
				tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfHeight); // point
				tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfWeight); // weight
				tmp.Write<BYTE>(ncm.lfMessageFont.lfItalic); // Italic
				tmp.Write<BYTE>(ncm.lfMessageFont.lfCharSet); // CharSet
				tmp.WriteString(ncm.lfMessageFont.lfFaceName);

				// First control - static label
				tmp.AlignToDword();
				tmp.Write<DWORD>(0); // help id
				tmp.Write<DWORD>(0); // window extended style
				tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE); // style
				tmp.Write<WORD>(margin); // x
				tmp.Write<WORD>(margin); // y
				tmp.Write<WORD>(width - (2 * margin)); // width
				tmp.Write<WORD>(labelHeight); // height
				tmp.Write<DWORD>(-1); // control ID
				tmp.Write<DWORD>(0x0082FFFF); // static
				tmp.WriteString(s2ws(this->message).c_str()); // text
				tmp.Write<WORD>(0); // no extra data

				// Second control - the OK button.
				tmp.AlignToDword();
				tmp.Write<DWORD>(0); // help id
				tmp.Write<DWORD>(0); // window extended style
				tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
				tmp.Write<WORD>(width - margin - buttonWidth); // x
				tmp.Write<WORD>(height - margin - buttonHeight); // y
				tmp.Write<WORD>(buttonWidth); // width
				tmp.Write<WORD>(buttonHeight); // height
				tmp.Write<DWORD>(IDOK); // control ID
				tmp.Write<DWORD>(0x0080FFFF); // button class atom
				tmp.WriteString(L"OK"); // text
				tmp.Write<WORD>(0); // no extra data

				if(this->showCancelButton)
				{
					// The Cancel button
					tmp.AlignToDword();
					tmp.Write<DWORD>(0); // help id
					tmp.Write<DWORD>(0); // window extended style
					tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
					tmp.Write<WORD>(width - 2 * margin - 2 * buttonWidth); // x
					tmp.Write<WORD>(height - margin - buttonHeight); // y
					tmp.Write<WORD>(buttonWidth); // width
					tmp.Write<WORD>(buttonHeight); // height
					tmp.Write<DWORD>(IDCANCEL); // control ID
					tmp.Write<DWORD>(0x0080FFFF); // button class atom
					tmp.WriteString(L"Cancel"); // text
					tmp.Write<WORD>(0); // no extra data
				}

				if(this->showInputText)
				{
					// The input field
					tmp.AlignToDword();
					tmp.Write<DWORD>(0); // help id
					tmp.Write<DWORD>(0); // window extended style
					tmp.Write<DWORD>(ES_LEFT | WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE); // style
					tmp.Write<WORD>(margin); // x
					tmp.Write<WORD>(margin + labelHeight + margin); // y
					tmp.Write<WORD>(width - (2 * margin)); // width
					tmp.Write<WORD>(inputHeight); // height
					tmp.Write<DWORD>(ID_INPUT_FIELD); // control ID
					tmp.Write<DWORD>(0x0081FFFF); // edit class atom
					tmp.WriteString(s2ws(this->inputText).c_str()); // text
					tmp.Write<WORD>(0); // no extra data
				}

				// Template is ready - go display it.
				fSuccess = DialogBoxIndirect(GetModuleHandle(NULL), tmp.Template(), hwnd, &Win32PopupDialog::Callback) >= 0;
			}
			ReleaseDC(NULL, hdc); // fixed 11 May
		}

		return fSuccess;
	}
}
