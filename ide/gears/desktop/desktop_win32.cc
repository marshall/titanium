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
// Windows-specific implementation of desktop shortcut creation.

// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. WIN32_CPPSRCS) are implemented
#ifdef WIN32
#include <assert.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include <wininet.h>

#include "gears/desktop/desktop.h"

#include "gears/base/common/basictypes.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/png_utils.h"
#include "gears/base/common/scoped_win32_handles.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#if BROWSER_IE
#include "gears/base/common/vista_utils.h"
#endif
#ifdef WINCE
#include "gears/desktop/dll_data_wince.h"
#endif
#include "gears/desktop/shortcut_utils_win32.h"
#include "gears/localserver/common/http_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
#include "genfiles/product_constants.h"

struct IcoHeader {
  uint16 reserved;
  uint16 type;
  uint16 count;
};

struct IcoDirectory {
  uint8 width;
  uint8 height;
  uint8 color_count;
  uint8 reserved;
  uint16 planes;
  uint16 bpp;
  uint32 data_size;
  uint32 offset;
};

#ifdef WINCE
bool WriteIconAsDLL(const char16 *file_path,
                    const uint8 *icon_data,
                    const int data_size) {
  // On WinCE, a shortcut can only make use of an icon embedded in a module as
  // a resource. There seems to be no documentation on the internal structure of
  // a WinCE EXE or DLL, so we can't create one from scratch on the fly. Instead
  // we use 'prototype' DLLs which contains no code, only a single icon. We
  // take a copy of that DLL and replace the icon with the required icon.
  //
  // There seems to be no documentation on the format used to represent an icon
  // when embedded in a module. Testing shows that it is not the same as on
  // Win32 (see http://msdn2.microsoft.com/en-us/library/ms997538.aspx). First,
  // the ICONIMAGE structure does not contain the AND mask (BITMAPINFOHEADER,
  // color table and XOR mask only). The data for each image is followed by a
  // footer in place of the AND mask. Also, the headers at the start of the
  // resource differ and the presence of the icon affects other areas of the
  // DLL. For these reasons, we use a separate prototype DLL for each of the two
  // sets of image sizes we support.

  // Read the icon header data, storing size information for each image. After
  // this, icon_data points to the start of the bitmap data for the first image.
  const IcoHeader *header = reinterpret_cast<const IcoHeader*>(icon_data);
  icon_data += sizeof(IcoHeader);
  struct IconImageData {
    int image_size;  // BITMAPINFOHEADER, color table, XOR mask and AND mask.
    int and_mask_size;
    const uint8 *footer;
    int footer_size;
  };
  std::vector<IconImageData> icon_image_data;
  for (int i = 0; i < header->count; ++i) {
    const IcoDirectory* directory =
        reinterpret_cast<const IcoDirectory*>(icon_data);
    icon_data += sizeof(IcoDirectory);
    int width = directory->width;
    int height = directory->height;
    // We only support square images.
    assert(width == height);
    IconImageData data;
    int mask_row_bytes = width / 8;
    mask_row_bytes = ((mask_row_bytes + 3) / 4) * 4;  // round up, multiple of 4
    data.and_mask_size = height * mask_row_bytes;
    data.image_size = sizeof(BITMAPINFOHEADER) +
                      4 * height * width +       // Color data
                      height * mask_row_bytes +  // XOR mask
                      data.and_mask_size;        // AND mask
    icon_image_data.push_back(data);
  }

  // For now, we only support two cases ...
  // - a single 16x16 image
  // - a 32x32 followed by a 16x16 image
  // Note that CreateIcoFile ensures that icon images are present in the icon
  // data in order of decreasing size.
  const int kImageSize16 =
      sizeof(BITMAPINFOHEADER) + 4 * 16 * 16 + 16 * 4 + 16 * 4;
  const int kImageSize32 =
      sizeof(BITMAPINFOHEADER) + 4 * 32 * 32 + 32 * 4 + 32 * 4;
  const uint8 *icon_dll_begin;
  const uint8 *icon_dll_header;
  const uint8 *icon_dll_end;
  int icon_dll_begin_size;
  int icon_dll_header_size;
  int icon_dll_end_size;
  if (icon_image_data.size() == 1) {
    assert(icon_image_data[0].image_size == kImageSize16);
    icon_dll_begin = reinterpret_cast<const uint8*>(kIcon16DllBegin);
    icon_dll_begin_size = kIcon16DllBeginSize;
    icon_dll_header = reinterpret_cast<const uint8*>(kIcon16DllHeader);
    icon_dll_header_size = kIcon16DllHeaderSize;
    icon_image_data[0].footer =
        reinterpret_cast<const uint8*>(kIcon16Image16Footer);
    icon_image_data[0].footer_size = kIcon16Image16FooterSize;
    icon_dll_end = reinterpret_cast<const uint8*>(kIcon16DllEnd);
    icon_dll_end_size = kIcon16DllEndSize;
  } else if (icon_image_data.size() == 2) {
    assert(icon_image_data[0].image_size == kImageSize32 &&
           icon_image_data[1].image_size == kImageSize16);
    icon_dll_begin = reinterpret_cast<const uint8*>(kIcon32and16DllBegin);
    icon_dll_begin_size = kIcon32and16DllBeginSize;
    icon_dll_header = reinterpret_cast<const uint8*>(kIcon32and16DllHeader);
    icon_dll_header_size = kIcon32and16DllHeaderSize;
    icon_image_data[0].footer =
        reinterpret_cast<const uint8*>(kIcon32and16Image32Footer);
    icon_image_data[0].footer_size = kIcon32and16Image32FooterSize;
    icon_image_data[1].footer =
        reinterpret_cast<const uint8*>(kIcon32and16Image16Footer);
    icon_image_data[1].footer_size = kIcon32and16Image16FooterSize;
    icon_dll_end = reinterpret_cast<const uint8*>(kIcon32and16DllEnd);
    icon_dll_end_size = kIcon32and16DllEndSize;
  } else {
    assert(false);
    return false;
  }

  // Calculate the file size.
  int file_size = icon_dll_begin_size +
                  icon_dll_header_size +
                  icon_dll_end_size;
  for (int i = 0; i < static_cast<int>(icon_image_data.size()); ++i) {
    file_size += icon_image_data[i].image_size -
                 icon_image_data[i].and_mask_size +
                 icon_image_data[i].footer_size;

  }
  uint8 *data = new uint8[file_size];
  if (NULL == data) {
    return false;
  }
  int data_index = 0;

  // Write the start of the DLL.
  memcpy(&data[data_index], icon_dll_begin, icon_dll_begin_size);
  data_index += icon_dll_begin_size;

  // Write the icon header.
  memcpy(&data[data_index], icon_dll_header, icon_dll_header_size);
  data_index += icon_dll_header_size;

  // Write the icon data.
  for (int i = 0; i < static_cast<int>(icon_image_data.size()); ++i) {
    memcpy(&data[data_index],
           icon_data,
           icon_image_data[i].image_size - icon_image_data[i].and_mask_size);
    data_index += icon_image_data[i].image_size -
                  icon_image_data[i].and_mask_size;
    memcpy(&data[data_index],
           icon_image_data[i].footer,
           icon_image_data[i].footer_size);
    data_index += icon_image_data[i].footer_size;
    icon_data += icon_image_data[i].image_size;
  }

  // Write the end of the DLL.
  memcpy(&data[data_index], icon_dll_end, icon_dll_end_size);

  // Write the data to file.
  // If the file already exists, CreateNewFile will fail, but that's OK.
  File::CreateNewFile(file_path);
  if (!File::WriteBytesToFile(file_path, data, file_size)) {
    return false;
  }
  return true;
}
#endif // WINCE

