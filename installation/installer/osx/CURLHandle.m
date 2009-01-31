//
//  CURLHandle.m
//
//  Created by Dan Wood <dwood@karelia.com> on Fri Jun 22 2001.
//  This is in the public domain, but please report any improvements back to the author.
//
//	The current version of CURLHandle is 2.0
//

#import "CURLHandle.h"
#import <Cocoa/Cocoa.h>
#define NSS(s) (NSString *)(s)
#include <SystemConfiguration/SystemConfiguration.h>



// Un-comment these to do some debugging things
//#define DEBUGCURL 1
//#define DEBUGCURL_SLOW

enum { DONE = 0, BAD = 0xBAD, HEAD = 0xEAD, BODY = 0xB0D };	// Clever, eh?

enum { NOT_LOADING, LOADING };

NSMutableSet		*sAcceptedURLs = nil;
BOOL				sAcceptAllHTTP = NO;
BOOL				sAllowsProxy = YES;		// by default, allow proxy to be used./
NSMutableDictionary *sCurlCache = nil;		// not static, so we can examine it externally
SCDynamicStoreRef	sSCDSRef = NULL;
NSString			*sProxyUserIDAndPassword = nil;

NSString *CURLHandleCacheDeleteNotification = @"CURLHandleCacheDeleteNotification";
NSString *CURLHandleCacheChangeNotification = @"CURLHandleCacheChangeNotification";
NSString *CURLHandleCacheCreateNotification = @"CURLHandleCacheCreateNotification";
NSString *CURLHandleCreatedNotification		= @"CURLHandleCreatedNotification";

/*"	Callback from reading a chunk of data.  Since we pass "self" in as the "data pointer",
	we can use that to get back into Objective C and do the work with the class.
"*/

size_t curlBodyFunction(void *ptr, size_t size, size_t nmemb, void *inSelf)
{
	return [(CURLHandle *)inSelf curlWritePtr:ptr size:size number:nmemb message:BODY];
}

/*"	Callback from reading a chunk of data.  Since we pass "self" in as the "data pointer",
	we can use that to get back into Objective C and do the work with the class.
"*/

size_t curlHeaderFunction(void *ptr, size_t size, size_t nmemb, void *inSelf)
{
	return [(CURLHandle *)inSelf curlWritePtr:ptr size:size number:nmemb message:HEAD];
}

@implementation CURLHandle

/*"	CURLHandle is a wrapper around a CURL.
	This is in the public domain, but please report any improvements back to the author
	(dwood_karelia_com).
	Be sure to be familiar with CURL and how it works; see http://curl.haxx.se/

	The idea is to have it handle http and possibly other schemes too.  At this time
	we don't support writing data (via HTTP PUT) and special situations such as HTTPS and
	firewall proxies haven't been tried out yet.
	
	This class maintains only basic functionality, any "bells and whistles" should be
	defined in a category to keep this file as simple as possible.

	Each instance is created to be associated with a URL.  But we can change the URL and
	use the previous connection, as the CURL documentation says.

	A URL cache is maintained as described in the #NSURLHandle documentation.  It's just a
	mutable dictionary, which is apparently about as sophisticated as Apple does.  In the
	future, it might be nice to have some sort of LRU scheme....

	Notifications are posted when the cache changes; it's possible for the client to track this
	cache for debugging or other nefarious purposes.
	
	%{#Note: Comments in methods with this formatting indicate quotes from the headers and
	documentation for #NSURLHandle and are provided to help prove "correctness."  Some
	come from an another document -- perhaps an earlier version of the documentation or release notes,
	but I can't find the original source. These are marked "(?source)"}

	The comment "#{DO NOT INVOKE SUPERCLASS}" indicates that #NSURLHandle does not provide
	an implementation available for overriding.

"*/

// -----------------------------------------------------------------------------
#pragma mark ----- ADDITIONAL CURLHANDLE INTERFACES
// -----------------------------------------------------------------------------
/*" You must call #curlGoodbye at end of program, for example in #{[NSApplication applicationWillTerminate:]}.
"*/

+ (void) curlGoodbye
{
	curl_global_cleanup();
	[[NSNotificationCenter defaultCenter] postNotificationName:CURLHandleCacheDeleteNotification object:self];
	[sCurlCache release];
	[sAcceptedURLs release];
}

/*" Add an individual URL to the set of URLs that CURLHandle will accept.  This is useful when you want
	to accept only certain URLs but not others.  If you want to have CURLHandle handle all HTTPs (which
	seems to work just fine), just invoke #{curlHelloSignature:acceptAll:} with YES instead of registering individual URLs.
"*/

+ (void)curlAcceptURL:(NSURL *)url
{
    [sAcceptedURLs addObject:url];
}

/*" Initialize CURLHandle and the underlying CURL.  This can be invoked when the program is launched or before any loading is needed.
	Parameter is YES if %all http URLs should be handled by this; NO if only ones registered with #curlAcceptURL:

	%{Now all that remains is to inform NSURLHandle of your new subclass; you do this by sending the NSURLHandle class the registerURLHandleClass: message, passing your subclass as the argument. Once this message has been sent, as NSURLHandle is asked to create handles for a given URL, it will in turn ask your subclass if it wants to handle the URL. If your subclass responds YES, NSURLHandle will instantiate your subclass for the URL. (?source)}
"*/

