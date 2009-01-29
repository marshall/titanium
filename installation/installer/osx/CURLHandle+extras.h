//
//  CURLHandle+extras.h
//
//  Created by Dan Wood <dwood@karelia.com> on Mon Oct 01 2001.
//  This is in the public domain, but please report any improvements back to the author.
//
//	The current version of CURLHandle is 1.9.
//

#import "CURLHandle.h"


@interface CURLHandle ( extras )

/*" Miscellaneous functions "*/

- (void) setProgressIndicator:(id)inProgressIndicator;

/*" Set options for the transfer "*/

- (void) setConnectionTimeout:(long) inSeconds;
- (void) setTransferTimeout:(long) inSeconds;
- (void) setCookieFile:(NSString *)inFilePath;
- (void) setRequestCookies:(NSDictionary *)inDict;
- (void) setFailsOnError:(BOOL)inFlag;
- (void) setFollowsRedirects:(BOOL)inFlag;
- (void) setPostString:(NSString *)inPostString;
- (void) setPostDictionary:(NSDictionary *)inDictionary;
- (void) setPostDictionary:(NSDictionary *)inDictionary encoding:(NSStringEncoding) inEncoding;
- (void) setReferer:(NSString *)inReferer;
- (void) setUserAgent:(NSString *)inUserAgent;
- (void) setUserName:(NSString*)inUserName password:(NSString *)inPassword;
- (void) setNoBody:(BOOL)inNoBody;
- (void) setRange:(NSString *)inRange;
- (void) setIfModSince:(NSDate *)inModDate;
- (void) setLowSpeedTime:(long) inSeconds;
- (void) setLowSpeedLimit:(long) inBytes;
- (void) setVerbose: (BOOL) beVerbose;

/*" Get information about the transfer "*/

- (double)downloadContentLength;
- (double)downloadSize;
- (double)downloadSpeed;
- (double)nameLookupTime;
- (double)pretransferTime;
- (double)totalTime;
- (double)uploadContentLength;
- (double)uploadSize;
- (double)uploadSpeed;
- (long)fileTime;
- (long)headerSize;
- (long)httpCode;
- (long)requestSize;

/*" Multipart post operations "*/
- (void) setMultipartPostDictionary: (NSDictionary *) inDictionary;
- (void) setMultipartPostDictionary: (NSDictionary *) values headers: (NSDictionary *) headers;


@end
