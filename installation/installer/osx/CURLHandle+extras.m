//
// CURLHandle+extras.m
//
// Created by Dan Wood <dwood@karelia.com> on Mon Oct 01 2001.
// This is in the public domain, but please report any improvements back to the author.
//
//	The current version of CURLHandle is 1.9.
//

#import "CURLHandle+extras.h"

@implementation CURLHandle ( extras )

/*"	This category adds useful methods to #CURLHandle for setting options and getting results.

	The descriptions are lifted from the current version of libcurl's documentation
	and modified where necessary.
"*/

// -----------------------------------------------------------------------------
#pragma mark -- OTHER GOODIES
// -----------------------------------------------------------------------------

/*"	Set a progress indicator for showing foreground load progress.
"*/
- (void) setProgressIndicator:(id)inProgressIndicator
{
	[inProgressIndicator retain];
	[mProgressIndicator release];
	mProgressIndicator = inProgressIndicator;
}

// -----------------------------------------------------------------------------
#pragma mark -- CURL OPTION SETTERS
// -----------------------------------------------------------------------------

/*"	Set the if-modified-since timestamp.

Pass an NSDate as parameter. It will be used to set the If-Modified-Since: header
in the http request sent to the remote server. This can be used to get a 304 result
if the remote file isn't newer than inModDate.

"*/

