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

/////////////////////////////////////////////////////////////////////////////
//
// Version
//

VS_VERSION_INFO VERSIONINFO
 FILEVERSION    PRODUCT_VERSION_MAJOR,PRODUCT_VERSION_MINOR,PRODUCT_VERSION_BUILD,PRODUCT_VERSION_PATCH
 PRODUCTVERSION PRODUCT_VERSION_MAJOR,PRODUCT_VERSION_MINOR,PRODUCT_VERSION_BUILD,PRODUCT_VERSION_PATCH
 FILEFLAGSMASK 0x3fL
#ifdef DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x4L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904e4"
        BEGIN
            VALUE "CompanyName", "Appcelerator, Inc."
            VALUE "FileVersion", "PRODUCT_VERSION"
            VALUE "LegalCopyright", "Copyright 2008 Appcelerator, Inc. All Rights Reserved."
            VALUE "ProductName", "PRODUCT_FRIENDLY_NAME_UQ PRODUCT_VERSION"
            VALUE "ProductVersion", "PRODUCT_VERSION"
            VALUE "FileDescription", "Appcelerator Titanium"
            VALUE "InternalName", "PRODUCT_SHORT_NAME_UQ.dll"
            VALUE "OriginalFilename", "PRODUCT_SHORT_NAME_UQ.dll"
#if BROWSER_NPAPI
            VALUE "MIMEType", "application/x-googlegears"
#endif
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END