+ (void) curlHelloSignature:(NSString *) inSignature acceptAll:(BOOL)inAcceptAllHTTP 
{
	CURLcode rc;
	sAcceptAllHTTP = inAcceptAllHTTP;
	sCurlCache = [[NSMutableDictionary alloc] init];		// set up static cache
	[[NSNotificationCenter defaultCenter] postNotificationName:CURLHandleCacheCreateNotification object:self];
	sAcceptedURLs = [[NSMutableSet alloc] init];			// set up static list of URLs this handles
	[NSURLHandle registerURLHandleClass:self];
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (0 != rc)
	{
		NSLog(@"Didn't curl_global_init, result = %d",rc);
	}
	
	// Now initialize System Config.
	sSCDSRef = SCDynamicStoreCreate(NULL,(CFStringRef)inSignature,NULL, NULL);
	if ( sSCDSRef == NULL )
	{
		NSLog(@"Didn't get SCDynamicStoreRef");
	}
}

/*"	Set a proxy user id and password, used by all CURLHandle. This should be done before any transfers are made."*/

+ (void) setProxyUserIDAndPassword:(NSString *)inString
{
	[inString retain];
	[sProxyUserIDAndPassword release];
	sProxyUserIDAndPassword = inString;
}

/*"	Set whether proxies are allowed or not.  Default value is YES.  If no, the proxy settings
	are ignored.
"*/
+ (void) setAllowsProxy:(BOOL) inBool
{
	sAllowsProxy = inBool;
}


/*"	Flush the entire cache of URLs.  There doesn't seem to be an NSURLHandle API to do this, so we provide our own.
"*/

+ (void)curlFlushEntireCache
{
	[sCurlCache removeAllObjects];
	[[NSNotificationCenter defaultCenter] postNotificationName:CURLHandleCacheChangeNotification object:sCurlCache];
}

/*"	Return the CURL object assocated with this, so categories can have other methods
	that do curl-specific stuff like #curl_easy_getinfo
"*/

- (CURL *) curl
{
	return mCURL;
}

/*"	Set the URL related to this CURLHandle.  This can actually be changed so the same CURL is used
	for different URLs, though they must be done sequentially.  (See libcurl documentation.)
	Note that doing so will confuse the cache, since cache is still keyed by original URL.
"*/

- (void) setURL:(NSURL *)inURL
{
	[inURL retain];
	[mNSURL release];
	mNSURL = inURL;
}

/*"	return the NSURL associated with this CURLHandle
"*/
- (NSURL *)url
{
	return mNSURL;
}

/*"	Set an option given a !{CURLoption} key.  Before transfer, the string will be used to invoke %curl_easy_setopt.  Categories with convenient APIs can make use of this.
"*/

- (void) setString:(NSString *)inString forKey:(CURLoption) inCurlOption
{
	[mStringOptions setObject:inString forKey:[NSNumber numberWithInt:inCurlOption]];
}

/*"	Set an option given a !{CURLoption} key.  Before transfer, the object, which  must be an NSString or an integer NSNumber will be used to invoke %curl_easy_setopt.  Categories with convenient APIs can make use of this.
"*/

- (void) setStringOrNumberObject:(id)inObject forKey:(CURLoption) inCurlOption
{
	[mStringOptions setObject:inObject forKey:[NSNumber numberWithInt:inCurlOption]];
}

/*"	Add these to the list of HTTP headers (besides cookie, user agent, referer -- see CURLOPT_HTTPHEADER).
"*/

- (void) setHTTPHeaders:(NSDictionary *)inDict
{
	if (nil == mHTTPHeaders)
	{
		mHTTPHeaders = [[NSMutableDictionary alloc] init];
	}
	[mHTTPHeaders addEntriesFromDictionary:inDict];
}

/*"	Set the file to be PUT
"*/
- (void) setPutFile:(NSString *)path
{
	mPutFile = fopen([path fileSystemRepresentation], "r");
	if (NULL == mPutFile) {
		NSLog(@"CURLHandle: setPutFile couldn't find file at %@", path);
		return;
	}
	fseek(mPutFile, 0, SEEK_END);
	curl_easy_setopt([self curl], CURLOPT_PUT, 1L);
	curl_easy_setopt([self curl], CURLOPT_UPLOAD, 1L);
	curl_easy_setopt([self curl], CURLOPT_INFILE, mPutFile);
	curl_easy_setopt([self curl], CURLOPT_INFILESIZE, ftell(mPutFile));
	rewind(mPutFile);
}

/*"	Set the file offset for performing the PUT.
"*/

- (void) setPutFileOffset:(int)offset
{
	if (NULL != mPutFile) {
		fseek(mPutFile, offset, SEEK_SET);
	}
}

- (void) setPutFile:(NSString *)path resumeUploadFromOffset:(off_t)offset_ {
	mPutFile = fopen([path fileSystemRepresentation], "r");
	if (NULL == mPutFile) {
		NSLog(@"CURLHandle: setPutFile:resumeUploadFromOffset: couldn't find file at %@", path);
		return;
	}
	
	fseek(mPutFile, 0, SEEK_END);
	off_t fileSize = ftello(mPutFile);
	rewind(mPutFile);
	
	curl_easy_setopt([self curl], CURLOPT_PUT, 1L);
	curl_easy_setopt([self curl], CURLOPT_UPLOAD, 1L);
	curl_easy_setopt([self curl], CURLOPT_INFILE, mPutFile);
	curl_easy_setopt([self curl], CURLOPT_INFILESIZE_LARGE, fileSize);
	curl_easy_setopt([self curl], CURLOPT_RESUME_FROM_LARGE, offset_);
}

