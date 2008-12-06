/***********************************************************************
 * wavelib.c
 *  
 *    Audio Library
 *
 *
 *  Supports .WAV files, Very Simplistic Parser
 *
 *
 * Toby Opferman Copyright (c) 2003
 * **
 * Modifications (c) 2008 Appcelerator Inc
 * Licensed under APL 2.0
 ***********************************************************************/
 
 
 #include <wavelib.h>
 
 
 
 
 /***********************************************************************
  * Internal Structures
  ***********************************************************************/
typedef struct {
    
    UCHAR IdentifierString[4];
    DWORD dwLength;

} RIFF_CHUNK, *PRIFF_CHUNK;


typedef struct {

    WORD  wFormatTag;         // Format category
    WORD  wChannels;          // Number of channels
    DWORD dwSamplesPerSec;    // Sampling rate
    DWORD dwAvgBytesPerSec;   // For buffer estimation
    WORD  wBlockAlign;        // Data block size
    WORD  wBitsPerSample;
    

} WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;


typedef struct _wave_sample {

     WAVEFORMATEX WaveFormatEx;
     char *pSampleData;
     UINT Index;
     UINT Size;
     DWORD dwId;
     DWORD bPlaying;
     struct _wave_sample *pNext;

} WAVE_SAMPLE, *PWAVE_SAMPLE;
 
#define SAMPLE_SIZE    (2*2*2000) 

typedef struct {
     
     HWAVEOUT hWaveOut;
     HANDLE hEvent;
     HANDLE hThread;
     WAVE_SAMPLE WaveSample;
     BOOL bWaveShouldDie;
     WAVEHDR WaveHdr[8];
     char AudioBuffer[8][SAMPLE_SIZE];
     BOOL bPaused;
	 BOOL bLoop;
	 DonePlayingFunc func;
	 void *userData;

} WAVELIB, *PWAVELIB;



 /***********************************************************************
  * Internal Functions
  ***********************************************************************/
