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
//
// Contains some urlmon.h definitions that are found in the Windows Vista
// Platform SDK. This file is can be used to program to those definitions
// without having the Vista PSDK.
//
// Only a small number of interface definitions have been hoisted:
//  IInternetProtocolEx
//  IUri
//  IWinInetCacheHints
//  IWinInetCacheHints2
//
// We only define these if they have not already been defined in urlmon.h
// so this interface file should be safe to use with the Vista PSDK.

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_URLMON_IE7_EXTRAS_H__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_URLMON_IE7_EXTRAS_H__

#include <urlmon.h>

// Forward declarations

#ifndef __IInternetProtocolEx_FWD_DEFINED__
#define __IInternetProtocolEx_FWD_DEFINED__
typedef interface IInternetProtocolEx IInternetProtocolEx;
#endif   /* __IInternetProtocolEx_FWD_DEFINED__ */

#ifndef __IUri_FWD_DEFINED__
#define __IUri_FWD_DEFINED__
typedef interface IUri IUri;
#endif   /* __IUri_FWD_DEFINED__ */

#ifndef __IWinInetCacheHints_FWD_DEFINED__
#define __IWinInetCacheHints_FWD_DEFINED__
typedef interface IWinInetCacheHints IWinInetCacheHints;
#endif   /* __IWinInetCacheHints_FWD_DEFINED__ */

#ifndef __IWinInetCacheHints2_FWD_DEFINED__
#define __IWinInetCacheHints2_FWD_DEFINED__
typedef interface IWinInetCacheHints2 IWinInetCacheHints2;
#endif   /* __IWinInetCacheHints2_FWD_DEFINED__ */


// Interface definitions

#ifndef __IInternetProtocolEx_INTERFACE_DEFINED__
#define __IInternetProtocolEx_INTERFACE_DEFINED__
MIDL_INTERFACE("C7A98E66-1010-492c-A1C8-C809E1F75905")
IInternetProtocolEx : public IInternetProtocol
{
public:
    virtual HRESULT STDMETHODCALLTYPE StartEx(
        /* [in] */ IUri *pUri,
        /* [in] */ IInternetProtocolSink *pOIProtSink,
        /* [in] */ IInternetBindInfo *pOIBindInfo,
        /* [in] */ DWORD grfPI,
        /* [in] */ HANDLE_PTR dwReserved) = 0;
};
// EXTERN_C const IID IID_IInternetProtocolEx;
#define IID_IInternetProtocolEx __uuidof(IInternetProtocolEx)
#endif  /* __IInternetProtocolEx_INTERFACE_DEFINED__ */

#ifndef __IUri_INTERFACE_DEFINED__
#define __IUri_INTERFACE_DEFINED__
typedef /* [public][public][public][public][public][helpstring] */
enum __MIDL_IUri_0001
    {  Uri_PROPERTY_ABSOLUTE_URI  = 0,
  Uri_PROPERTY_STRING_START  = Uri_PROPERTY_ABSOLUTE_URI,
  Uri_PROPERTY_AUTHORITY  = ( Uri_PROPERTY_STRING_START + 1 ) ,
  Uri_PROPERTY_DISPLAY_URI  = ( Uri_PROPERTY_AUTHORITY + 1 ) ,
  Uri_PROPERTY_DOMAIN  = ( Uri_PROPERTY_DISPLAY_URI + 1 ) ,
  Uri_PROPERTY_EXTENSION  = ( Uri_PROPERTY_DOMAIN + 1 ) ,
  Uri_PROPERTY_FRAGMENT  = ( Uri_PROPERTY_EXTENSION + 1 ) ,
  Uri_PROPERTY_HOST  = ( Uri_PROPERTY_FRAGMENT + 1 ) ,
  Uri_PROPERTY_PASSWORD  = ( Uri_PROPERTY_HOST + 1 ) ,
  Uri_PROPERTY_PATH  = ( Uri_PROPERTY_PASSWORD + 1 ) ,
  Uri_PROPERTY_PATH_AND_QUERY  = ( Uri_PROPERTY_PATH + 1 ) ,
  Uri_PROPERTY_QUERY  = ( Uri_PROPERTY_PATH_AND_QUERY + 1 ) ,
  Uri_PROPERTY_RAW_URI  = ( Uri_PROPERTY_QUERY + 1 ) ,
  Uri_PROPERTY_SCHEME_NAME  = ( Uri_PROPERTY_RAW_URI + 1 ) ,
  Uri_PROPERTY_USER_INFO  = ( Uri_PROPERTY_SCHEME_NAME + 1 ) ,
  Uri_PROPERTY_USER_NAME  = ( Uri_PROPERTY_USER_INFO + 1 ) ,
  Uri_PROPERTY_STRING_LAST  = Uri_PROPERTY_USER_NAME,
  Uri_PROPERTY_HOST_TYPE  = ( Uri_PROPERTY_STRING_LAST + 1 ) ,
  Uri_PROPERTY_DWORD_START  = Uri_PROPERTY_HOST_TYPE,
  Uri_PROPERTY_PORT  = ( Uri_PROPERTY_DWORD_START + 1 ) ,
  Uri_PROPERTY_SCHEME  = ( Uri_PROPERTY_PORT + 1 ) ,
  Uri_PROPERTY_ZONE  = ( Uri_PROPERTY_SCHEME + 1 ) ,
  Uri_PROPERTY_DWORD_LAST  = Uri_PROPERTY_ZONE
    }   Uri_PROPERTY;

