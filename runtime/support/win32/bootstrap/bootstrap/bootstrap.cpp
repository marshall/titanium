/**
 * This file is part of Appcelerator's Titanium project.
 *
 * Copyright 2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */
#include "stdafx.h"
#include "bootstrap.h"
#include "unzip.h"
#include <stdio.h>

HINSTANCE instance;
DWORD threadId = 0;
HANDLE thread = NULL;
HWND billboard;
HANDLE threadEvent;


/**
 * This is the Titanium Win32 bootstrap executable that is used to launch
 * the real titanium runtime process which is contained inside the executable.
 *
 * This file has a zip file of an application archive in ZIP format which is 
 * appended directly to the end of this binary (append as binary, not text!)
 * and will be ignored by window.  the zip uncompression process reads from the
 * end of the file and will extract the archive into a temporary directory
 * and then execute the titanium runtime to run the application.
 *
 */


bool DirectoryExists(const char* dirName)
{
	DWORD attribs = GetFileAttributesA(dirName);
	if (attribs == INVALID_FILE_ATTRIBUTES) 
	{
		return false;
	}
	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL CALLBACK BillboardProc(HWND hwndDlg, 
                            UINT message, 
                            WPARAM wParam, 
                            LPARAM lParam) 
{ 
    switch (message) 
    { 
        case WM_INITDIALOG: 
            SetWindowPos( hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
            return TRUE;
    } 

    return FALSE; 
} 

DWORD WINAPI StaticThreadProc( LPVOID lpParameter )
{
    MSG msg;
    HANDLE startEvent = *(HANDLE*)lpParameter;  // thread's read event

	billboard = CreateDialog(instance, 
						MAKEINTRESOURCE(IDD_FORMVIEW), 
						GetDesktopWindow(), 
						BillboardProc);

    ShowWindow(billboard, SW_SHOW); 

	SetEvent(startEvent); // Signal event

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        if (!::IsDialogMessage( billboard, &msg ))
        {
            if (msg.message == WM_USER + 1 )
            {
                // Tell the dialog to destroy itself
                DestroyWindow(billboard);

                // Tell our thread to break out of message pump
                PostThreadMessage( threadId, WM_QUIT, 0, 0 );
            }
        }
    } 

    return( 0L );
}

void ShowBillboard()
{
	threadEvent = CreateEvent( 
        NULL,     // no security attributes
        FALSE,    // auto-reset event
        FALSE,    // initial state is not signaled
        NULL);    // object not named


	thread = CreateThread(NULL, 
                            0L, 
                            StaticThreadProc, 
                            (LPVOID)&threadEvent, 
                            0, 
                            &threadId );

    // Wait for any message sent or posted to this queue 
    // or for one of the passed handles be set to signaled.
	// This is important as the window does not get created until
	// the StaticThreadProc gets called

	HANDLE handles[1];
	handles[0] = threadEvent;
    DWORD result = WaitForMultipleObjects(1, handles, FALSE, INFINITE); 
}

void TeardownBillboard()
{
	if (thread == NULL)
		return;

    // Tell the thread to destroy the modeless dialog
    while (!(PostThreadMessage( threadId, WM_USER+1, 0, 0 )))
	{
		Sleep(5);
	}

    WaitForSingleObject( thread, INFINITE );
    CloseHandle( thread );

	thread = NULL;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
	instance = hInstance;
	
	// put up the splash screen
	ShowBillboard();	

	// get the path to this executable
	HMODULE module = GetModuleHandle(NULL);
	char path[MAX_PATH];
	GetModuleFileName(module,(char*)&path,512);

	// get the directory that the executable lives in
	char *ch = strrchr(path,'\\');
	char dir[MAX_PATH];
	strcpy(dir,path);
	dir[strlen(dir)-strlen(ch)+1] = '\0';

	// try and setup a temporary directory
	char tempdir[MAX_PATH];
	sprintf(tempdir,"%s\\.ti",dir);

	bool extract = true, cleanup = false;

	// first check to see if the directory exists
	if (!DirectoryExists(tempdir))
	{
		// doesn't exist, try and create our temporary directory
		// and make it hidden
		if (CreateDirectory(tempdir,NULL))
		{
			SetFileAttributes(tempdir,FILE_ATTRIBUTE_HIDDEN);
		}
		else
		{
			// we couldn't make the directory for whatever reason,
			// fall back to using a temp directory
			GetTempPath(MAX_PATH,tempdir);
			cleanup = true;
		}
	}
	else
	{
		// this is a subsequent run and we've already extracted our
		// files, go ahead and skip the extract and just run
		extract = false;
	}

	// we need to extract our zip into our tempdir
	if (extract)
	{
		HZIP hz = OpenZip(path,0);
		SetUnzipBaseDir(hz,tempdir);
		ZIPENTRY ze; 
		GetZipItem(hz,-1,&ze); 
		for (int zi=0; zi < ze.index; zi++)
		{ 
			GetZipItem(hz,zi,&ze);
			UnzipItem(hz,zi,ze.name);
		}
		CloseZip(hz);
	}

	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

	char exepath[MAX_PATH];
	DWORD exitCode = 1;

	// inside the zip must be an executable named titanium.exe
	// that is our main titanium runtime that is generic
	sprintf(exepath,"%s\\titanium.exe",tempdir);

	// launch our subprocess as a child of this process 
	// and wait for it to exit before we exit
    if (CreateProcessA( exepath,	
						lpCmdLine,	// pass along any arguments passed to us
						0, 
						0, 
						false, 
						CREATE_DEFAULT_ERROR_MODE, 
						0, 
						tempdir,	// set working directory to tempdir
                        &si, 
						&pi) != false)
     {
		  // we wait for 3 seconds and then hide the splash
		  // but we want on the process in case it exists before our timeout
          WaitForSingleObject(pi.hProcess,3000);

		  // Tell the thread to destroy the modeless dialog
          TeardownBillboard();
		  thread = NULL;

          // Watch the process
          exitCode = WaitForSingleObject(pi.hProcess,INFINITE);
     }
     else
     {
          // CreateProcess failed
          exitCode = GetLastError();
     }

	 // clean up
	 CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);

	 // if we had to use a temporary directory, go ahead and remove 
	 // it and clean up
	 if (cleanup)
	 {
		 RemoveDirectory(tempdir);
	 }
    
	 // in case we failed launch
	 TeardownBillboard();
	 
	 return exitCode;
}