/*"	Return the cookie array from the latest request.  Equivalent to getting a property of COOKIES.
"*/
- (NSArray *)getResponseCookies
{
	NSArray *result = [[self headerString] headersMatchingKey:@"set-cookie"];
	return result;
}

+ (NSString *) curlVersion
{
	return [NSString stringWithCString: curl_version()];
}


// -----------------------------------------------------------------------------
#pragma mark ----- NSURLHANDLE OVERRIDES
// -----------------------------------------------------------------------------

/*"	Make the CURLHandle go away.

	This will only be invoked after the background thread has completed, since the
	target of a thread detachment is retained.
"*/

- (void) dealloc
{
	// First, clear out the port's delegate, so it won't try to send message to deleted CURLHandle
	[mPort setDelegate:nil];
	// And remove the port from the runloop
	[[NSRunLoop currentRunLoop] removePort:mPort forMode:(NSString *)kCFRunLoopCommonModes];
    [mPort release];
	
	[mMainThread release];
	curl_easy_cleanup(mCURL);
	mCURL = nil;
	[mProgressIndicator release];
	[mNSURL release];
	[mHeaderBuffer release];			mHeaderBuffer = 0;
	[mHeaderString release];
	[mStringOptions release];
	[mProxies release];
	[mHTTPHeaders release];
	[super dealloc];
}

/*"	%{Returns whether an URL handle can be initialized with anURL. If anURL uses an unsupported scheme, this method returns NO. Subclasses of NSURLHandle must override this method. to identify which URLs they can service.}
	
	Success if either the "all HTTP" switch is on and the URL is an HTTP url,
	or if it's a member of the set of URLs accepted explicitly.

	#{DO NOT INVOKE SUPERCLASS}.
"*/

+ (BOOL)canInitWithURL:(NSURL *)anURL
{
	NSString *scheme = [[anURL scheme] lowercaseString];
	return (sAcceptAllHTTP &&
			( [scheme isEqualToString:@"http"] || [scheme isEqualToString:@"https"] || [scheme isEqualToString:@"ftp"]) )
		|| [sAcceptedURLs containsObject:anURL];
}

/*" %{Returns the URL handle from the cache that has serviced anURL or another identical URL. Subclasses of NSURLHandle must override this method. Returns nil if there is no such handle.}

	%{cachedHandleForURL: should look in the cache (maintained by your subclass) for an existing handle that services an URL identical to the one passed. If so, the cached handle should be returned. If not, a new handle should be created for the URL, stored in the cache, then returned. (?source)}
	We have super cache the handle as well, though it's only to cause the data not
	to be flushed.  Because the superclass is actually abstract (using the whole
	class cluster mechanism), it's not like we're caching the URL and so is the parent.
"*/

+ (NSURLHandle *)cachedHandleForURL:(NSURL *)anURL
{
	NSURLHandle *result = nil;
	result = [sCurlCache objectForKey:anURL];
	if (nil == result)
	{
		result =[[[self alloc] initWithURL:anURL cached:YES] autorelease];
	}
	else
	{
#ifdef DEBUGCURL
		NSLog(@"Found in cache: %@",anURL);
#endif
	}
	return result;
}

/*" %{initWithURL:cached: is the designated initializer for NSURLHandle; the second argument specifies whether the handle will be placed in the cache. (?source)}

	%{Initializes a newly created URL handle with the URL anURL. willCache controls whether the URL handle will cache its data and respond to requests from equivalent URLs for the cached data. Subclasses of NSURLHandle must override this method.}

	#{TODO: initWithURL ought to clean up better if init failed; release what was allocated.}
	Note that this will not actually look up a URL in the cache if you specify YES.
	If you want to get an existing URL from the cache, use #cachedHandleForURL:.
"*/

