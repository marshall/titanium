/***********************************************************************
 * testaudio.h
 *  
 *    Audio Library
 *
 *
 * Toby Opferman Copyright (c) 2003
 ***
 * Modifications Copyright (c) 2008 Appcelerator Inc.
 * Licensed under APL 2.0
 ***********************************************************************/


#ifndef __WAVELIB_H__
#define __WAVELIB_H__

#include <windows.h>
#include <mmsystem.h>

typedef PVOID HWAVELIB;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DonePlayingFunc)(void *data);

HWAVELIB WaveLib_Init(const wchar_t *pAudioFile, BOOL bLoop, BOOL bPause, DonePlayingFunc func, void *data);
void WaveLib_UnInit(HWAVELIB hWaveLib);
void WaveLib_Pause(HWAVELIB hWaveLib, BOOL bPause);
void WaveLib_Loop(HWAVELIB hWaveLib, BOOL bLoop);
HWAVEOUT WaveLib_GetWaveOut(HWAVELIB hWaveLib);

#ifdef __cplusplus
}
#endif



#endif

