/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape Portable Runtime (NSPR).
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * Contributors:
 *
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per
 * MPL Section 3.3
 *
 * Date         Modified by     Description of modification
 * 04/10/2000   IBM Corp.       Added DebugBreak() definitions for OS/2
 */

#include "primpl.h"
#include "prenv.h"
#include "prprf.h"
#include <string.h>

/*
 * Lock used to lock the log.
 *
 * We can't define _PR_LOCK_LOG simply as PR_Lock because PR_Lock may
 * contain assertions.  We have to avoid assertions in _PR_LOCK_LOG
 * because PR_ASSERT calls PR_LogPrint, which in turn calls _PR_LOCK_LOG.
 * This can lead to infinite recursion.
 */
static PRLock *_pr_logLock;
#if defined(_PR_PTHREADS) || defined(_PR_BTHREADS)
#define _PR_LOCK_LOG() PR_Lock(_pr_logLock);
#define _PR_UNLOCK_LOG() PR_Unlock(_pr_logLock);
#elif defined(_PR_GLOBAL_THREADS_ONLY)
#define _PR_LOCK_LOG() { _PR_LOCK_LOCK(_pr_logLock)
#define _PR_UNLOCK_LOG() _PR_LOCK_UNLOCK(_pr_logLock); }
#else

#define _PR_LOCK_LOG() \
{ \
    PRIntn _is; \
    PRThread *_me = _PR_MD_CURRENT_THREAD(); \
    if (!_PR_IS_NATIVE_THREAD(_me)) \
        _PR_INTSOFF(_is); \
    _PR_LOCK_LOCK(_pr_logLock)

#define _PR_UNLOCK_LOG() \
    _PR_LOCK_UNLOCK(_pr_logLock); \
    PR_ASSERT(_me == _PR_MD_CURRENT_THREAD()); \
    if (!_PR_IS_NATIVE_THREAD(_me)) \
        _PR_INTSON(_is); \
}

#endif

#if defined(XP_PC)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/*
 * On NT, we can't define _PUT_LOG as PR_Write or _PR_MD_WRITE,
 * because every asynchronous file io operation leads to a fiber context
 * switch.  So we define _PUT_LOG as fputs (from stdio.h).  A side
 * benefit is that fputs handles the LF->CRLF translation.  This
 * code can also be used on other platforms with file stream io.
 */
#if defined(WIN32) || defined(XP_OS2)
#define _PR_USE_STDIO_FOR_LOGGING
#endif

/*
** Coerce Win32 log output to use OutputDebugString() when
** NSPR_LOG_FILE is set to "WinDebug".
*/
#if defined(XP_PC)
#define WIN32_DEBUG_FILE (FILE*)-2
#endif

/* Macros used to reduce #ifdef pollution */

#if defined(_PR_USE_STDIO_FOR_LOGGING) && defined(XP_PC)
#define _PUT_LOG(fd, buf, nb) \
    PR_BEGIN_MACRO \
    if (logFile == WIN32_DEBUG_FILE) { \
        char savebyte = buf[nb]; \
        buf[nb] = '\0'; \
        OutputDebugString(buf); \
        buf[nb] = savebyte; \
    } else { \
        fwrite(buf, 1, nb, fd); \
        fflush(fd); \
    } \
    PR_END_MACRO
#elif defined(_PR_USE_STDIO_FOR_LOGGING)
#define _PUT_LOG(fd, buf, nb) {fwrite(buf, 1, nb, fd); fflush(fd);}
#elif defined(_PR_PTHREADS)
#define _PUT_LOG(fd, buf, nb) PR_Write(fd, buf, nb)
#elif defined(XP_MAC)
#define _PUT_LOG(fd, buf, nb) _PR_MD_WRITE_SYNC(fd, buf, nb)
#else
#define _PUT_LOG(fd, buf, nb) _PR_MD_WRITE(fd, buf, nb)
#endif

/************************************************************************/

static PRLogModuleInfo *logModules;

static char *logBuf = NULL;
static char *logp;
static char *logEndp;
#ifdef _PR_USE_STDIO_FOR_LOGGING
static FILE *logFile = NULL;
#else
static PRFileDesc *logFile = 0;
#endif

#define LINE_BUF_SIZE           512
#define DEFAULT_BUF_SIZE        16384

#ifdef _PR_NEED_STRCASECMP