// Creates the icon file which contains the various different sized icons.
static bool CreateIcoFile(const std::string16 &icons_path,
                          const Desktop::ShortcutInfo &shortcut) {
  std::vector<const Desktop::IconData *> icons_to_write;

  // Add each icon size that has been provided to the icon list.
#ifdef WINCE
  // We don't use 128x128 or 48x48 on WinCE.
#else
  // We ignore 128x128 because it isn't supported by Windows.
  if (!shortcut.icon48x48.raw_data.empty()) {
    icons_to_write.push_back(&shortcut.icon48x48);
  }
#endif
  if (!shortcut.icon32x32.raw_data.empty()) {
    icons_to_write.push_back(&shortcut.icon32x32);
  }

  if (!shortcut.icon16x16.raw_data.empty()) {
    icons_to_write.push_back(&shortcut.icon16x16);
  }

  // Make sure we have at least one icon.
  if (icons_to_write.empty()) {
    return false;
  }

  // Initialize to the size of the header.
  int data_size = sizeof(IcoHeader);

  for (size_t i = 0; i < icons_to_write.size(); ++i) {
    // Increase data_size by size of the icon data.
    data_size += sizeof(BITMAPINFOHEADER);

    // Increase data_size by size of the image and mask data.

    // Note: in the .ico format, each *row* of image and mask data must be
    // a multiple of 4 bytes.  Pad if necessary.
    int row_bytes;

    // 32 bits per pixel for the image data.
    row_bytes = icons_to_write[i]->width * 4;  // already a multiple of 4
    data_size += row_bytes * icons_to_write[i]->height;

    // 1 bit per pixel for the XOR mask, then 1 bit per pixel for the AND mask.
    row_bytes = icons_to_write[i]->width / 8;
    row_bytes = ((row_bytes + 3) / 4) * 4;  // round up to multiple of 4
    data_size += row_bytes * icons_to_write[i]->height;  // XOR mask
    data_size += row_bytes * icons_to_write[i]->height;  // AND mask

    // Increase data_size by size of directory entry.
    data_size += sizeof(IcoDirectory);
  }

  // Allocate the space for the icon.
  uint8 *data = new uint8[data_size];
  memset(data, 0, data_size);

  IcoHeader *icon_header = reinterpret_cast<IcoHeader *>(data);
  icon_header->reserved = 0;  // Must be 0;
  icon_header->type = 1;  // 1 for ico.
  icon_header->count = icons_to_write.size();

  // Icon image data starts past the header and the directory.
  int dest_offset =
      sizeof(IcoHeader) +
      icons_to_write.size() * sizeof(IcoDirectory);
  for (size_t i = 0; i < icons_to_write.size(); ++i) {
    IcoDirectory directory;
    directory.width = icons_to_write[i]->width;
    directory.height = icons_to_write[i]->height;
    directory.color_count = 0;
    directory.reserved = 0;
    directory.planes = 1;
    directory.bpp = 32;

    // Size of the header + size of the pixels + size of the bitmask.
    // TODO: Is this correct? Should it account for rounding to multiples of 4?
    directory.data_size =
        sizeof(BITMAPINFOHEADER) +
        4 * icons_to_write[i]->width * icons_to_write[i]->height +
        icons_to_write[i]->width * icons_to_write[i]->height / 4;

    directory.offset = dest_offset;

    BITMAPINFOHEADER bmp_header;
    memset(&bmp_header, 0, sizeof(bmp_header));
    bmp_header.biSize = sizeof(bmp_header);
    bmp_header.biWidth = icons_to_write[i]->width;
    // 'biHeight' is the combined height of the XOR and AND masks.
    bmp_header.biHeight = icons_to_write[i]->height * 2;
    bmp_header.biPlanes = 1;
    bmp_header.biBitCount = 32;

    // Write the directory entry.
    memcpy(&data[sizeof(IcoHeader) + i * sizeof(IcoDirectory)],
           reinterpret_cast<uint8 *>(&directory),
           sizeof(IcoDirectory));

    // Write the bitmap header to the data segment.
    memcpy(&data[dest_offset],
           reinterpret_cast<uint8 *>(&bmp_header),
           sizeof(BITMAPINFOHEADER));

    // Move the offset past the header.
    dest_offset += sizeof(BITMAPINFOHEADER);

    // Write the color data.
    // Note that icon rows are stored bottom to top, so we flip the row order.
    for (int row = (icons_to_write[i]->height - 1); row >= 0; --row) {
      int src_row_offset = row * icons_to_write[i]->width * 4;

      // Copy a single row.
      memcpy(&data[dest_offset],
             reinterpret_cast<const uint8*>(
                 &icons_to_write[i]->raw_data.at(src_row_offset)),
             4 * icons_to_write[i]->width);

      // Move the write offset forward one row.
      dest_offset += 4 * icons_to_write[i]->width;
    }

    // Compute mask information.
    int mask_row_bytes = icons_to_write[i]->width / 8;
    mask_row_bytes = ((mask_row_bytes + 3) / 4) * 4;  // round up, multiple of 4

    // Write the XOR mask.
    for (int row = (icons_to_write[i]->height - 1); row >= 0; --row) {
      // 'stripe' is an 8-column segment, for grouping 1bpp data into bytes
      for (int stripe = 0; stripe < (icons_to_write[i]->width / 8); ++stripe) {
        int xor_mask_value = 0;
        for (int bit = 0; bit < 8; ++bit) {
          // If alpha is 0x00, make bit transparent (1), else leave opaque (0).
          int raw_pixel_offset =
              (((row * icons_to_write[i]->width) + (stripe * 8) + bit) * 4);
          if (0 == icons_to_write[i]->raw_data.at(raw_pixel_offset + 3)) {
            xor_mask_value |= (0x80 >> bit);
          }
        }
        data[dest_offset + stripe] = static_cast<uint8>(xor_mask_value);
      }
      // Update offset *after* finishing mask row, as it may include padding.
      dest_offset += mask_row_bytes;
    }

    // Move the write offset past the AND mask (unused in WinXP and later).
    dest_offset += mask_row_bytes * icons_to_write[i]->height;
  }

#ifdef WINCE
  // On WinCE, we don't write the icon directly, but embed it in a DLL.
  bool success = WriteIconAsDLL(icons_path.c_str(), data, data_size);
#else
  File::CreateNewFile(icons_path.c_str());
  bool success = File::WriteBytesToFile(icons_path.c_str(), data, data_size);
#endif

  delete[] data;

  return success;
}

