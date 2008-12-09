/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "resource.h"
#include "ti_splash.h"
#include <stdio.h>

bool TiSplash::shown = false;
TiSplash* TiSplash::instance_ = NULL;

#define SPLASH_WIDTH 200
#define SPLASH_HEIGHT 70

void TiSplash::createSplash()
{
	hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_TITANIUM_SPLASH), NULL, TiSplash::DlgProc, 0);
}

TiSplash::TiSplash(HINSTANCE hInstance_) : hInstance(hInstance_)
{
	createSplash();
}

void TiSplash::show() {
	shown  = true;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

void TiSplash::hide()
{
	shown = false;
	EndDialog(hWnd, 0);
}

/*static*/
INT_PTR CALLBACK TiSplash::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}