void CALLBACK WaveLib_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
BOOL WaveLib_OpenWaveSample(const wchar_t *pFileName, PWAVE_SAMPLE pWaveSample);
void WaveLib_WaveOpen(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
void WaveLib_WaveDone(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
DWORD WINAPI WaveLib_AudioThread(PVOID pDataInput);
void WaveLib_CreateThread(PWAVELIB pWaveLib);
void WaveLib_SetupAudio(PWAVELIB pWaveLib);
void WaveLib_WaveClose(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
void WaveLib_AudioBuffer(PWAVELIB pWaveLib, UINT Index);


 /***********************************************************************
  * WaveLib_Init
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 HWAVELIB WaveLib_Init(const wchar_t *pWaveFile, BOOL bLoop, BOOL bPause, DonePlayingFunc func, void *userData)
 {
     PWAVELIB pWaveLib = NULL;
 
     if(pWaveLib = (PWAVELIB)LocalAlloc(LMEM_ZEROINIT, sizeof(WAVELIB)))
     {
         pWaveLib->bPaused = bPause;
		 pWaveLib->bLoop = bLoop;
		 pWaveLib->func = func;
		 pWaveLib->userData = userData;

         if(WaveLib_OpenWaveSample(pWaveFile, &pWaveLib->WaveSample))
         {
             if(waveOutOpen(&pWaveLib->hWaveOut, WAVE_MAPPER, &pWaveLib->WaveSample.WaveFormatEx, (ULONG)WaveLib_WaveOutputCallback, (ULONG)pWaveLib, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
             {
                WaveLib_UnInit((HWAVELIB)pWaveLib);
                pWaveLib = NULL;
             }
             else
             {
 
                 if(pWaveLib->bPaused)
                 {
                     waveOutPause(pWaveLib->hWaveOut);
                 }

                 WaveLib_CreateThread(pWaveLib);
             }
         }
         else
         {
             WaveLib_UnInit((HWAVELIB)pWaveLib);
             pWaveLib = NULL;
         }
     }

     return (HWAVELIB)pWaveLib;
 }



 /***********************************************************************
  * WaveLib_Pause
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 void WaveLib_Pause(HWAVELIB hWaveLib, BOOL bPause)
 {
     PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;

     pWaveLib->bPaused = bPause;

     if(pWaveLib->bPaused)
     {
         waveOutPause(pWaveLib->hWaveOut);
     }
     else
     {
         waveOutRestart(pWaveLib->hWaveOut);
     }
 }

 void WaveLib_Loop(HWAVELIB hWaveLib, BOOL bLoop)
 {
	PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;
	BOOL wasLooping = pWaveLib->bLoop;
	pWaveLib->bLoop = bLoop;

	if (!wasLooping) {
		SetEvent(pWaveLib->hEvent);
	}
 }

 HWAVEOUT WaveLib_GetWaveOut(HWAVELIB hWaveLib)
 {
	PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;
	return pWaveLib->hWaveOut;
 }


 /***********************************************************************
  * WaveLib_Init
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 void WaveLib_UnInit(HWAVELIB hWaveLib)
 {
     PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;

     if(pWaveLib)
     {
         if(pWaveLib->hThread)
         {
             pWaveLib->bWaveShouldDie = TRUE;

             SetEvent(pWaveLib->hEvent);
             WaitForSingleObject(pWaveLib->hThread, INFINITE);

             CloseHandle(pWaveLib->hEvent);
             CloseHandle(pWaveLib->hThread);
         }

         if(pWaveLib->hWaveOut)
         {
             waveOutClose(pWaveLib->hWaveOut);
         }


         if(pWaveLib->WaveSample.pSampleData)
         {
             LocalFree(pWaveLib->WaveSample.pSampleData);
         }

         LocalFree(pWaveLib);
     }

 }
 
 
 /***********************************************************************
  * WaveLib_WaveOutputCallback
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/ 
void CALLBACK WaveLib_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    PWAVELIB pWaveLib = (PWAVELIB)dwInstance;

    switch(uMsg)
    {
      case WOM_OPEN:
            WaveLib_WaveOpen(hwo, pWaveLib);
            break;

       case WOM_DONE:
            WaveLib_WaveDone(hwo, pWaveLib);
            break;

       case WOM_CLOSE:
            WaveLib_WaveClose(hwo, pWaveLib);
            break;
    }
}



 
 /***********************************************************************
  * WaveLib_WaveOpen
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_WaveOpen(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{
  // Do Nothing
}


 /***********************************************************************
  * WaveLib_WaveDone
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_WaveDone(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{
    SetEvent(pWaveLib->hEvent);
}


 /***********************************************************************
  * WaveLib_WaveClose
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_WaveClose(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{

}



 /***********************************************************************
  * WaveLib_OpenWaveFile
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
BOOL WaveLib_OpenWaveSample(const wchar_t *pFileName, PWAVE_SAMPLE pWaveSample)
{
    BOOL bSampleLoaded = FALSE;
    HANDLE hFile;
    RIFF_CHUNK RiffChunk = {0};
    DWORD dwBytes, dwReturnValue;
    WAVE_FILE_HEADER WaveFileHeader;
    DWORD dwIncrementBytes;

    if(hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL))
    {
        char szIdentifier[5] = {0};

        SetFilePointer(hFile, 12, NULL, FILE_CURRENT);
        

        ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
        ReadFile(hFile, &WaveFileHeader, sizeof(WaveFileHeader), &dwBytes, NULL);

        pWaveSample->WaveFormatEx.wFormatTag      = WaveFileHeader.wFormatTag;         
        pWaveSample->WaveFormatEx.nChannels       = WaveFileHeader.wChannels;          
        pWaveSample->WaveFormatEx.nSamplesPerSec  = WaveFileHeader.dwSamplesPerSec;    
        pWaveSample->WaveFormatEx.nAvgBytesPerSec = WaveFileHeader.dwAvgBytesPerSec;   
        pWaveSample->WaveFormatEx.nBlockAlign     = WaveFileHeader.wBlockAlign;  
        pWaveSample->WaveFormatEx.wBitsPerSample  = WaveFileHeader.wBitsPerSample;
        pWaveSample->WaveFormatEx.cbSize          = 0;

        dwIncrementBytes = dwBytes;

        do {
             SetFilePointer(hFile, RiffChunk.dwLength - dwIncrementBytes, NULL, FILE_CURRENT);
             
             dwReturnValue = GetLastError();

             if(dwReturnValue == 0)
             {
                 dwBytes = ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
             
                 dwIncrementBytes = 0;

                 memcpy(szIdentifier, RiffChunk.IdentifierString, 4); 
             }

        } while(_stricmp(szIdentifier, "data") && dwReturnValue == 0) ;

        if(dwReturnValue == 0)
        {
            pWaveSample->pSampleData = (char *)LocalAlloc(LMEM_ZEROINIT, RiffChunk.dwLength);

            pWaveSample->Size = RiffChunk.dwLength;

            ReadFile(hFile, pWaveSample->pSampleData, RiffChunk.dwLength, &dwBytes, NULL);

            CloseHandle(hFile);

            bSampleLoaded = TRUE;
        }
    }

    return bSampleLoaded;
}





 /***********************************************************************
  * WaveLib_CreateThread
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_CreateThread(PWAVELIB pWaveLib)
{
    DWORD dwThreadId;

    pWaveLib->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    pWaveLib->hThread = CreateThread(NULL, 0, WaveLib_AudioThread, pWaveLib, 0, &dwThreadId);

}

 /***********************************************************************
  * WaveLib_AudioThread
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
DWORD WINAPI WaveLib_AudioThread(PVOID pDataInput)
{
    PWAVELIB pWaveLib = (PWAVELIB)pDataInput;
    DWORD dwReturnValue = 0;
    UINT Index;

    WaveLib_SetupAudio(pWaveLib);

    while(!pWaveLib->bWaveShouldDie)
    {
		if (!pWaveLib->bLoop)
			WaitForSingleObject(pWaveLib->hEvent, INFINITE);

        for(Index = 0; Index < 8; Index++)
        {
            if(pWaveLib->WaveHdr[Index].dwFlags & WHDR_DONE)
            {
               WaveLib_AudioBuffer(pWaveLib, Index);
               waveOutWrite(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));
            }
        }
			
		if (!pWaveLib->bLoop) {
			if (pWaveLib->func != NULL) {
				pWaveLib->func(pWaveLib->userData);
			}
		}
    }

    waveOutReset(pWaveLib->hWaveOut);

    return dwReturnValue;
}




 /***********************************************************************
  * WaveLib_AudioMixer
  *  
  *    Audio Mixer
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_AudioBuffer(PWAVELIB pWaveLib, UINT Index)
{
    UINT uiBytesNotUsed = SAMPLE_SIZE;

    pWaveLib->WaveHdr[Index].dwFlags &= ~WHDR_DONE;

    if(pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index < uiBytesNotUsed)
    {
        memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData + pWaveLib->WaveSample.Index, pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index);

        uiBytesNotUsed -= (pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index);

        memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData, uiBytesNotUsed);

        pWaveLib->WaveSample.Index = uiBytesNotUsed;

        uiBytesNotUsed = 0;
    }
    else
    {
       memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData + pWaveLib->WaveSample.Index, uiBytesNotUsed);

       pWaveLib->WaveSample.Index += SAMPLE_SIZE;
       uiBytesNotUsed = 0;
    }

    pWaveLib->WaveHdr[Index].lpData = pWaveLib->AudioBuffer[Index];

    pWaveLib->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE - uiBytesNotUsed;
}






 /***********************************************************************
  * WaveLib_SetupAudio
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_SetupAudio(PWAVELIB pWaveLib)
{
    UINT Index = 0;

    for(Index = 0; Index < 8; Index++)
    {
        pWaveLib->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE;
        pWaveLib->WaveHdr[Index].lpData         = pWaveLib->AudioBuffer[Index]; 

        waveOutPrepareHeader(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));

        WaveLib_AudioBuffer(pWaveLib, Index);

        waveOutWrite(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));

    }
}