- (void) setIfModSince:(NSDate *)inModDate
{
	curl_easy_setopt([self curl], CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
	curl_easy_setopt([self curl], CURLOPT_TIMEVALUE, [inModDate timeIntervalSince1970]);
}


/*"	Set the byte range.

Pass a string as parameter. It will be used to set the Range: header in
the http request sent to the remote server. This is used to get partial
downloads. String in the form of X-Y or X-Y,N-M

"*/
- (void) setRange:(NSString *)inRange
{
	[self setString:inRange forKey:CURLOPT_RANGE];
}

/*"	Set the http operation to HEAD instead of GET.

Pass a BOOL as parameter. It will be used to set the verb to HEAD in the
http request sent to the remote server. This means that no body will be returned,
just the mime header.

"*/

- (void) setNoBody:(BOOL)inNoBody
{
	curl_easy_setopt([self curl], CURLOPT_NOBODY, (long)inNoBody);
}


/*"	Set the connection timeout.

	Pass a long. It should contain the maximum time in
	seconds that you allow the connection to the server
	to take. This only limits the connection phase,
	once it has connected, this option is of no more
	use. Set to zero to disable connection timeout (it
	will then only timeout on the system's internal
	timeouts).
			 
	According to man 3 curl_easy_setopt, CURLOPT_CONNECTTIMEOUT uses signals and thus isn't thread-safe. However, in the same man page it's stated that if you TURN OFF SIGNALLING, you can still use CURLOPT_CONNECTTIMEOUT! This will DISABLE any features that use signals, so beware! (But turning off the connection timeout by setting to zero will turn it back on.)

"*/

- (void) setConnectionTimeout:(long) inSeconds	// only applies to foreground thread!
{
	curl_easy_setopt([self curl], CURLOPT_NOSIGNAL, inSeconds != 0);
	curl_easy_setopt([self curl], CURLOPT_CONNECTTIMEOUT, inSeconds);
}

/*" Set the transfer timeout.

Pass a long. It should contain the maximum time in seconds that you allow the transfer from the server to take.

According to man 3 curl_easy_setopt, CURLOPT_TIMEOUT uses signals and thus isn't thread-safe. However, in the same man page it's stated that if you TURN OFF SIGNALLING, you can still use CURLOPT_TIMEOUT! This will DISABLE any features that use signals, so beware! (But turning off the connection timeout by setting to zero will turn it back on.)

"*/
- (void) setTransferTimeout:(long) inSeconds
{
	curl_easy_setopt([self curl], CURLOPT_NOSIGNAL, inSeconds != 0);
	curl_easy_setopt([self curl], CURLOPT_TIMEOUT, inSeconds);
}


/*"	Set the user agent string.

	Pass a string as parameter. It will be used to set the User-Agent: header
	in the http request sent to the remote server. This
	can be used to fool servers or scripts.

"*/

- (void) setUserAgent:(NSString *)inUserAgent
{
	[self setString:inUserAgent forKey:CURLOPT_USERAGENT]; 
}

/*"	Set the referer [sic] string.

	Pass a string as parameter. It will be used to set the Referer: header in
	the http request sent to the remote server. This can
	be used to fool servers or scripts.

"*/

- (void) setReferer:(NSString *)inReferer
{
	[self setString:inReferer forKey:CURLOPT_REFERER]; 
}

/*"	Set whether redirects are followed automatically. (Default to true)

	A YES parameter tells the library to follow any
	Location: header that the server sends as part of a
	HTTP header.
			 
	NOTE: this means that the library will re-send the
	same request on the new location and follow new
	Location: headers all the way until no more such
	headers are returned.

"*/

- (void) setFollowsRedirects:(BOOL)inFlag		// default to true
{
	mResult = curl_easy_setopt([self curl], CURLOPT_FOLLOWLOCATION, inFlag);
}

/*"	Set whether failure codes should return an error.

	A YES parameter tells the library to fail
	silently if the HTTP code returned is equal to or
	larger than 300. The default action would be to
	return the page normally, ignoring that code.
"*/

- (void) setFailsOnError:(BOOL)inFlag		// default to true.
{
	mResult = curl_easy_setopt([self curl], CURLOPT_FAILONERROR, inFlag);
}

/*"	Set the user and password.

	Pass strings to use for the connection.
"*/

- (void) setUserName:(NSString*)inUserName password:(NSString *)inPassword	// default to none
{
	if (nil != inUserName && ![inUserName isEqualToString:@""])
	{
		[self setString:[NSString stringWithFormat:@"%@:%@",inUserName,inPassword] forKey:CURLOPT_USERPWD];
	}
}

/*"	Set the cookie repository file.

	Pass a string as parameter. It should contain the name of a file for holding 
	cookie data. The cookie data may be in Netscape/ Mozilla cookie data format 
	or just regular HTTP-style headers dumped to a file.

	Use this method to avoid manual cookie manipulation with setRequestCookies & getResponseCookies
"*/

- (void) setCookieFile:(NSString *)inFilePath
{
	if (![inFilePath isEqualToString:@""])
	{
		[self setString:inFilePath forKey:CURLOPT_COOKIEFILE];
	}
}

/*"	Set the HTTP request's cookie data manually.
	The Dictionary contains any number of entries where the key is the cookie name, as
	identified in an incoming response header "Set-Cookie:" line, and the value is either the
	cookie value as a string (the simple case) or an NSDictionary with the value of the cookie
	stored under the "value" key; any other key/values representing other cookie attributes
	will be ignored.
"*/

- (void) setRequestCookies:(NSDictionary *)inDict
{
	if (0 != [inDict count])
	{
		NSMutableString *buf = [NSMutableString string];
		NSEnumerator *theEnum = [inDict keyEnumerator];
		id key;
	
		while (nil != (key = [theEnum nextObject]) )
		{
			id value = [inDict objectForKey:key];
			if ([value respondsToSelector:@selector(objectForKey:)])
			{
				// This is actually a dictionary, delve down to get the "value" key
				value = [value objectForKey:@"value"];
			}
			[buf appendString:key];
			[buf appendString:@"="];
			[buf appendString:value];
			[buf appendString:@"; "];
		}
		[self setString:buf forKey:CURLOPT_COOKIE];
	}
}

/*"	Set post data as a string, for XML-RPC communication and such that doesn't want data pairs.
"*/
- (void) setPostString:(NSString *)inPostString
{
	[self setString:inPostString forKey:CURLOPT_POSTFIELDS];
}

/*"	Set the dictionary of post options, and make the request be a HTTP POST. Note that this does not have a way to specify the encoding.

"*/

- (void) setPostDictionary:(NSDictionary *)inDictionary
{
	NSString *postFields = @"";
	if (nil != inDictionary)
	{
		postFields = [inDictionary formatForHTTP];
	}
	[self setString:postFields forKey:CURLOPT_POSTFIELDS];
}

/*"	Set the dictionary of post options, and make the request be a HTTP POST, specifying the string encoding.
"*/
- (void) setPostDictionary:(NSDictionary *)inDictionary encoding:(NSStringEncoding) inEncoding
{
	NSString *postFields = @"";
	if (nil != inDictionary)
	{
		postFields = [inDictionary formatForHTTPUsingEncoding:inEncoding];
	}
	[self setString:postFields forKey:CURLOPT_POSTFIELDS];
}
 
/*"	Pass in the time in seconds that the transfer should be below the given speed limit for the library to consider it too slow and abort.
"*/
- (void) setLowSpeedTime:(long) inSeconds
{
	curl_easy_setopt([self curl], CURLOPT_LOW_SPEED_TIME, inSeconds);
}

/*"	Pass in the transfer speed in bytes per second that the transfer should be below
 during the low speed seconds for the library to consider it too slow and abort.
"*/
- (void) setLowSpeedLimit:(long) inBytes
{
	curl_easy_setopt([self curl], CURLOPT_LOW_SPEED_LIMIT, inBytes);
}

/*" Set the verbosity of CURL.
"*/
- (void) setVerbose: (BOOL) beVerbose
{
	curl_easy_setopt([self curl], CURLOPT_VERBOSE, beVerbose);
}

// -----------------------------------------------------------------------------
#pragma mark -- MULTIPART POST DICTIONARIES
// -----------------------------------------------------------------------------

/*" Sets the values to use for a multipart POST operation. The values dictionary may include NSString, NSNumber, and NSData objects. NSString objects will automatically be converted to UTF8.

The method setPostDictionary: is similar, except that it encodes all of the form data into a single string that is sent along with the request. For example, given a dictionary such as:

{name = "Andrew Zamler-Carhart"; company = "KavaSoft"}

...setPostDictionary: will cause curl to send an HTTP request such as the following:

POST /form.php HTTP/1.1
Content-Type: application/x-www-form-urlencoded
name=Andrew+Zamler-Carhart&company=KavaSoft

Some web servers expect to receive form data in a multipart format, especially those that accept binary file uploads. Passing the same dictionary to this method will result in an HTTP request like the following:

POST /form.php HTTP/1.1
Expect: 100-continue
Content-Type: multipart/form-data; boundary=curlf2RiuFIdRn36daIvycelja9wqMl

After the client receives a "HTTP/1.1 100 Continue" message, multipart data such as the following will be sent:

--curlf2RiuFIdRn36daIvycelja9wqMl
	Content-Disposition: form-data; name="name"

	Andrew Zamler-Carhart
--curlf2RiuFIdRn36daIvycelja9wqMl
	Content-Disposition: form-data; name="company"

	KavaSoft
--curlf2RiuFIdRn36daIvycelja9wqMl--

This method is based on sample code included in the libcurl guide:

http://curl.haxx.se/libcurl/c/the-guide.html

The command-line utility tcpdump is useful for debugging HTTP transactions. You can monitor all traffic to and from the web server www.myserver.com using the following command:

	sudo tcpdump -Atq -s 0 host www.myserver.com
	"*/

- (void) setMultipartPostDictionary: (NSDictionary *) values {
	[self setMultipartPostDictionary: values headers: nil];
}

/*" This method is like setMultipartPostDictionary:, but it allows you to specify custom headers for some of the values. It takes a dictionary of headers, using keys that match those in the values dictionary. Custom headers are optional, so you only need to supply key/value pairs for those form elements that require custom headers. If you don't need any custom headers at all, just pass nil or use the simpler method above.

For example, given these dictionaries as input:

values = {title = "Hello"; body = "<HTML><BODY>Hello, world!</BODY></HTML>"}
headers = {body = "Content-Type: text/html; filename = \"hello.html\""}

...the following data will sent to the HTTP server:

--curlf2RiuFIdRn36daIvycelja9wqMl
	Content-Disposition: form-data; name="title"

	Hello
--curlf2RiuFIdRn36daIvycelja9wqMl
	Content-Disposition: form-data; name="body"
				Content-Type: text/html; filename = "hello.html"

	<HTML><BODY>Hello, world!</BODY></HTML>
--curlf2RiuFIdRn36daIvycelja9wqMl--
	"*/

- (void) setMultipartPostDictionary: (NSDictionary *) values 
	headers: (NSDictionary *) headers 
{
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	
	NSEnumerator *keysEnum = [values keyEnumerator];
	NSString *key;

	while (key = [keysEnum nextObject]) {
		id value = [values objectForKey: key];
		NSString *customHeader = [headers objectForKey: key];
		const char *valueCString = "";
			
		// set valueCString to a C string representation of the value object
		// We use UTF8String, because the docs for NSString state, "Use of [cString]
		// is discouraged as it will be deprecated in the near future. Instead it is
		// recommended to use UTF8String to convert arbitrary NSStrings to a 
		// lossless 8-bit representation. 
		if ([value isKindOfClass: [NSData class]]) {
			valueCString = (const char*)[(NSData *) value bytes];
		} else if ([value isKindOfClass: [NSNumber class]]) {
			valueCString = [[(NSNumber *) value stringValue] UTF8String];
		} else if ([value isKindOfClass: [NSString class]]) {
			valueCString = [(NSString *) value UTF8String];
		}
			
		if (customHeader) {
			// create the header structure 
			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, [customHeader UTF8String]);

			// add the value to the form using the custom header
			curl_formadd(&post, &last, CURLFORM_COPYNAME, [key UTF8String], 
							 CURLFORM_COPYCONTENTS, valueCString, 
							 CURLFORM_CONTENTHEADER, headers, CURLFORM_END);
		} else {
			// add the value to the form using just the default header
			curl_formadd(&post, &last, CURLFORM_COPYNAME, [key UTF8String],
							 CURLFORM_COPYCONTENTS, valueCString, CURLFORM_END);
		}
	}

	// Set the post data
	curl_easy_setopt(mCURL, CURLOPT_HTTPPOST, post);
}

