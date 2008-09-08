/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/firefox-3.0rc1-source/mozilla/netwerk/cache/public/nsICache.idl
 */

#ifndef __gen_nsICache_h__
#define __gen_nsICache_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
typedef PRInt32 nsCacheStoragePolicy;

typedef PRInt32 nsCacheAccessMode;


/* starting interface:    nsICache */
#define NS_ICACHE_IID_STR "ec1c0063-197d-44bb-84ba-7525d50fc937"

#define NS_ICACHE_IID \
  {0xec1c0063, 0x197d, 0x44bb, \
    { 0x84, 0xba, 0x75, 0x25, 0xd5, 0x0f, 0xc9, 0x37 }}

/**
 * nsICache is a namespace for various cache constants.  It does not represent
 * an actual object.
 */
class NS_NO_VTABLE NS_SCRIPTABLE nsICache {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICACHE_IID)

  /**
     * Access Modes
     *
     *
     * Mode Requested | Not Cached          | Cached
     * ------------------------------------------------------------------------
     * READ           | KEY_NOT_FOUND       | NS_OK
     *                | Mode = NONE         | Mode = READ
     *                | No Descriptor       | Descriptor
     * ------------------------------------------------------------------------
     * WRITE          | NS_OK               | NS_OK            (Cache service
     *                | Mode = WRITE        | Mode = WRITE      dooms existing
     *                | Descriptor          | Descriptor        cache entry)
     * ------------------------------------------------------------------------
     * READ_WRITE     | NS_OK               | NS_OK
     * (1st req.)     | Mode = WRITE        | Mode = READ_WRITE
     *                | Descriptor          | Descriptor
     * ------------------------------------------------------------------------
     * READ_WRITE     | N/A                 | NS_OK
     * (Nth req.)     |                     | Mode = READ
     *                |                     | Descriptor
     * ------------------------------------------------------------------------
     *
     *
     * Access Requested:
     *
     * READ	       - I only want to READ, if there isn't an entry just fail
     * WRITE       - I have something new I want to write into the cache, make
     *               me a new entry and doom the old one, if any.
     * READ_WRITE  - I want to READ, but I'm willing to update an existing
     *               entry if necessary, or create a new one if none exists.
     *
     *
     * Access Granted:
     *
     * NONE        - No descriptor is provided. You get zilch. Nada. Nothing.
     * READ		   - You can READ from this descriptor.
     * WRITE	   - You must WRITE to this descriptor because the cache entry
     *               was just created for you.
     * READ_WRITE  - You can READ the descriptor to determine if it's valid,
     *               you may WRITE if it needs updating.
     *
     *
     * Comments:
     *
     * If you think that you might need to modify cached data or meta data,
     * then you must open a cache entry requesting WRITE access.  Only one
     * cache entry descriptor, per cache entry, will be granted WRITE access.
     * 
     * Usually, you will request READ_WRITE access in order to first test the
     * meta data and informational fields to determine if a write (ie. going
     * to the net) may actually be necessary.  If you determine that it is 
     * not, then you would mark the cache entry as valid (using MarkValid) and
     * then simply read the data from the cache.
     *
     * A descriptor granted WRITE access has exclusive access to the cache
     * entry up to the point at which it marks it as valid.  Once the cache
     * entry has been "validated", other descriptors with READ access may be
     * opened to the cache entry.
     *
     * If you make a request for READ_WRITE access to a cache entry, the cache
     * service will downgrade your access to READ if there is already a
     * cache entry descriptor open with WRITE access.
     *
     * If you make a request for only WRITE access to a cache entry and another
     * descriptor with WRITE access is currently open, then the existing cache
     * entry will be 'doomed', and you will be given a descriptor (with WRITE
     * access only) to a new cache entry.
     *
     */
  enum { ACCESS_NONE = 0 };

  enum { ACCESS_READ = 1 };

  enum { ACCESS_WRITE = 2 };

  enum { ACCESS_READ_WRITE = 3 };

  /**
     * Storage Policy
     *
     * The storage policy of a cache entry determines the device(s) to which
     * it belongs.  See nsICacheSession and nsICacheEntryDescriptor for more
     * details.
     *
     * STORE_ANYWHERE        - Allows the cache entry to be stored in any device.
     *                         The cache service decides which cache device to use
     *                         based on "some resource management calculation."
     * STORE_IN_MEMORY       - Requires the cache entry to reside in non-persistent
     *                         storage (ie. typically in system RAM).
     * STORE_ON_DISK         - Requires the cache entry to reside in persistent
     *                         storage (ie. typically on a system's hard disk).
     * STORE_ON_DISK_AS_FILE - Requires the cache entry to reside in persistent
     *                         storage, and in a separate file.
     * STORE_OFFLINE         - Requires the cache entry to reside in persistent,
     *                         reliable storage for offline use.
     */
  enum { STORE_ANYWHERE = 0 };

  enum { STORE_IN_MEMORY = 1 };

  enum { STORE_ON_DISK = 2 };

  enum { STORE_ON_DISK_AS_FILE = 3 };

  enum { STORE_OFFLINE = 4 };

  /**
     * All entries for a cache session are stored as streams of data or
     * as objects.  These constant my be used to specify the type of entries
     * when calling nsICacheService::CreateSession().
     */
  enum { NOT_STREAM_BASED = 0 };

  enum { STREAM_BASED = 1 };

  /**
     * The synchronous OpenCacheEntry() may be blocking or non-blocking.  If a cache entry is
     * waiting to be validated by another cache descriptor (so no new cache descriptors for that
     * key can be created, OpenCacheEntry() will return NS_ERROR_CACHE_WAIT_FOR_VALIDATION in
     * non-blocking mode.  In blocking mode, it will wait until the cache entry for the key has
     * been validated or doomed.  If the cache entry is validated, then a descriptor for that
     * entry will be created and returned.  If the cache entry was doomed, then a descriptor
     * will be created for a new cache entry for the key. 
     */
  enum { NON_BLOCKING = 0 };

  enum { BLOCKING = 1 };

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsICache, NS_ICACHE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICACHE \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICACHE(_to) \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICACHE(_to) \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsCache : public nsICache
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICACHE

  nsCache();

private:
  ~nsCache();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsCache, nsICache)

nsCache::nsCache()
{
  /* member initializers and constructor code */
}

nsCache::~nsCache()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif

#include "nsNetError.h"

#endif /* __gen_nsICache_h__ */
