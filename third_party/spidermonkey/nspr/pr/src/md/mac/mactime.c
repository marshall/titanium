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

#include <OSUtils.h>
#include <Timer.h>

#include "primpl.h"

#include "mactime.h"

unsigned long gJanuaryFirst1970Seconds;

/*
 * The geographic location and time zone information of a Mac
 * are stored in extended parameter RAM.  The ReadLocation
 * produdure uses the geographic location record, MachineLocation,
 * to read the geographic location and time zone information in
 * extended parameter RAM.
 *
 * Because serial port and SLIP conflict with ReadXPram calls,
 * we cache the call here.
 *
 * Caveat: this caching will give the wrong result if a session
 * extend across the DST changeover time.
 */

static void MyReadLocation(MachineLocation *loc)
{
    static MachineLocation storedLoc;
    static Boolean didReadLocation = false;
    
    if (!didReadLocation) {
        ReadLocation(&storedLoc);
        didReadLocation = true;
    }
    *loc = storedLoc;
}

static long GMTDelta(void)
{
    MachineLocation loc;
    long gmtDelta;

    MyReadLocation(&loc);
    gmtDelta = loc.u.gmtDelta & 0x00ffffff;
    if (gmtDelta & 0x00800000) {    /* test sign extend bit */
        gmtDelta |= 0xff000000;
    }
    return gmtDelta;
}

void MacintoshInitializeTime(void)
{
    /*
     * The NSPR epoch is midnight, Jan. 1, 1970 GMT.
     *
     * At midnight Jan. 1, 1970 GMT, the local time was
     *     midnight Jan. 1, 1970 + GMTDelta().
     *
     * Midnight Jan. 1, 1970 is 86400 * (365 * (1970 - 1904) + 17)
     *     = 2082844800 seconds since the Mac epoch.
     * (There were 17 leap years from 1904 to 1970.)
     *
     * So the NSPR epoch is 2082844800 + GMTDelta() seconds since
     * the Mac epoch.  Whew! :-)
     */
    gJanuaryFirst1970Seconds = 2082844800 + GMTDelta();
}

/*
 *-----------------------------------------------------------------------
 *
 * PR_Now --
 *
 *     Returns the current time in microseconds since the epoch.
 *     The epoch is midnight January 1, 1970 GMT.
 *     The implementation is machine dependent.  This is the Mac
 *     Implementation.
 *     Cf. time_t time(time_t *tp)
 *
 *-----------------------------------------------------------------------
 */

PRTime PR_Now(void)
{
    unsigned long currentTime;    /* unsigned 32-bit integer, ranging
                                     from midnight Jan. 1, 1904 to 
                                     6:28:15 AM Feb. 6, 2040 */
    PRTime retVal;
    int64  usecPerSec;

    /*
     * Get the current time expressed as the number of seconds
     * elapsed since the Mac epoch, midnight, Jan. 1, 1904 (local time).
     * On a Mac, current time accuracy is up to a second.
     */
    GetDateTime(&currentTime);

    /*
     * Express the current time relative to the NSPR epoch,
     * midnight, Jan. 1, 1970 GMT.
     *
     * At midnight Jan. 1, 1970 GMT, the local time was
     *     midnight Jan. 1, 1970 + GMTDelta().
     *
     * Midnight Jan. 1, 1970 is 86400 * (365 * (1970 - 1904) + 17)
     *     = 2082844800 seconds since the Mac epoch.
     * (There were 17 leap years from 1904 to 1970.)
     *
     * So the NSPR epoch is 2082844800 + GMTDelta() seconds since
     * the Mac epoch.  Whew! :-)
     */
    currentTime = currentTime - 2082844800 - GMTDelta();

    /* Convert seconds to microseconds */
    LL_I2L(usecPerSec, PR_USEC_PER_SEC);
    LL_I2L(retVal, currentTime);
    LL_MUL(retVal, retVal, usecPerSec);

    return retVal;
}

/*
 *-------------------------------------------------------------------------
 *
 * PR_LocalTimeParameters --
 * 
 *     returns the time parameters for the local time zone
 *
 *     This is the machine-dependent implementation for Mac.
 *
 *     Caveat: On a Mac, we only know the GMT and DST offsets for
 *     the current time, not for the time in question.
 *     Mac has no support for DST handling.
 *     DST changeover is all manually set by the user.
 *
 *-------------------------------------------------------------------------
 */

PRTimeParameters PR_LocalTimeParameters(const PRExplodedTime *gmt)
{
#pragma unused (gmt)

    PRTimeParameters retVal;
    MachineLocation loc;

    MyReadLocation(&loc);

    /* 
     * On a Mac, the GMT value is in seconds east of GMT.  For example,
     * San Francisco is at -28,800 seconds (8 hours * 3600 seconds per hour)
     * east of GMT.  The gmtDelta field is a 3-byte value contained in a
     * long word, so you must take care to get it properly.
     */

    retVal.tp_gmt_offset = loc.u.gmtDelta & 0x00ffffff;
    if (retVal.tp_gmt_offset & 0x00800000) {    /* test sign extend bit */
	retVal.tp_gmt_offset |= 0xff000000;
    }

    /*
     * The daylight saving time value, dlsDelta, is a signed byte
     * value representing the offset for the hour field -- whether
     * to add 1 hour, subtract 1 hour, or make no change at all.
     */

    if (loc.u.dlsDelta) {
    	retVal.tp_gmt_offset -= 3600;
    	retVal.tp_dst_offset = 3600;
    } else {
    	retVal.tp_dst_offset = 0;
    }
    return retVal;
}

PRIntervalTime _MD_GetInterval(void)
{
    PRIntervalTime retVal;
    PRUint64 upTime, microtomilli;	

    /*
     * Use the Microseconds procedure to obtain the number of
     * microseconds elapsed since system startup time.
     */
    Microseconds((UnsignedWide *)&upTime);
    LL_I2L(microtomilli, PR_USEC_PER_MSEC);
    LL_DIV(upTime, upTime, microtomilli);
    LL_L2I(retVal, upTime);
	
    return retVal;
}

struct tm *Maclocaltime(const time_t * t)
{
	DateTimeRec dtr;
	MachineLocation loc;
	time_t macLocal = *t + gJanuaryFirst1970Seconds; /* GMT Mac */
	static struct tm statictime;
	static const short monthday[12] =
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	SecondsToDate(macLocal, &dtr);
	statictime.tm_sec = dtr.second;
	statictime.tm_min = dtr.minute;
	statictime.tm_hour = dtr.hour;
	statictime.tm_mday = dtr.day;
	statictime.tm_mon = dtr.month - 1;
	statictime.tm_year = dtr.year - 1900;
	statictime.tm_wday = dtr.dayOfWeek - 1;
	statictime.tm_yday = monthday[statictime.tm_mon]
		+ statictime.tm_mday - 1;
	if (2 < statictime.tm_mon && !(statictime.tm_year & 3))
		++statictime.tm_yday;
	MyReadLocation(&loc);
	statictime.tm_isdst = loc.u.dlsDelta;
	return(&statictime);
}


