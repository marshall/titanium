// Copyright 2007, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "gears/base/common/exception_handler_osx/nshost_macaddress.h"

#import <openssl/md5.h>
#import <stdio.h>

#import <CoreFoundation/CoreFoundation.h>

#import <IOKit/IOKitLib.h>
#import <IOKit/network/IOEthernetController.h>
#import <IOKit/network/IOEthernetInterface.h>
#import <IOKit/network/IONetworkInterface.h>

void GMLog(id x, ...) {}
#define GMDebuggerAssertAlways(x,y,z)

// helper functions
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices);
static kern_return_t GetMACAddress(io_iterator_t intfIterator,
  UInt8 *MACAddress, UInt8 bufferSize);

@implementation NSHost (MACAddress)

// Return the MAC address of this host.
// This must be called from [NSHost currentHost];
// The result may be used as an ID which is unique to this host.
- (NSString *)MACAddress {
  NSString *result = nil;

  if (![self isEqualToHost:[[self class] currentHost]]) {
    GMDebuggerAssertAlways(@"%@ isn't current host %@", self,
      [[self class] currentHost]);
    return nil;
  }

  kern_return_t	kernResult = KERN_SUCCESS;

  io_iterator_t	intfIterator;
  UInt8	MACAddress[kIOEthernetAddressSize];

  kernResult = FindEthernetInterfaces(&intfIterator);

  if (KERN_SUCCESS != kernResult) {
    return nil;
  }
  else {
    kernResult = GetMACAddress(intfIterator, MACAddress, sizeof(MACAddress));

    if (KERN_SUCCESS != kernResult) {
      return nil;
    }
  	else {
      result = [NSString stringWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x",
        MACAddress[0], MACAddress[1], MACAddress[2], MACAddress[3],
        MACAddress[4], MACAddress[5] ];
  	}
  }

  IOObjectRelease(intfIterator);	// Release the iterator.

  return result;
}

// Return the MAC address of this host, obfuscated for privacy.
// This must be called from [NSHost currentHost];
// The result may be used as an ID which is unique to this host.
- (NSString *)obfuscatedMACAddress {
  NSString *address = [self MACAddress];

  if (!address) return nil;

  const char *s = [address UTF8String];

  MD5_CTX c;
  MD5_Init(&c);
  MD5_Update(&c, s, strlen(s) );

  unsigned char hash[16];
  MD5_Final(hash, &c);

  UInt32 *hash32 = (UInt32*)hash;

  NSString *result = [NSString stringWithFormat:@"%04x%04x%04x%04x",
    hash32[0], hash32[1], hash32[2], hash32[3] ];

  return result;
}

@end

// code adapted from Apple sample code GetPrimaryMACAddress.c
// http://developer.apple.com/samplecode/GetPrimaryMACAddress/listing1.html
//