- (id) initWithURL:(NSURL *)anURL cached:(BOOL)willCache
{
#ifdef DEBUGCURL
	NSLog(@"...initWithURL: %@",anURL);
#endif
	if ((self = [super initWithURL:anURL cached:willCache]))
	{
		mMainThread = [[NSThread currentThread] retain];	// remember main thread

		mPort = [[NSPort port] retain];
		[mPort setDelegate:self];

// #if 1
// #warning # this may be leaking ... there are two retains going on here.  Apple bug report #2885852, still open after TWO YEARS!
		[[NSRunLoop currentRunLoop] addPort:mPort forMode:(NSString *)kCFRunLoopCommonModes];
// #else
// #warning # This attempt to compensate for the leak causes crashes....
// {
// 			int oldCount = [mPort retainCount];
// 			int newCount;
// 			[[NSRunLoop currentRunLoop] addPort:mPort forMode:(NSString *)kCFRunLoopCommonModes];
// 			newCount = [mPort retainCount];
// 
// 			if (newCount - oldCount > 1)
// 			{
// 				[mPort release];
// //#ifdef DEBUGCURL
// 				NSLog(@"Extra NSPort retain; released %@",mPort);
// //#endif
// 			}
// 			
// }
// #endif

		mCURL = curl_easy_init();
		if (nil == mCURL)
		{
			return nil;
		}
		[self setURL:anURL];
		
		// Store the URL
		if (willCache)
		{
			[sCurlCache setObject:self forKey:anURL];
			[[NSNotificationCenter defaultCenter] postNotificationName:CURLHandleCacheChangeNotification object:sCurlCache];
		}
		mErrorBuffer[0] = 0;	// initialize the error buffer to empty
		mHeaderBuffer = [[NSMutableData alloc] init];
		mHeaderString = nil;	// not using yet
		mStringOptions = [[NSMutableDictionary alloc] init];
				
		// SET OPTIONS -- NOTE THAT WE DON'T SET ANY STRINGS DIRECTLY AT THIS STAGE.
		// Put error messages here
		mResult = curl_easy_setopt(mCURL, CURLOPT_ERRORBUFFER, &mErrorBuffer);
			if(mResult) return nil;

		mResult = curl_easy_setopt(mCURL, CURLOPT_FOLLOWLOCATION, YES);
			if(mResult) return nil;
		mResult = curl_easy_setopt(mCURL, CURLOPT_FAILONERROR, YES);
			if(mResult) return nil;

		// send all data to the C function
		mResult = curl_easy_setopt(mCURL, CURLOPT_WRITEFUNCTION, curlBodyFunction);
			if(mResult) return nil;
		mResult = curl_easy_setopt(mCURL, CURLOPT_HEADERFUNCTION, curlHeaderFunction);
			if(mResult) return nil;
		// pass self to the callback
		mResult = curl_easy_setopt(mCURL, CURLOPT_WRITEHEADER, self);
			if(mResult) return nil;
		mResult = curl_easy_setopt(mCURL, CURLOPT_FILE, self);
			if(mResult) return nil;

			// Finally, post a notification that the CURLHandle was created.
			// This is so that handles created behind our back, like images in an HTML view,
			// can be given a client to pay attention to their being loaded.

		[[NSNotificationCenter defaultCenter] postNotificationName:CURLHandleCreatedNotification object:self];
	}
	return self;
}

/*" %{Returns the property for key propertyKey; returns nil if there is no such key. Subclasses of NSURLHandle must override this method.}
	
	#{DO NOT INVOKE SUPERCLASS}.
"*/

- (id)propertyForKey:(NSString *)propertyKey
{
	id result = [self propertyForKeyIfAvailable:propertyKey];
	if (nil == result)
	{
		// get some more expensive things...
	}
	return result;
}

/*" %{Returns the property for key propertyKey only if the value is already available, i.e., the client doesn't need to do any work.}

	#{DO NOT INVOKE SUPERCLASS}.
	
	#{TODO: We can't assume any encoding for header.  Perhaps we could look for the encoding value in the header, and try again if it doesn't match?}

	#{TODO: Apple defines some keys, but what the heck are they?  "Description Forthcoming"....}

This first attempts to handle the Apple-defined NSHTTPProperty... keys.  Then if it's HEADER we just return the whole header string.  If it's COOKIES, we return the cookie as an array; this can be further parsed with #parsedCookies:.

Otherwise, we try to get it by just getting a header with that property name (case-insensitive).
"*/

- (id)propertyForKeyIfAvailable:(NSString *)propertyKey
{
	id result = nil;
	long resultLong;
	if ([propertyKey hasPrefix:@"NSHTTPProperty"])
	{
		if ([propertyKey isEqualToString:NSHTTPPropertyStatusCodeKey])
		{
			mResult = curl_easy_getinfo(mCURL, CURLINFO_HTTP_CODE, &resultLong );
			result = [NSNumber numberWithLong:resultLong];
		}
		else if ([propertyKey isEqualToString:NSHTTPPropertyStatusReasonKey])
		{
			result = [[self headerString] headerStatus];
		}
		else if ([propertyKey isEqualToString:NSHTTPPropertyServerHTTPVersionKey])
		{
			result = [[self headerString] headerHTTPVersion];
		}
		else if ([propertyKey isEqualToString:NSHTTPPropertyRedirectionHeadersKey])
		{
			result = [[self headerString] headerMatchingKey:@"location"];
		}
		else if ([propertyKey isEqualToString:NSHTTPPropertyErrorPageDataKey])
		{
			result = @"NSHTTPPropertyErrorPageDataKey -- needs to be the body information sent.";
		}
	}
	else if ([propertyKey isEqualToString:@"HEADER"])
	{
		result = [self headerString];
	}
	else if ([propertyKey isEqualToString:@"COOKIES"])
	{
		result = [self getResponseCookies];
	}
	else	// Now see if we can find any headers loaded with that property as the "title"
			// and if we do, get the value from the first match (unlikely to be more than 
			// one match if the client is using this mechanism to get a specific header value.
	{
		result = [[self headerString] headerMatchingKey:[propertyKey lowercaseString]];
	}
	return result;
}

/*" %{The last three methods, loadInForeground, beginLoadInBackground, and endLoadInBackground do the meaty work of your subclass. They are called from resourceData, loadInBackground, and cancelLoadInBackground respectively, after checking the status of the handle. (For instance, resourceData will not call loadInForeground if the handle has already been loaded; it will simply return the existing data.) (?source)}

	%{Loads the receiver's data in the synchronously. Called by resourceData. Subclasses of NSURLHandle must override this method.}

	#{DO NOT INVOKE SUPERCLASS}.
"*/

