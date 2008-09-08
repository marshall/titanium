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
** w16callb.c -- Implement Win16 Callback functions
**
** Functions here are to replace functions normally in
** LIBC which are not implemented in MSVC's LIBC.
** Some clients of NSPR expect to statically link
** to NSPR and get these functions.
**
** Some are implemented as callbacks to the .EXE
** some are implemented directly in this module.
**
*/

#include "primpl.h"
#include "windowsx.h"

/*
** _pr_callback_funcs -- This is where clients register the 
** callback function structure.
*/
struct PRMethodCallbackStr * _pr_callback_funcs;

/*
** PR_MDInitWin16() -- Register the PRMethodCallback table pointer
** 
*/
void PR_MDRegisterCallbacks(struct PRMethodCallbackStr *f)
{
    _pr_callback_funcs = f;
}

/*
** NSPR re-implenentations of various C runtime functions:
*/

/*
** PR_MD_printf() -- exported as printf()
**
*/
int  PR_MD_printf(const char *fmt, ...)
{
    char buffer[1024];
    int ret = 0;
    va_list args;

    va_start(args, fmt);

#ifdef DEBUG
    PR_vsnprintf(buffer, sizeof(buffer), fmt, args);
    {   
        if (_pr_callback_funcs != NULL && _pr_callback_funcs->auxOutput != NULL) {
            (* _pr_callback_funcs->auxOutput)(buffer);
        } else {
            OutputDebugString(buffer);
        }
    }
#endif

    va_end(args);
    return ret;
}

/*
** PR_MD_sscanf() -- exported as sscanf()
**
*/
int  PR_MD_sscanf(const char *buf, const char *fmt, ...)
{
	int		retval;
	va_list	arglist;

	va_start(arglist, fmt);
	retval = vsscanf((const unsigned char *)buf, (const unsigned char *)fmt, arglist);
	va_end(arglist);
	return retval;
}

/*
** PR_MD_strftime() -- exported as strftime
**
*/
size_t PR_MD_strftime(char *s, size_t len, const char *fmt, const struct tm *p) 
{
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->strftime)(s, len, fmt, p);
    } else {
        PR_ASSERT(0);
        return 0;
    }
}


/*
** PR_MD_malloc() -- exported as malloc()
**
*/
void *PR_MD_malloc( size_t size )
{
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->malloc)( size );
    } else {
        return GlobalAllocPtr(GPTR, (DWORD)size);
    }
} /* end malloc() */

/*
** PR_MD_calloc() -- exported as calloc()
**
*/
void *PR_MD_calloc( size_t n, size_t size )
{
    void *p;
    size_t sz;
    
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->calloc)( n, size );
    } else {
        sz = n * size;
        p = GlobalAllocPtr(GPTR, (DWORD)sz );
        memset( p, 0x00, sz );
        return p;
    }
} /* end calloc() */

/*
** PR_MD_realloc() -- exported as realloc()
**
*/
void *PR_MD_realloc( void* old_blk, size_t size )
{
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->realloc)( old_blk, size );
    } else {
        return GlobalReAllocPtr( old_blk, (DWORD)size, GPTR);
    }
} /* end realloc */

/*
** PR_MD_free() -- exported as free()
**
*/
void PR_MD_free( void *ptr )
{
    if( _pr_callback_funcs ) {
        (*_pr_callback_funcs->free)( ptr );
        return;
    } else {
        GlobalFreePtr( ptr );
        return;
    }
} /* end free() */

/*
** PR_MD_getenv() -- exported as getenv()
**
*/
char *PR_MD_getenv( const char *name )
{
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->getenv)( name );
    } else {
        return 0;
    }
} /* end getenv() */


/*
** PR_MD_perror() -- exported as perror()
**
** well, not really (lth. 12/5/97).
** XXX hold this thought.
**
*/
void PR_MD_perror( const char *prefix )
{
    return;
} /* end perror() */

/*
** PR_MD_putenv() -- exported as putenv()
**
*/
int  PR_MD_putenv(const char *assoc)
{
    if( _pr_callback_funcs ) {
        return (*_pr_callback_funcs->putenv)(assoc);
    } else {
        PR_ASSERT(0);
        return NULL;
    }
}

/*
** PR_MD_fprintf() -- exported as fprintf()
**
*/
int  PR_MD_fprintf(FILE *fPtr, const char *fmt, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    PR_vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (fPtr == NULL) 
    {
        if (_pr_callback_funcs != NULL && _pr_callback_funcs->auxOutput != NULL) 
        {
            (* _pr_callback_funcs->auxOutput)(buffer);
        } 
        else 
        {
            OutputDebugString(buffer);
        }
    } 
    else 
    {
        fwrite(buffer, 1, strlen(buffer), fPtr); /* XXX Is this a sec. hole? */
    }

    va_end(args);
    return 0;
}

/* end w16callb.c */