/*
 * strcasecmp is defined in /usr/ucblib/libucb.a on some platforms
 * such as NCR and Unixware.  Linking with both libc and libucb
 * may cause some problem, so I just provide our own implementation
 * of strcasecmp here.
 */

static const unsigned char uc[] =
{
    '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
    '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
    '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
    '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
    ' ',    '!',    '"',    '#',    '$',    '%',    '&',    '\'',
    '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
    '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    'X',    'Y',    'Z',    '[',    '\\',   ']',    '^',    '_',
    '`',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    'X',    'Y',    'Z',    '{',    '|',    '}',    '~',    '\177'
};

PRIntn strcasecmp(const char *a, const char *b)
{
    const unsigned char *ua = (const unsigned char *)a;
    const unsigned char *ub = (const unsigned char *)b;

    if( ((const char *)0 == a) || (const char *)0 == b ) 
        return (PRIntn)(a-b);

    while( (uc[*ua] == uc[*ub]) && ('\0' != *a) )
    {
        a++;
        ua++;
        ub++;
    }

    return (PRIntn)(uc[*ua] - uc[*ub]);
}

#endif /* _PR_NEED_STRCASECMP */

void _PR_InitLog(void)
{
    char *ev;

    _pr_logLock = PR_NewLock();

    ev = PR_GetEnv("NSPR_LOG_MODULES");
    if (ev && ev[0]) {
        char module[64];  /* Security-Critical: If you change this
                           * size, you must also change the sscanf
                           * format string to be size-1.
                           */
        PRBool isSync = PR_FALSE;
        PRIntn evlen = strlen(ev), pos = 0;
        PRInt32 bufSize = DEFAULT_BUF_SIZE;
        while (pos < evlen) {
            PRIntn level = 1, count = 0, delta = 0;
            count = sscanf(&ev[pos], "%63[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-]%n:%d%n",
                           module, &delta, &level, &delta);
            pos += delta;
            if (count == 0) break;

            /*
            ** If count == 2, then we got module and level. If count
            ** == 1, then level defaults to 1 (module enabled).
            */
            if (strcasecmp(module, "sync") == 0) {
                isSync = PR_TRUE;
            } else if (strcasecmp(module, "bufsize") == 0) {
                if (level >= LINE_BUF_SIZE) {
                    bufSize = level;
                }
            } else {
                PRLogModuleInfo *lm = logModules;
                PRBool skip_modcheck =
                    (0 == strcasecmp (module, "all")) ? PR_TRUE : PR_FALSE;

                while (lm != NULL) {
                    if (skip_modcheck) lm -> level = (PRLogModuleLevel)level;
                    else if (strcasecmp(module, lm->name) == 0) {
                        lm->level = (PRLogModuleLevel)level;
                        break;
                    }
                    lm = lm->next;
                }
            }
            /*found:*/
            count = sscanf(&ev[pos], " , %n", &delta);
            pos += delta;
            if (count == EOF) break;
        }
        PR_SetLogBuffering(isSync ? 0 : bufSize);

#ifdef XP_UNIX
        if ((getuid() != geteuid()) || (getgid() != getegid())) {
            return;
        }
#endif /* XP_UNIX */

        ev = PR_GetEnv("NSPR_LOG_FILE");
        if (ev && ev[0]) {
            if (!PR_SetLogFile(ev)) {
#ifdef XP_PC
                char* str = PR_smprintf("Unable to create nspr log file '%s'\n", ev);
                if (str) {
                    OutputDebugString(str);
                    PR_smprintf_free(str);
                }
#else
                fprintf(stderr, "Unable to create nspr log file '%s'\n", ev);
#endif
            }
        } else {
#ifdef _PR_USE_STDIO_FOR_LOGGING
            logFile = stderr;
#else
            logFile = _pr_stderr;
#endif
        }
    }
}

void _PR_LogCleanup(void)
{
    PRLogModuleInfo *lm = logModules;

    PR_LogFlush();

#ifdef _PR_USE_STDIO_FOR_LOGGING
    if (logFile
        && logFile != stdout
        && logFile != stderr
#ifdef XP_PC
        && logFile != WIN32_DEBUG_FILE
#endif
        ) {
        fclose(logFile);
        logFile = NULL;
    }
#else
    if (logFile && logFile != _pr_stdout && logFile != _pr_stderr) {
        PR_Close(logFile);
        logFile = NULL;
    }
#endif

    if (logBuf)
        PR_DELETE(logBuf);

    while (lm != NULL) {
        PRLogModuleInfo *next = lm->next;
        free((/*const*/ char *)lm->name);
        PR_Free(lm);
        lm = next;
    }
    logModules = NULL;

    if (_pr_logLock) {
        PR_DestroyLock(_pr_logLock);
        _pr_logLock = NULL;
    }
}