- (NSData *)loadInForeground
{
	BOOL wasIndeterminate = NO;
	NSData *result = nil;
	mAbortBackground = NO;

	if (nil != mProgressIndicator)	// make progress indicator be indeterminate
	{
		wasIndeterminate = [mProgressIndicator isIndeterminate];
		[mProgressIndicator setIndeterminate:YES];
		[mProgressIndicator display];
	}
	[self prepareAndPerformCurl];
	
	// HACK: in some circumstances, the retain count of self->_data is OK, but we crash in [super resourceData]
	// if we don't bump up the retain count.  So I'm going to try to autorelease this to make sure
	// it isn't nuked.
	[[(self->_data) retain] autorelease];

	if (nil != mProgressIndicator)	// restore progress indicator to previous state
	{
		[mProgressIndicator setIndeterminate:wasIndeterminate];
	}

	if (0 == mResult)
	{
		result = [self availableResourceData];		// now there should be data
	}
	return result;
}

/*" %{beginLoadInBackground should start a background load of the data, then return. (?source)}
	%{Called from -loadInBackground, above.}
	%{Called when a background load begins. This method is provided mainly for subclasses that wish to take advantage of the superclass' failure reporting mechanism.}

	#{DO NOT INVOKE SUPERCLASS}.

	We just set a couple of status flags and then detach the background thread at this point,
	as long as it's not already happening.
"*/

- (void)beginLoadInBackground
{
	if (NSURLHandleLoadInProgress != [self status])
	{
		mAbortBackground = NO;
		[NSThread detachNewThreadSelector:@selector(curlThreadBackgroundLoad:)
			toTarget:self
			withObject:nil];
	}
}

/*" %{Called to cancel a load currently in progress. You should call super's implementation at the end of your implementation of this method.}

	%{Finally, your subclass should override cancelLoadInBackground to stop a background load in progress. Once a handle has received a #cancelLoadInBackground message, it must not send any further #didLoadBytes:loadComplete: or #backgroundLoadDidFailWithReason: messages. (?source)}
	This just sets a flag so that the next time the background thread is about to send a message,
	it will not.  However, all current operations will still execute.  But we just won't do anything
	with the results.
"*/

- (void)cancelLoadInBackground
{
	mAbortBackground = YES;
	[super cancelLoadInBackground];
}

/*" %{Called by cancelLoadInBackground to halt any background loading. You should call super's implementation at the end of your implementation of this method.}
	#{DO NOT INVOKE SUPERCLASS}
"*/

- (void)endLoadInBackground
{
	// I don't think there's anything I need to do at this point
}


// -----------------------------------------------------------------------------
#pragma mark ----- CURL DATA LOADING SUPPORT
// -----------------------------------------------------------------------------

/*"	Actually set up for loading and do the perform.  This happens in either
	the foreground or background thread.  Before doing the perform, we collect up
	all the saved-up string-valued options, and set them right before the perform.
	This is because we create temporary (autoreleased) c-strings.
"*/

- (void) prepareAndPerformCurl
{
	struct curl_slist *httpHeaders = nil;
	// Set the options
	NSEnumerator *theEnum = [mStringOptions keyEnumerator];
	NSString *theKey;
	while (nil != (theKey = [theEnum nextObject]) )
	{
		id theObject = [mStringOptions objectForKey:theKey];

		if ([theObject isKindOfClass:[NSNumber class]])
		{
			mResult = curl_easy_setopt(mCURL, (CURLoption)[theKey intValue], [theObject intValue]);
		}
		else if ([theObject respondsToSelector:@selector(cString)])
		{
			mResult = curl_easy_setopt(mCURL, (CURLoption)[theKey intValue], [theObject cString]);
		}
		else
		{
			NSLog(@"Ignoring CURL option of type %@ for key %@", [theObject class], theKey);
			mResult = 0;	// ignore the option, so don't have an error.
		}
		if (0 != mResult)
		{
			return;
		}
	}

	// Set the proxy info.  Ignore errors -- just don't do proxy if errors.
	if (sAllowsProxy)	// normally this is YES.
	{
		NSString *proxyHost = nil;
		NSNumber *proxyPort = nil;
		NSString *scheme = [[mNSURL scheme] lowercaseString];

		// Allocate and keep the proxy dictionary
		if (nil == mProxies)
		{
			mProxies = (NSDictionary *) SCDynamicStoreCopyProxies(sSCDSRef);
		}


		if (mProxies
			&& [scheme isEqualToString:@"http"]
			&& [[mProxies objectForKey:NSS(kSCPropNetProxiesHTTPEnable)] boolValue] )
		{
			proxyHost = (NSString *) [mProxies objectForKey:NSS(kSCPropNetProxiesHTTPProxy)];
			proxyPort = (NSNumber *)[mProxies objectForKey:NSS(kSCPropNetProxiesHTTPPort)];
		}
		if (mProxies
			&& [scheme isEqualToString:@"https"]
			&& [[mProxies objectForKey:NSS(kSCPropNetProxiesHTTPSEnable)] boolValue] )
		{
			proxyHost = (NSString *) [mProxies objectForKey:NSS(kSCPropNetProxiesHTTPSProxy)];
			proxyPort = (NSNumber *)[mProxies objectForKey:NSS(kSCPropNetProxiesHTTPSPort)];
		}

		if (mProxies
			&& [scheme isEqualToString:@"ftp"]
			&& [[mProxies objectForKey:NSS(kSCPropNetProxiesFTPEnable)] boolValue] )
		{
			proxyHost = (NSString *) [mProxies objectForKey:NSS(kSCPropNetProxiesFTPProxy)];
			proxyPort = (NSNumber *)[mProxies objectForKey:NSS(kSCPropNetProxiesFTPPort)];
		}
		
		if (proxyHost && proxyPort)
		{
			mResult = curl_easy_setopt(mCURL, CURLOPT_PROXY, [proxyHost cString]);
			mResult = curl_easy_setopt(mCURL, CURLOPT_PROXYPORT, [proxyPort longValue]);

			// Now, provide a user/password if one is globally set.
			if (nil != sProxyUserIDAndPassword)
			{
				mResult = curl_easy_setopt(mCURL, CURLOPT_PROXYUSERPWD, [sProxyUserIDAndPassword cString] );
			}
		}
	}
	
	// Set the HTTP Headers.  (These will override options set with above)
	{
		NSEnumerator *theEnum = [mHTTPHeaders keyEnumerator];
		id theKey;
		while (nil != (theKey = [theEnum nextObject]) )
		{
			id theValue = [mHTTPHeaders objectForKey:theKey];
			NSString *pair = [NSString stringWithFormat:@"%@: %@",theKey,theValue];
			httpHeaders = curl_slist_append( httpHeaders, [pair cString] );
		}
		curl_easy_setopt(mCURL, CURLOPT_HTTPHEADER, httpHeaders);
	}

	// Set the URL
	mResult = curl_easy_setopt(mCURL, CURLOPT_URL, [[mNSURL absoluteString] cString]);
	if (0 != mResult)
	{
		return;
	}
	// clear the buffers
	[mHeaderBuffer setLength:0];	// empty out header buffer
	[mHeaderString release];		// release and invalidate any cached string of header
	mHeaderString = nil;
	
	// Do the transfer
	mResult = curl_easy_perform(mCURL);

	if (nil != mPutFile)
	{
		fclose(mPutFile);
		mPutFile = nil;
	}

	if (nil != httpHeaders)
	{
		curl_slist_free_all(httpHeaders);
	}
}

