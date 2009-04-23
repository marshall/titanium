#include <windows.h>
#include <new.h>
#include <objbase.h>
#include <vector>
#include <string>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils.h>
#include "Progress.h"
#include "Resource.h"
#include "IntroDialog.h"

using std::string;
using std::wstring;
using kroll::Application;
using kroll::FileUtils;

extern HINSTANCE mainInstance;
extern HICON mainIcon;

extern Application* app;
extern string updateFile;
extern string appPath;
extern string runtimeHome;
extern string appInstallPath;
extern bool doInstall;
extern bool installStartMenuIcon;

extern wstring StringToWString(string);

HWND nameLabel = NULL;
HWND versionLabel = NULL;
HWND publisherLabel = NULL;
HWND urlLabel = NULL;
HWND licenseBlurb = NULL;
HWND licenseTextBox = NULL;
HWND installBox = NULL;
HWND installLocationText = NULL;
HWND installLocationButton = NULL;
HWND startMenuCheck = NULL;
HWND securityBlurb = NULL;
HWND runButton = NULL;
HWND cancelButton = NULL;

void IntializeDialog(HWND hwnd)
{
	nameLabel = GetDlgItem(hwnd, IDC_NAME_LABEL);
	versionLabel = GetDlgItem(hwnd, IDC_VERSION_LABEL);
	publisherLabel = GetDlgItem(hwnd, IDC_PUBLISHER_LABEL);
	urlLabel = GetDlgItem(hwnd, IDC_URL_LABEL);
	licenseTextBox = GetDlgItem(hwnd, IDC_LICENSETEXT);
	licenseBlurb = GetDlgItem(hwnd, IDC_LICENSE_BLURB);
	installBox = GetDlgItem(hwnd, IDC_INSTALL_BOX);
	installLocationText = GetDlgItem(hwnd, IDC_INSTALL_LOCATION_EDIT);
	installLocationButton = GetDlgItem(hwnd, IDC_INSTALL_LOCATION_BUTTON);
	startMenuCheck = GetDlgItem(hwnd, IDC_START_MENU_CHECK);
	securityBlurb = GetDlgItem(hwnd, IDC_SECURITY_BLURB);
	runButton = GetDlgItem(hwnd, IDC_RUN);
	cancelButton = GetDlgItem(hwnd, IDC_CANCEL);

	// Set the name label's font to be a bit bigger
	LOGFONT newFontStruct;
	HFONT currentFont = (HFONT) SendMessage(nameLabel, WM_GETFONT, (WPARAM)0, (LPARAM)0);
	GetObject(currentFont, sizeof newFontStruct, &newFontStruct);
	newFontStruct.lfWeight = FW_BOLD;
	newFontStruct.lfHeight = 30;
	HFONT newFont = CreateFontIndirect(&newFontStruct);
	SendMessage(nameLabel, WM_SETFONT, (WPARAM)newFont, LPARAM(0));
	SendMessage(nameLabel, WM_SETTEXT, 0, (LPARAM) app->name.c_str());

	string version = "Unknown";
	if (!app->version.empty())
		version = app->version;
	if (!updateFile.empty())
		version.append(" (Update)");
	SendMessage(versionLabel, WM_SETTEXT, 0, (LPARAM) version.c_str());

	if (!app->publisher.empty())
		SendMessage(publisherLabel, WM_SETTEXT, 0, (LPARAM) app->publisher.c_str());
	if (!app->url.empty())
		SendMessage(urlLabel, WM_SETTEXT, 0, (LPARAM) app->url.c_str());

	// Set license text
	string licenseText = app->GetLicenseText();
	if (licenseText.empty())
	{
		::ShowWindow(licenseTextBox , SW_HIDE);
		::ShowWindow(licenseBlurb , SW_HIDE);

		// *wince* -- I can't believe I'm about to do this
		int width = 530;
		int height = 260;
		SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		int installBoxX = 10;
		int installBoxY = 110;
		SetWindowPos(installBox, NULL, installBoxX, installBoxY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(installLocationText, NULL, installBoxX + 8, installBoxY + 17, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(installLocationButton, NULL, width - 73, installBoxY + 16, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(startMenuCheck, NULL, installBoxX + 12, installBoxY + 45, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(securityBlurb, NULL, installBoxX + 25, installBoxY + 70, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		
		SetWindowPos(runButton, NULL, width - 90, installBoxY + 95, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(cancelButton, NULL, width - 175, installBoxY + 95, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SendMessage(licenseTextBox, WM_SETTEXT, 0, (LPARAM) licenseText.c_str());
	}

	// Hide installation location controls when this isn't a full app installation
	if (app->IsInstalled())
	{
		::ShowWindow(installLocationButton , SW_HIDE);
		::ShowWindow(startMenuCheck , SW_HIDE);
	}

	SendMessage(installLocationText, WM_SETTEXT, 0, (LPARAM) appInstallPath.c_str());

	// Set intro dialog icon
	SendMessage(hwnd, WM_SETICON, (WPARAM)true, (LPARAM)mainIcon);

	// Center the dialog
	HDC hScreenDC = CreateCompatibleDC(NULL);
	int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
	int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);
	DeleteDC(hScreenDC);
	RECT dialogRect;
	GetWindowRect(hwnd, &dialogRect);
	int centerX = ( screenWidth - (dialogRect.right - dialogRect.left)) / 2;
	int centerY = ( screenHeight - (dialogRect.bottom - dialogRect.top)) / 2;
	SetWindowPos(hwnd, NULL, centerX, centerY-20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void InstallLocationClicked()
{
	BROWSEINFO bi = { 0 };
	bi.lpszTitle =  (LPCSTR) "Pick installation directory";
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != 0)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			appInstallPath = FileUtils::Join(path, app->name.c_str(), NULL);
			SendMessage(installLocationText, WM_SETTEXT, 0, (LPARAM) appInstallPath.c_str());
		}

		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

	}
}
BOOL CALLBACK DialogProc(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	int controlID = LOWORD(wParam);
	int command = HIWORD (wParam);
	switch (message)
	{
		case WM_INITDIALOG:
			IntializeDialog(hwnd);
			return TRUE;

		case WM_COMMAND:
			if (controlID == IDC_CANCEL && command == BN_CLICKED)
			{
				DestroyWindow(hwnd);
			}
			else if (controlID == IDC_RUN && command == BN_CLICKED)
			{
				doInstall = true;
				LRESULT checked = SendMessage(startMenuCheck, BM_GETCHECK, 0, 0);
				if (checked == BST_CHECKED)
					installStartMenuIcon = true;

				DestroyWindow(hwnd);
			}
			else if (controlID == IDC_INSTALL_LOCATION_BUTTON && command == BN_CLICKED)
			{
				InstallLocationClicked();
			}
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return TRUE;
	}
	return FALSE;
}