// Returns an iterator containing the primary (built-in) Ethernet interface.
// The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices) {
    kern_return_t		kernResult;
    CFMutableDictionaryRef	matchingDict;
    CFMutableDictionaryRef	propertyMatchDict;

    // Ethernet interfaces are instances of class kIOEthernetInterfaceClass.
    // IOServiceMatching is a convenience function to create a dictionary with
    // the key kIOProviderClassKey and the specified value.
    matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

    // Note that another option here would be:
    // matchingDict = IOBSDMatching("en0");

    if (NULL == matchingDict) {
        GMLog(@"IOServiceMatching returned a NULL dictionary.\n");
    }
    else {
        // Each IONetworkInterface object has a Boolean property with the key
        // kIOPrimaryInterface.
        // Only the primary (built-in) interface has this property set to TRUE.

        // IOServiceGetMatchingServices uses the default matching criteria
        // defined by IOService. This considers only the following properties
        // plus any family-specific matching in this order of precedence
        // (see IOService::passiveMatch):
        //
        // kIOProviderClassKey (IOServiceMatching)
        // kIONameMatchKey (IOServiceNameMatching)
        // kIOPropertyMatchKey
        // kIOPathMatchKey
        // kIOMatchedServiceCountKey
        // family-specific matching
        // kIOBSDNameKey (IOBSDNameMatching)
        // kIOLocationMatchKey

        // The IONetworkingFamily does not define any family-specific matching.
        // This means that in order to have IOServiceGetMatchingServices consider
        // the kIOPrimaryInterface property, we must add that property
        // to a separate dictionary and then add that to our matching dictionary
        // specifying kIOPropertyMatchKey.

        propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
													  &kCFTypeDictionaryKeyCallBacks,
													  &kCFTypeDictionaryValueCallBacks);

        if (NULL == propertyMatchDict) {
            GMLog(@"CFDictionaryCreateMutable returned a NULL dictionary.\n");
        }
        else {
            // Set the value in the dictionary of the property with the
            // given key, or add the key to the dictionary if it doesn't exist.
            // This call retains the value object passed in.
            CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface),
              kCFBooleanTrue);

            // Now add the dictionary containing the matching value for
            // kIOPrimaryInterface to our main matching dictionary.
            // This call will retain propertyMatchDict, so we can release our
            // reference on propertyMatchDict after adding it to matchingDict.
            CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey),
              propertyMatchDict);
            CFRelease(propertyMatchDict);
        }
    }

    // IOServiceGetMatchingServices retains the returned iterator, so release
    // the iterator when we're done with it.
    // IOServiceGetMatchingServices also consumes a reference on the matching
    // dictionary so we don't need to release the dictionary explicitly.
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict,
      matchingServices);
    if (KERN_SUCCESS != kernResult) {
        GMLog(@"IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
    }

    return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address
// of the last one.
// If no interfaces are found the MAC address is set to an empty string.
// In this sample the iterator should contain just the primary interface.
static kern_return_t GetMACAddress( io_iterator_t intfIterator,
                                    UInt8         *MACAddress,
                                    UInt8         bufferSize) {
    io_object_t		intfService;
    io_object_t		controllerService;
    kern_return_t	kernResult = KERN_FAILURE;

    // Make sure the caller provided enough buffer space. Protect against buffer
    // overflow problems.
	if (bufferSize < kIOEthernetAddressSize) {
		return kernResult;
	}

	// Initialize the returned address
    bzero(MACAddress, bufferSize);

    // IOIteratorNext retains the returned object,
    // so release it when we're done with it.
    while ((intfService = IOIteratorNext(intfIterator))) {
        CFTypeRef	MACAddressAsCFData;

        // IONetworkControllers can't be found directly by the
        // IOServiceGetMatchingServices call, since they are hardware nubs
        // and do not participate in driver matching. In other words,
        // registerService() is never called on them. So we've found the
        // IONetworkInterface and will get its parent controller
        // by asking for it specifically.

        // IORegistryEntryGetParentEntry retains the returned object,
        // so release it when we're done with it.
        kernResult = IORegistryEntryGetParentEntry(intfService,
												   kIOServicePlane,
												   &controllerService);

        if (KERN_SUCCESS != kernResult) {
            printf("IORegistryEntryGetParentEntry returned 0x%08x\n",
              kernResult);
        }
        else {
            // Retrieve the MAC address property from the I/O Registry in
            // the form of a CFData
            MACAddressAsCFData = IORegistryEntryCreateCFProperty(
                controllerService,
                CFSTR(kIOMACAddress),
                kCFAllocatorDefault,
                0);

            if (MACAddressAsCFData) {
                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes(MACAddressAsCFData,
                  CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                CFRelease(MACAddressAsCFData);
            }

            // Done with the parent Ethernet controller object so we release it.
            IOObjectRelease(controllerService);
        }

        // Done with the Ethernet interface object so we release it.
        IOObjectRelease(intfService);
    }

    return kernResult;
}
