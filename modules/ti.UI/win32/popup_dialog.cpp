/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "popup_dialog.h"

namespace ti {
	Win32PopupDialog::Win32PopupDialog(HWND _windowHandle) : windowHandle(_windowHandle)
	{
		this->showInputText = false;
	}

	Win32PopupDialog::~Win32PopupDialog()
	{
	}

	int Win32PopupDialog::Show()
	{
		std::cout << "should show popup dialog" << std::endl;

		// TODO - construct and display the dialog

		this->inputText.clear();
		this->inputText.append("this is the typed text");

		return IDYES;
	}

	BOOL Win32PopupDialog::Callback(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		return TRUE;
	}
}