/*"	Method executed in new background thread to load the URL.  It sets up an
	autorelease pool, does the load (which has callbacks), and then sends a
	port message to indicate that the load is done.
	The CURLHandle is retained for the duration of this thread, so it won't be deallocated
	until the thread is done executing.
"*/

- (void) curlThreadBackgroundLoad:(id)notUsed
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[self prepareAndPerformCurl];

#ifdef DEBUGCURL_SLOW
[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];	// wait before finish message
#endif

	// Send message that we are done or had an error, if we weren't aborting.
	// (Aborting should not let backgroundLoadDidFailWithReason get called)
	if (!mAbortBackground)
	{
		BOOL sent = NO;
		NSPortMessage *message
			= [[NSPortMessage alloc] initWithSendPort:mPort
				receivePort:mPort components:nil];
		[message setMsgid:(0 == mResult) ? DONE : BAD ];
		sent = [message sendBeforeDate:[NSDate dateWithTimeIntervalSinceNow:60.0]];
		if (!sent)
		{
			NSLog(@"CURLHandle couldn't send DONE message" );
		}
		[message release];
	}

	[pool release];
}

/*"	Continue the writing callback in Objective C; now we have our instance variables.
"*/

- (size_t) curlWritePtr:(void *)inPtr size:(size_t)inSize number:(size_t)inNumber message:(int)inMessageID
{
	size_t written = inSize*inNumber;
	NSData *data = [NSData dataWithBytes:inPtr length:written];

	if (mAbortBackground)
	{
		written = -1;		// signify to Curl that we are stopping
							// Do NOT send message; see "cancelLoadInBackground" comments
	}
	else if ([NSThread currentThread] != mMainThread)	// in background if in different thread
	{
		BOOL sent = NO;
		NSArray *dataArray		= [NSArray arrayWithObject:data];
		NSPortMessage *message	= [[NSPortMessage alloc] initWithSendPort:mPort
			receivePort:mPort components:dataArray];
		[message setMsgid:inMessageID];
		sent = [message sendBeforeDate:[NSDate dateWithTimeIntervalSinceNow:60.0]];
		if (!sent)
		{
			NSLog(@"CURLHandle couldn't send message, data length = %d", [data length] );
		}
		[message release];
	}
	else	// Foreground, just write the bytes
	{
		if (nil != mProgressIndicator)
		{
			[mProgressIndicator animate:nil];
			[mProgressIndicator display];
		}

		if (HEAD == inMessageID)
		{
			[mHeaderBuffer appendData:data];
		}
		else if (BODY == inMessageID)	// notify superclass of new bytes
		{
			[self didLoadBytes:data loadComplete:NO];
		}
	}
	return written;
}

/*" NSPortDelegate method gets called in the foreground thread.  Now we're ready to call our
	data-processor, which is called from both head and body.
"*/

- (void)handlePortMessage:(NSPortMessage *)portMessage
{
    int message = [portMessage msgid];
	NSArray *components	= [portMessage components];

	if (!mAbortBackground)	// only process if we haven't aborted
	{
		switch (message)
		{
			case DONE:
	#ifdef DEBUGCURL
				NSLog(@"+++++DONE load: %@",mNSURL);
	#endif
				[self didLoadBytes:nil loadComplete:YES];
				break;
			case BAD:
				// "As a background load progresses, subclasses should call these methods"
				// "Sends the failure message to clients"
				[self backgroundLoadDidFailWithReason:[self curlError]];
				break;
			case BODY:
				if (nil != components)
				{
					[self didLoadBytes:[components objectAtIndex:0] loadComplete:NO];	// notify foreground loading
				}
				break;
			case HEAD:
				if (nil != components)
				{
					[mHeaderBuffer appendData:[components objectAtIndex:0]];
				}
				break;
		}
	}
}

