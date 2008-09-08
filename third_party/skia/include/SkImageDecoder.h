/* include/graphics/SkImageDecoder.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkImageDecoder_DEFINED
#define SkImageDecoder_DEFINED

#include "SkBitmap.h"
#include "SkRefCnt.h"

class SkStream;

/** \class SkImageDecoder

    Base class for decoding compressed images into a SkBitmap
*/
class SkImageDecoder {
public:
    virtual ~SkImageDecoder();
    
    enum Format {
        kUnknown_Format,
        kBMP_Format,
        kGIF_Format,
        kICO_Format,
        kJPEG_Format,
        kPNG_Format,
        kWBMP_Format,
        
        kLastKnownFormat = kWBMP_Format
    };
        
    /** Return the compressed data's format (see Format enum)
    */
    virtual Format getFormat() const;

    /** Returns true if the decoder should try to dither the resulting image.
        The default setting is true.
    */
    bool getDitherImage() const { return fDitherImage; }
    
    /** Set to true if the the decoder should try to dither the resulting image.
        The default setting is true.
    */
    void setDitherImage(bool dither) { fDitherImage = dither; }

    /** \class Peeker
    
        Base class for optional callbacks to retrieve meta/chunk data out of
        an image as it is being decoded.
    */
    class Peeker : public SkRefCnt {
    public:
        /** Return true to continue decoding, or false to indicate an error, which
            will cause the decoder to not return the image.
        */
        virtual bool peek(const char tag[], const void* data, size_t length) = 0;
    };

    Peeker* getPeeker() const { return fPeeker; }
    Peeker* setPeeker(Peeker*);
    
    /** \class Peeker
    
        Base class for optional callbacks to retrieve meta/chunk data out of
        an image as it is being decoded.
    */
    class Chooser : public SkRefCnt {
    public:
        virtual void begin(int count) {}
        virtual void inspect(int index, SkBitmap::Config config, int width, int height) {}
        /** Return the index of the subimage you want, or -1 to choose none of them.
        */
        virtual int choose() = 0;
    };

    Chooser* getChooser() const { return fChooser; }
    Chooser* setChooser(Chooser*);

    SkBitmap::Allocator* getAllocator() const { return fAllocator; }
    SkBitmap::Allocator* setAllocator(SkBitmap::Allocator*);

    // sample-size, if set to > 1, tells the decoder to return a smaller than
    // original bitmap, sampling 1 pixel for every size pixels. e.g. if sample
    // size is set to 3, then the returned bitmap will be 1/3 as wide and high,
    // and will contain 1/9 as many pixels as the original.
    // Note: this is a hint, and the codec may choose to ignore this, or only
    // approximate the sample size.
    int getSampleSize() const { return fSampleSize; }
    void setSampleSize(int size);
    void resetSampleSize() { this->setSampleSize(1); }

    /** Passed to the decode method. If kDecodeBounds_Mode is passed, then
        only the bitmap's width/height/config need be set. If kDecodePixels_Mode
        is passed, then the bitmap must have pixels or a pixelRef.
    */
    enum Mode {
        kDecodeBounds_Mode, //!< only return width/height/config in bitmap
        kDecodePixels_Mode  //!< return entire bitmap (including pixels)
    };
    
    /** Given a stream, decode it into the specified bitmap.
        If the decoder can decompress the image, it calls bitmap.setConfig(),
        and then if the Mode is kDecodePixels_Mode, call allocPixelRef(),
        which will allocated a pixelRef. To access the pixel memory, the codec
        needs to call lockPixels/unlockPixels on the
        bitmap. It can then set the pixels with the decompressed image.
        If the image cannot be decompressed, return false.
        
        note: document use of Allocator, Peeker and Chooser <reed>
    */
    virtual bool onDecode(SkStream*, SkBitmap* bitmap, SkBitmap::Config pref,
                          Mode) = 0;

    /** Given a stream, this will try to find an appropriate decoder object.
        If none is found, the method returns NULL.
    */
    static SkImageDecoder* Factory(SkStream*);
    
    /** Decode the image stored in the specified file, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeFile(const char file[], SkBitmap* bitmap,
                             SkBitmap::Config prefConfig, Mode);
    static bool DecodeFile(const char file[], SkBitmap* bitmap)
    {
        return DecodeFile(file, bitmap, SkBitmap::kNo_Config, kDecodePixels_Mode);
    }
    /** Decode the image stored in the specified memory buffer, and store the
        result in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeMemory(const void* buffer, size_t size, SkBitmap* bitmap,
                             SkBitmap::Config prefConfig, Mode);
    static bool DecodeMemory(const void* buffer, size_t size, SkBitmap* bitmap)
    {
        return DecodeMemory(buffer, size, bitmap, SkBitmap::kNo_Config,
                            kDecodePixels_Mode);
    }
    /** Decode the image stored in the specified SkStream, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most
        natural config given the image data. If pref something other than
        kNo_Config, the decoder will attempt to decode the image into that
        format, unless there is a conflict (e.g. the image has per-pixel alpha
        and the bitmap's config does not support that), in which case the
        decoder will choose a closest match configuration.
    */
    static bool DecodeStream(SkStream* stream, SkBitmap* bitmap,
                             SkBitmap::Config prefConfig, Mode);
    static bool DecodeStream(SkStream* stream, SkBitmap* bitmap)
    {
        return DecodeStream(stream, bitmap, SkBitmap::kNo_Config,
                            kDecodePixels_Mode);
    }
    
    /*  Given a format, return true if there is a currently installed decoder
        for that format. Since a given build may not include all codecs (to save
        code-size), this may return false.
    */
    static bool SupportsFormat(Format);

    /** Return the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config, but this can be changed with SetDeviceConfig()
    */
    static SkBitmap::Config GetDeviceConfig();
    /** Set the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config.
        This can be queried with GetDeviceConfig()
    */
    static void SetDeviceConfig(SkBitmap::Config);

  /** @cond UNIT_TEST */
    SkDEBUGCODE(static void UnitTest();)
  /** @endcond */

protected:
    SkImageDecoder();

    // helper function for decoders to handle the (common) case where there is only
    // once choice available in the image file.
    bool chooseFromOneChoice(SkBitmap::Config config, int width, int height) const;

    /*  Helper for subclasses. Call this to allocate the pixel memory given the bitmap's
        width/height/rowbytes/config. Returns true on success. This method handles checking
        for an optional Allocator.
    */
    bool allocPixelRef(SkBitmap*, SkColorTable*) const;

private:
    Peeker*                 fPeeker;
    Chooser*                fChooser;
    SkBitmap::Allocator*    fAllocator;
    int                     fSampleSize;
    bool                    fDitherImage;

    // illegal
    SkImageDecoder(const SkImageDecoder&);
    SkImageDecoder& operator=(const SkImageDecoder&);
};

#ifdef SK_SUPPORT_IMAGE_ENCODE

class SkWStream;

class SkImageEncoder {
public:
    enum Type {
        kJPEG_Type,
        kPNG_Type
    };
    static SkImageEncoder* Create(Type);

    virtual ~SkImageEncoder();

    /*  Quality ranges from 0..100 */

    bool encodeFile(const char file[], const SkBitmap&, int quality = 80);
    bool encodeStream(SkWStream*, const SkBitmap&, int quality = 80);

protected:
    virtual bool onEncode(SkWStream*, const SkBitmap&, int quality) = 0;
};

#endif /* SK_SUPPORT_IMAGE_ENCODE */

///////////////////////////////////////////////////////////////////////

#endif