// -----------------------------------------------------------------------------
#pragma mark -- INFO GETTERS -- AFTER THE PERFORM.
// -----------------------------------------------------------------------------
//
// Wrappers around curl_easy_getinfo
//
//

/*"	Return the last used effective URL. %{This doesn't ever seem to give me anything real}

"*/

- (NSString *)effectiveURL
{
	char url[1024];
	curl_easy_getinfo([self curl], CURLINFO_EFFECTIVE_URL, url);
	return [NSString stringWithCString:url];
}

/*"	Return the last received HTTP code.

"*/

- (long)httpCode
{
	long aLong;
	curl_easy_getinfo([self curl], CURLINFO_HTTP_CODE, &aLong);
	return aLong;
}

/*"	Return the remote time
	of the retrieved document. If you get 0, it can be
	because of many reasons (unknown, the server hides
	it or the server doesn't support the command that
	tells document time etc) and the time of the document is unknown.

"*/

- (long)fileTime
{
	long aLong;
	curl_easy_getinfo([self curl], CURLINFO_FILETIME, &aLong);
	return aLong;
}

/*"	Return the total transaction time in seconds for the previous transfer.
"*/

- (double)totalTime
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_TOTAL_TIME, &aDouble);
	return aDouble;
}

/*"	Return the time, in seconds, it took from the start until the name resolving was completed.

"*/