bool Desktop::CreateShortcutPlatformImpl(const SecurityOrigin &origin,
                                         const ShortcutInfo &shortcut,
                                         uint32 locations,
                                         std::string16 *error) {
  char16 browser_path[MAX_PATH] = {0};

  if (!GetModuleFileName(NULL, browser_path, MAX_PATH)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string16 icons_path;
  if (!GetDataDirectory(origin, &icons_path)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  AppendDataName(STRING16(L"icons"), kDataSuffixForDesktop, &icons_path);
  icons_path += kPathSeparator;
  icons_path += shortcut.app_name;
#ifdef WINCE
  // On WinCE, we don't write the icon directly, but embed it in a DLL. We don't
  // use extension '.dll' because this casues some devices to warn the user that
  // they are running an untrusted application when the icon is used.
  icons_path += STRING16(L".icon");
#else
  icons_path += STRING16(L".ico");
#endif

  if (!CreateIcoFile(icons_path, shortcut)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

#if BROWSER_IE
  if (VistaUtils::IsRunningOnVista()) {
    std::string16 broker_path;
    if (!GetInstallDirectory(&broker_path)) {
      *error = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }
    broker_path += STRING16(L"\\vista_broker.exe");

    // Build up the command line
    std::string16 locations_string = IntegerToString16(locations);
    const char16 *command_line_parts[] = {
      broker_path.c_str(),
      shortcut.app_name.c_str(),
      browser_path,
      shortcut.app_url.c_str(),
      icons_path.c_str(),
      locations_string.c_str(),
    };

    std::string16 command_line;
    for (int i = 0; i < ARRAYSIZE(command_line_parts); ++i) {
      // Escape any double quotes since we also use them for argument
      // delimiters.
      std::string16 part(command_line_parts[i]);
      ReplaceAll(part, std::string16(STRING16(L"\\")),
                 std::string16(STRING16(L"\\\\")));

      command_line += STRING16(L"\"");
      command_line += part;
      command_line += STRING16(L"\" ");
    }

    STARTUPINFO startup_info = {0};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info = {0};

    BOOL success = CreateProcessW(NULL,  // application name (NULL to get from
                                         // command line)
                                  const_cast<char16 *>(command_line.c_str()),
                                  NULL,  // process attributes (NULL means
                                         // process handle not inheritable)
                                  NULL,  // thread attributes (NULL means thread
                                         // handle not inheritable)
                                  FALSE, // inherit handles
                                  0,     // creation flags
                                  NULL,  // environment block (NULL to use
                                         // parent's)
                                  NULL,  // starting block (NULL to use
                                         // parent's)
                                  &startup_info,
                                  &process_info);
    if (!success) {
      *error = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }

    // To make sure that process and thread handles gets destroyed on exit.
    SAFE_HANDLE process_handle(process_info.hProcess);
    SAFE_HANDLE thread_handle(process_info.hThread);

    // Wait for process to exit.
    const int kTimeoutMs = 10000;
    if (WaitForSingleObject(process_info.hProcess,
                            kTimeoutMs) != WAIT_OBJECT_0) {
      *error = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }

    DWORD return_value = 0;
    if (!GetExitCodeProcess(process_info.hProcess, &return_value)) {
      *error = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }

    if (return_value != 0) {
      *error = STRING16(L"Could not create shortcut. Broker failed on line: ");
      *error += IntegerToString16(return_value);
      *error += STRING16(L".");
      return false;
    }
    return true;
  }
#endif

  for (uint32 location = 0x8000; location > 0; location >>= 1) {
    if (locations & location) {
      if (!CreateShortcutFileWin32(shortcut.app_name, browser_path,
                                   shortcut.app_url, icons_path,
                                   location, error)) {
        return false;
      }
    }
  }
  return true;
}
#endif  // #ifdef WIN32