static void _PR_SetLogModuleLevel( PRLogModuleInfo *lm )
{
    char *ev;

    ev = PR_GetEnv("NSPR_LOG_MODULES");
    if (ev && ev[0]) {
        char module[64];  /* Security-Critical: If you change this
                           * size, you must also change the sscanf
                           * format string to be size-1.
                           */
        PRIntn evlen = strlen(ev), pos = 0;
        while (pos < evlen) {
            PRIntn level = 1, count = 0, delta = 0;

            count = sscanf(&ev[pos], "%63[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-]%n:%d%n",
                           module, &delta, &level, &delta);
            pos += delta;
            if (count == 0) break;

            /*
            ** If count == 2, then we got module and level. If count
            ** == 1, then level defaults to 1 (module enabled).
            */
            if (lm != NULL)
            {
                if ((strcasecmp(module, "all") == 0)
                    || (strcasecmp(module, lm->name) == 0))
                {
                    lm->level = (PRLogModuleLevel)level;
                }
            }
            count = sscanf(&ev[pos], " , %n", &delta);
            pos += delta;
            if (count == EOF) break;
        }
    }
} /* end _PR_SetLogModuleLevel() */

PR_IMPLEMENT(PRLogModuleInfo*) PR_NewLogModule(const char *name)
{
    PRLogModuleInfo *lm;

        if (!_pr_initialized) _PR_ImplicitInitialization();

    lm = PR_NEWZAP(PRLogModuleInfo);
    if (lm) {
        lm->name = strdup(name);
        lm->level = PR_LOG_NONE;
        lm->next = logModules;
        logModules = lm;
        _PR_SetLogModuleLevel(lm);
    }
    return lm;
}

PR_IMPLEMENT(PRBool) PR_SetLogFile(const char *file)
{
#ifdef _PR_USE_STDIO_FOR_LOGGING
    FILE *newLogFile;

#ifdef XP_PC
    if ( strcmp( file, "WinDebug") == 0)
    {
        newLogFile = WIN32_DEBUG_FILE;
    }
    else
#endif
    {
        newLogFile = fopen(file, "w");
        if (!newLogFile)
            return PR_FALSE;

        /* We do buffering ourselves. */
        setvbuf(newLogFile, NULL, _IONBF, 0);
    }
    if (logFile
        && logFile != stdout
        && logFile != stderr
#ifdef XP_PC
        && logFile != WIN32_DEBUG_FILE
#endif
        ) {
        fclose(logFile);
    }
    logFile = newLogFile;
    return PR_TRUE;
#else
    PRFileDesc *newLogFile;

    newLogFile = PR_Open(file, PR_WRONLY|PR_CREATE_FILE|PR_TRUNCATE, 0666);
    if (newLogFile) {
        if (logFile && logFile != _pr_stdout && logFile != _pr_stderr) {
            PR_Close(logFile);
        }
        logFile = newLogFile;
#if defined(XP_MAC)
        SetLogFileTypeCreator(file);
#endif
    }
    return (PRBool) (newLogFile != 0);
#endif /* _PR_USE_STDIO_FOR_LOGGING */
}

PR_IMPLEMENT(void) PR_SetLogBuffering(PRIntn buffer_size)
{
    PR_LogFlush();

    if (logBuf)
        PR_DELETE(logBuf);

    if (buffer_size >= LINE_BUF_SIZE) {
        logp = logBuf = (char*) PR_MALLOC(buffer_size);
        logEndp = logp + buffer_size;
    }
}