- (double)nameLookupTime
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_NAMELOOKUP_TIME, &aDouble);
	return aDouble;
}

/*"	Return the time, in seconds, it took from the start until the connect to the remote host (or proxy) was completed.

"*/

- (double)connectTime
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_CONNECT_TIME, &aDouble);
	return aDouble;
}

/*"	Return the time, in
	seconds, it took from the start until the file
	transfer is just about to begin. This includes all
	pre-transfer commands and negotiations that are specific to the
	particular protocol(s) involved.

"*/

- (double)pretransferTime
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_PRETRANSFER_TIME, &aDouble);
	return aDouble;
}

/*"	Return the total amount of bytes that were uploaded.

"*/

- (double)uploadSize
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_SIZE_UPLOAD, &aDouble);
	return aDouble;
}

/*"	Return the total amount of bytes that were downloaded.

"*/

- (double)downloadSize
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_SIZE_DOWNLOAD, &aDouble);
	return aDouble;
}

/*"	Return the average download speed that curl measured for the complete download.

"*/

- (double)downloadSpeed
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_SPEED_DOWNLOAD, &aDouble);
	return aDouble;
}

/*"	Return the average upload speed that curl measured for the complete upload.

"*/

- (double)uploadSpeed
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_SPEED_UPLOAD, &aDouble);
	return aDouble;
}

/*"	Return the total size of all the headers received.

"*/

- (long)headerSize
{ 
	long aLong;
	curl_easy_getinfo([self curl], CURLINFO_HEADER_SIZE, &aLong);
	return aLong;
}

/*"	Return the total size
	of the issued requests. This is so far only for HTTP
	requests. Note that this may be more than one
	request if #setFollowsRedirects is YES.

"*/

- (long)requestSize
{ 
	long aLong;
	curl_easy_getinfo([self curl], CURLINFO_REQUEST_SIZE, &aLong);
	return aLong;
}

/*"	Return the content length of the download. This is the value read from the Content-Length: field.

"*/

- (double)downloadContentLength
{ 
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &aDouble);
	return aDouble;
}

/*"	Return the specified size of the upload.
"*/

- (double)uploadContentLength
{
	double aDouble;
	curl_easy_getinfo([self curl], CURLINFO_CONTENT_LENGTH_UPLOAD, &aDouble);
	return aDouble;
}

/*"	Sets the accepted encodings. If encodings is a non-zero length string, then it is used as the
value for the Accept-encoding header. If it is a zero length string, then all supported
encodings are used (gzip, deflate at the time of writing). If it is nil, then the Accept-encoding
header is cleared. gzip got support in version 7.10.5 or later of the curl library.
"*/
- (void)setAcceptEncoding:(NSString *)encodings
{
	[self setString:encodings forKey:CURLOPT_ENCODING];
}

/*"	Sets CURL to accept all supported encodings transparently.
"*/
- (void)setAcceptCompression:(BOOL)yorn
{
	if( yorn )
		[self setAcceptEncoding:@""];
	else
		[self setAcceptEncoding:nil];
}

@end