/*"	Convert curl's error buffer into an NSString if possible, or return the result code number as a string.  This is pass into #backgroundLoadDidFailWithReason to set the failureReason string.
"*/

- (NSString *)curlError
{
	NSString *result = [NSString stringWithCString:mErrorBuffer];
	if (nil == result)
	{
		result = [NSString stringWithFormat:@"(curl result code # %d)", mResult];
	}
	return result;
}

/*" Return the current header, as a string. This is meant to be invoked after all
the headers are read; the entire header is cached into a string after converting from raw data. "*/

- (NSString *)headerString
{
#warning We can't really assume a header encoding, trying 7-bit ASCII only.  Maybe there is some way to know?

	if (nil == mHeaderString)		// Has it not been initialized yet?
	{
		mHeaderString = [[NSString alloc] initWithData:mHeaderBuffer encoding:NSASCIIStringEncoding];
	}
	return mHeaderString;
}



@end

// -----------------------------------------------------------------------------
#pragma mark ----- CATEGORIES
// -----------------------------------------------------------------------------

@implementation NSDictionary ( CurlHTTPExtensions )

/*"	This category adds methods for dealing with HTTP input and output to an #NSDictionary.
"*/

/*"	Convert a dictionary to an HTTP-formatted string with 7-bit ASCII encoding;
	see #formatForHTTPUsingEncoding.
"*/

- (NSString *) formatForHTTP
{
	return [self formatForHTTPUsingEncoding:NSASCIIStringEncoding];
		// default to dumb ASCII only
}

/*"	Convert a dictionary to an HTTP-formatted string with the given encoding.
	Spaces are turned into !{+}; other special characters are escaped with !{%};
	keys and values are output as %{key}=%{value}; in between arguments is !{&}.
"*/

- (NSString *) formatForHTTPUsingEncoding:(NSStringEncoding)inEncoding
{
	return [self formatForHTTPUsingEncoding:inEncoding ordering:nil];
}

/*"	Convert a dictionary to an HTTP-formatted string with the given encoding, as above.  The inOrdering parameter specifies the order to place the inputs, for servers that care about this.  (Note that keys in the dictionary that aren't in inOrdering will not be included.)  If inOrdering is nil, all keys and values will be output in an unspecified order.
"*/

- (NSString *) formatForHTTPUsingEncoding:(NSStringEncoding)inEncoding ordering:(NSArray *)inOrdering
{
	NSMutableString *s = [NSMutableString stringWithCapacity:256];
	NSEnumerator *e = (nil == inOrdering) ? [self keyEnumerator] : [inOrdering objectEnumerator];
	id key;
	CFStringEncoding cfStrEnc = CFStringConvertNSStringEncodingToEncoding(inEncoding);

	while ((key = [e nextObject]))
	{
        id keyObject = [self objectForKey: key];
		// conform with rfc 1738 3.3, also escape URL-like characters that might be in the parameters
		NSString *escapedKey
		= (NSString *) CFURLCreateStringByAddingPercentEscapes(
														 NULL, (CFStringRef) key, NULL, (CFStringRef) @";:@&=/+", cfStrEnc);
        if ([keyObject respondsToSelector: @selector(objectEnumerator)])
        {
            NSEnumerator	*multipleValueEnum = [keyObject objectEnumerator];
            id				aValue;

            while ((aValue = [multipleValueEnum nextObject]))
            {
                NSString *escapedObject
                = (NSString *) CFURLCreateStringByAddingPercentEscapes(
                                                                       NULL, (CFStringRef) [aValue description], NULL, (CFStringRef) @";:@&=/+", cfStrEnc);
                [s appendFormat:@"%@=%@&", escapedKey, escapedObject];
            }
        }
        else
        {
            NSString *escapedObject
            = (NSString *) CFURLCreateStringByAddingPercentEscapes(
                                                                   NULL, (CFStringRef) [keyObject description], NULL, (CFStringRef) @";:@&=/+", cfStrEnc);
            [s appendFormat:@"%@=%@&", escapedKey, escapedObject];
        }
	}
	// Delete final & from the string
	if (![s isEqualToString:@""])
	{
		[s deleteCharactersInRange:NSMakeRange([s length]-1, 1)];
	}
	return s;	
}

@end

@implementation NSArray ( CurlHTTPExtensions )

/*"	This category on NSArray adds methods for header and cookie access.
"*/