typedef /* [public][helpstring] */
enum __MIDL_IUri_0002
    {  Uri_HOST_UNKNOWN  = 0,
  Uri_HOST_DNS  = ( Uri_HOST_UNKNOWN + 1 ) ,
  Uri_HOST_IPV4  = ( Uri_HOST_DNS + 1 ) ,
  Uri_HOST_IPV6  = ( Uri_HOST_IPV4 + 1 ) ,
  Uri_HOST_IDN  = ( Uri_HOST_IPV6 + 1 )
    }   Uri_HOST_TYPE;

MIDL_INTERFACE("A39EE748-6A27-4817-A6F2-13914BEF5890")
IUri : public IUnknown
{
public:
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPropertyBSTR(
        /* [range][in] */ Uri_PROPERTY uriProp,
        /* [out] */ BSTR *pbstrProperty,
        /* [in] */ DWORD dwFlags) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPropertyLength(
        /* [range][in] */ Uri_PROPERTY uriProp,
        /* [out] */ DWORD *pcchProperty,
        /* [in] */ DWORD dwFlags) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPropertyDWORD(
        /* [range][in] */ Uri_PROPERTY uriProp,
        /* [out] */ DWORD *pdwProperty,
        /* [in] */ DWORD dwFlags) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasProperty(
        /* [range][in] */ Uri_PROPERTY uriProp,
        /* [out] */ BOOL *pfHasProperty) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAbsoluteUri(
        /* [out] */ BSTR *pbstrAbsoluteUri) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAuthority(
        /* [out] */ BSTR *pbstrAuthority) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDisplayUri(
        /* [out] */ BSTR *pbstrDisplayString) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDomain(
        /* [out] */ BSTR *pbstrDomain) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetExtension(
        /* [out] */ BSTR *pbstrExtension) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFragment(
        /* [out] */ BSTR *pbstrFragment) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetHost(
        /* [out] */ BSTR *pbstrHost) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPassword(
        /* [out] */ BSTR *pbstrPassword) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPath(
        /* [out] */ BSTR *pbstrPath) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPathAndQuery(
        /* [out] */ BSTR *pbstrPathAndQuery) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetQuery(
        /* [out] */ BSTR *pbstrQuery) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRawUri(
        /* [out] */ BSTR *pbstrRawUri) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSchemeName(
        /* [out] */ BSTR *pbstrSchemeName) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUserInfo(
        /* [out] */ BSTR *pbstrUserInfo) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUserName(
        /* [out] */ BSTR *pbstrUserName) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetHostType(
        /* [out] */ DWORD *pdwHostType) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPort(
        /* [out] */ DWORD *pdwPort) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScheme(
        /* [out] */ DWORD *pdwScheme) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetZone(
        /* [out] */ DWORD *pdwZone) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetProperties(
        /* [out] */ LPDWORD pdwFlags) = 0;

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsEqual(
        /* [in] */ IUri *pUri,
        /* [out] */ BOOL *pfEqual) = 0;
};
// EXTERN_C const IID IID_IUri;
#define IID_IUri __uuidof(IUri)
#endif /* __IUri_INTERFACE_DEFINED__ */

#ifndef __IWinInetCacheHints_INTERFACE_DEFINED__
#define __IWinInetCacheHints_INTERFACE_DEFINED__
typedef /* [unique] */ IWinInetCacheHints *LPWININETCACHEHINTS;
MIDL_INTERFACE("DD1EC3B3-8391-4fdb-A9E6-347C3CAAA7DD")
IWinInetCacheHints : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetCacheExtension(
        /* [in] */ LPCWSTR pwzExt,
        /* [size_is][out][in] */ LPVOID pszCacheFile,
        /* [out][in] */ DWORD *pcbCacheFile,
        /* [out][in] */ DWORD *pdwWinInetError,
        /* [out][in] */ DWORD *pdwReserved) = 0;

};
// EXTERN_C const IID IID_IWinInetCacheHints;
#define IID_IWinInetCacheHints __uuidof(IWinInetCacheHints)
#endif  /* __IWinInetCacheHints_INTERFACE_DEFINED__ */


#ifndef __IWinInetCacheHints2_INTERFACE_DEFINED__
#define __IWinInetCacheHints2_INTERFACE_DEFINED__
typedef /* [unique] */ IWinInetCacheHints2 *LPWININETCACHEHINTS2;
MIDL_INTERFACE("7857AEAC-D31F-49bf-884E-DD46DF36780A")
IWinInetCacheHints2 : public IWinInetCacheHints
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetCacheExtension2(
        /* [in] */ LPCWSTR pwzExt,
        /* [size_is][out] */ WCHAR *pwzCacheFile,
        /* [out][in] */ DWORD *pcchCacheFile,
        /* [out] */ DWORD *pdwWinInetError,
        /* [out] */ DWORD *pdwReserved) = 0;
};
// EXTERN_C const IID IID_IWinInetCacheHints2;
#define IID_IWinInetCacheHints2 __uuidof(IWinInetCacheHints2)
#endif  /* __IWinInetCacheHints2_INTERFACE_DEFINED__ */


// An additional QUERYOPTION defined for IE7 in the platform SDK for Vista
enum ExtraQUERYOPTION {
  QUERY_USES_HISTORYFOLDER = QUERY_IS_SAFE + 1
};


#endif  // GEARS_THIRD_PARTY_PASSTHRU_APP_URLMON_IE7_EXTRAS_H__
