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


#ifndef TI_SPLASH_H_
#define TI_SPLASH_H_

#include <windows.h>

class TiSplash
{
private:
	static TiSplash *instance_;
	static bool shown;
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE hInstance;
	HWND hWnd;
	void createSplash();

	TiSplash(HINSTANCE hInstance);

public:
	void show();
	void hide();

	static TiSplash* instance() {
		return instance_;
	}

	static TiSplash* init(HINSTANCE hInstance) {
		if (instance_ == NULL) {
			instance_ = new TiSplash(hInstance);
		}
		return instance_;
	}
	
	static bool isShown() { return shown; }
};

#endif