PR_IMPLEMENT(void) PR_LogPrint(const char *fmt, ...)
{
    va_list ap;
    char line[LINE_BUF_SIZE];
    char *line_long = NULL;
    PRUint32 nb_tid, nb;
    PRThread *me;

    if (!_pr_initialized) _PR_ImplicitInitialization();

    if (!logFile) {
        return;
    }

    me = PR_GetCurrentThread();
    nb_tid = PR_snprintf(line, sizeof(line)-1, "%ld[%p]: ",
#if defined(_PR_DCETHREADS)
             /* The problem is that for _PR_DCETHREADS, pthread_t is not a 
              * pointer, but a structure; so you can't easily print it...
              */
                         me ? &(me->id): 0L, me);
#elif defined(_PR_BTHREADS)
                         me, me);
#else
                         me ? me->id : 0L, me);
#endif

    va_start(ap, fmt);
    nb = nb_tid + PR_vsnprintf(line+nb_tid, sizeof(line)-nb_tid-1, fmt, ap);
    va_end(ap);

    /*
     * Check if we might have run out of buffer space (in case we have a
     * long line), and malloc a buffer just this once.
     */
    if (nb == sizeof(line)-2) {
        va_start(ap, fmt);
        line_long = PR_vsmprintf(fmt, ap);
        va_end(ap);
        /* If this failed, we'll fall back to writing the truncated line. */
    }

    if (line_long) {
        nb = strlen(line_long);
        _PR_LOCK_LOG();
        if (logBuf != 0) {
            _PUT_LOG(logFile, logBuf, logp - logBuf);
            logp = logBuf;
        }
        /* Write out the thread id and the malloc'ed buffer. */
        _PUT_LOG(logFile, line, nb_tid);
        _PUT_LOG(logFile, line_long, nb);
        /* Ensure there is a trailing newline. */
        if (!nb || (line_long[nb-1] != '\n')) {
            char eol[2];
            eol[0] = '\n';
            eol[1] = '\0';
            _PUT_LOG(logFile, eol, 1);
        }
        _PR_UNLOCK_LOG();
        PR_smprintf_free(line_long);
    } else {
        /* Ensure there is a trailing newline. */
        if (nb && (line[nb-1] != '\n')) {
            line[nb++] = '\n';
            line[nb] = '\0';
        }
        _PR_LOCK_LOG();
        if (logBuf == 0) {
            _PUT_LOG(logFile, line, nb);
        } else {
            /* If nb can't fit into logBuf, write out logBuf first. */
            if (logp + nb > logEndp) {
                _PUT_LOG(logFile, logBuf, logp - logBuf);
                logp = logBuf;
            }
            /* nb is guaranteed to fit into logBuf. */
            memcpy(logp, line, nb);
            logp += nb;
        }
        _PR_UNLOCK_LOG();
    }
    PR_LogFlush();
}

PR_IMPLEMENT(void) PR_LogFlush(void)
{
    if (logBuf && logFile) {
        _PR_LOCK_LOG();
            if (logp > logBuf) {
                _PUT_LOG(logFile, logBuf, logp - logBuf);
                logp = logBuf;
            }
        _PR_UNLOCK_LOG();
    }
}

PR_IMPLEMENT(void) PR_Abort(void)
{
    PR_LogPrint("Aborting");
    abort();
}

#if defined(XP_OS2)
/*
 * Added definitions for DebugBreak() for 2 different OS/2 compilers.
 * Doing the int3 on purpose for Visual Age so that a developer can
 * step over the instruction if so desired.  Not always possible if
 * trapping due to exception handling IBM-AKR
 */
#if defined(XP_OS2_VACPP)
#include <builtin.h>
static void DebugBreak(void) { _interrupt(3); }
#elif defined(XP_OS2_EMX)
static void DebugBreak(void) { asm("int $3"); }
#else
static void DebugBreak(void) { }
#endif
#endif /* XP_OS2 */

PR_IMPLEMENT(void) PR_Assert(const char *s, const char *file, PRIntn ln)
{
    PR_LogPrint("Assertion failure: %s, at %s:%d\n", s, file, ln);
#if defined(XP_UNIX) || defined(XP_OS2) || defined(XP_BEOS)
    fprintf(stderr, "Assertion failure: %s, at %s:%d\n", s, file, ln);
#endif
#ifdef XP_MAC
    dprintf("Assertion failure: %s, at %s:%d\n", s, file, ln);
#endif
#if defined(WIN32) || defined(XP_OS2)
    DebugBreak();
#endif
#ifndef XP_MAC
    abort();
#endif
}

#ifdef XP_MAC
PR_IMPLEMENT(void) PR_Init_Log(void)
{
	_PR_InitLog();
}
#endif