/*"	Parse the array of cookie lines, turning it into a dictionary of key/values
(suitable for passing to #setRequestCookies:).

For instance, this will turn

( "B=96lr6csucjt99&b=2; expires=Thu, 15 Apr 2010 20:00:00 GMT; path=/; domain=.yahoo.com"; )

into

{
	B = {
		value = "96lr6csucjt99&b=2";
		expires = "Thu, 15 Apr 2010 20:00:00 GMT";
		path = "/";
		domain = ".yahoo.com";
	}
}

"*/
- (NSDictionary *)parsedCookies
{
	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	NSEnumerator *theEnum = [self objectEnumerator];
	NSString *cookieLine;

	while (nil != (cookieLine = [theEnum nextObject]) )
	{
		NSArray *components = [cookieLine componentsSeparatedByString:@"; "];
		NSEnumerator *theEnum = [components objectEnumerator];
		NSString *thePair;
		BOOL firstOne = YES;		// the first one is handled specially
		NSMutableDictionary *attributesDictionary = [NSMutableDictionary dictionary];

		while (nil != (thePair = [theEnum nextObject]) )
		{
			NSRange whereEquals = [thePair rangeOfString:@"="];
			if (NSNotFound != whereEquals.location)
			{
				NSString *key = [thePair substringToIndex:whereEquals.location];
				NSString *val = [thePair substringFromIndex:whereEquals.location + 1];
				if (firstOne)
				{
					[result setObject:attributesDictionary forKey:key];
					[attributesDictionary setObject:val forKey:@"value"];
					firstOne = NO;
				}
				else
				{
					[attributesDictionary setObject:val forKey:key];
				}
			}
		}
	}
	return result;
}

@end

@implementation NSString ( CurlHeaderExtensions )

- (NSString *) headerStatus
{
	// Get the first line of the headers
	NSArray *components = [self componentsSeparatedByLineSeparators];
	NSString *theFirstLine = [components objectAtIndex:0];
	// Pull out from the second "word"
	NSArray *theLineComponents = [theFirstLine componentsSeparatedByString: @" "];
	NSRange theRange = NSMakeRange(2, [theLineComponents count] - 2);
	NSString *theResult = [[theLineComponents subarrayWithRange: theRange] componentsJoinedByString: @" "];
	return theResult;
}

- (NSString *) headerHTTPVersion
{
	NSString *result = nil;
	// Get the first "word" of the first line of the headers
	NSRange whereSpace = [self rangeOfString:@" "];
	if (NSNotFound != whereSpace.location)
	{
		result = [self substringToIndex:whereSpace.location];
	}
	return result;
}

/*"	Create an array of values from the HTTP headers string that match the given header key.
"*/

- (NSArray *) headersMatchingKey:(NSString *)inKey
{
	NSMutableArray *result = [NSMutableArray array];
	NSArray *components = [self componentsSeparatedByLineSeparators];
	NSEnumerator *theEnum = [components objectEnumerator];
	NSString *theLine = [theEnum nextObject];		// result code -- ignore
	while (nil != (theLine = [theEnum nextObject]) )
	{
		if ([[theLine headerKey] isEqualToString:inKey])
		{
			// Add it to the resulting array
			[result addObject:[theLine headerValue]];
		}
	}
	return result;
}


/*" Return a the single (first) value of a header.  Returns NULL if not found. "*/

- (NSString *)headerMatchingKey:(NSString *)inKey
{
	NSString *result = nil;
	NSArray *headerArray = [self headersMatchingKey:inKey];
	if ([headerArray count] > 0)
	{
		result = [headerArray objectAtIndex:0];
	}
	return result;
}


/*"	Create an array of dictionaries from the HTTP headers. 
(lowercasing header titles for ease of comparison)

The array consists of any number of
single-item dictionaries, with each dictionary consisting of a single key
corresponding to the header title (e.g. "set-cookie") and value corresponding to
the rest of the data on line (following ":")

"*/

- (NSArray *) allHTTPHeaderDicts
{
	NSMutableArray *result = [NSMutableArray array];
	NSArray *components = [self componentsSeparatedByLineSeparators];
	
	NSEnumerator *theEnum = [components objectEnumerator];
	NSString *theLine = [theEnum nextObject];		// result code -- ignore
	while (nil != (theLine = [theEnum nextObject]) )
	{
		NSString *key = [theLine headerKey];
		NSString *value = [theLine headerValue];
		if (nil != key && nil != value)
		{
			// Add a single dictionary for this header name/value
			[result addObject:[NSDictionary dictionaryWithObject:value forKey:key]];
		}
	}
	return result;
}

/*" Given a line of a header, e.g. "Foo: Bar", return the key in lowercase form, e.g. "foo". "*/

- (NSString *) headerKey
{
	NSString *result = nil;
	NSRange whereColon = [self rangeOfString:@": "];
	if (NSNotFound != whereColon.location)
	{
		result = [[self substringToIndex:whereColon.location] lowercaseString];
	}
	return result;
}

/*" Given a line of a header, e.g. "Foo: Bar", return the value in lowercase form, e.g. "bar". "*/

- (NSString *) headerValue
{
	NSString *result = nil;
	NSRange whereColon = [self rangeOfString:@": "];
	if (NSNotFound != whereColon.location)
	{
		result = [self substringFromIndex:whereColon.location + 2];
	}
	return result;
}


/*"	Split a string into lines separated by any of the various newline characters.  Equivalent to componentsSeparatedByString:@"\n" but it works with the different line separators: \r, \n, \r\n, 0x2028, 0x2029 "*/

- (NSArray *) componentsSeparatedByLineSeparators
{
	NSMutableArray *result	= [NSMutableArray array];
	NSRange range = NSMakeRange(0,0);
	unsigned start, end;
	unsigned contentsEnd = 0;
	
	while (contentsEnd < [self length])
	{
		[self getLineStart:&start end:&end contentsEnd:&contentsEnd forRange:range];
		[result addObject:[self substringWithRange:NSMakeRange(start,contentsEnd-start)]];
		range.location = end;
		range.length = 0;
	}
	return result;
}
@end

