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

// This code is based on nsPNGDecoder in Mozilla and PNGImageDecoder in WebKit.

#include <assert.h>
#include "gears/base/common/png_utils.h"


#if defined(PNG_gAMA_SUPPORTED) && defined(PNG_READ_GAMMA_SUPPORTED)
// Gamma correction constants. Everyone uses these two constants -- 2.2 and
// 1/2.2 because that is what IE did back in the day and they want to be
// compatible. For the whole sad story, see:
// https://bugzilla.mozilla.org/show_bug.cgi?id=53597
// nsPNGDecoder.cpp in Mozilla
static const double kMaxGamma = 21474.83;  // Maximum gamma accepted by png
                                           // library.
static const double kDefaultGamma = 2.2;
static const double kInverseGamma = 1.0 / kDefaultGamma;
#endif

// Maximum pixel dimension we'll try to decode.
static const png_uint_32 kMaxSize = 4096;


static void ConvertBetweenBGRAandRGBA(const unsigned char* input,
                                      int pixel_width, unsigned char* output) {
  for (int x = 0; x < pixel_width; x++) {
    const unsigned char* pixel_in = &input[x * 4];
    unsigned char* pixel_out = &output[x * 4];
    pixel_out[0] = pixel_in[2];
    pixel_out[1] = pixel_in[1];
    pixel_out[2] = pixel_in[0];
    pixel_out[3] = pixel_in[3];
  }
}

static void ConvertRGBAtoRGB(const unsigned char* rgba, int pixel_width,
                             unsigned char* rgb) {
  for (int x = 0; x < pixel_width; x++) {
    const unsigned char* pixel_in = &rgba[x * 4];
    unsigned char* pixel_out = &rgb[x * 3];
    pixel_out[0] = pixel_in[0];
    pixel_out[1] = pixel_in[1];
    pixel_out[2] = pixel_in[2];
  }
}

static void ConvertRGBtoRGBA(const unsigned char* rgb, int pixel_width,
                             unsigned char* rgba) {
  for (int x = 0; x < pixel_width; x++) {
    const unsigned char* pixel_in = &rgb[x * 3];
    unsigned char* pixel_out = &rgba[x * 4];
    pixel_out[0] = pixel_in[0];
    pixel_out[1] = pixel_in[1];
    pixel_out[2] = pixel_in[2];
    pixel_out[3] = 0xff;
  }
}

#if defined(PNG_WRITE_SUPPORTED)
static void ConvertBGRAtoRGB(const unsigned char* bgra, int pixel_width,
                             unsigned char* rgb) {
  for (int x = 0; x < pixel_width; x++) {
    const unsigned char* pixel_in = &bgra[x * 4];
    unsigned char* pixel_out = &rgb[x * 3];
    pixel_out[0] = pixel_in[2];
    pixel_out[1] = pixel_in[1];
    pixel_out[2] = pixel_in[0];
  }
}
#endif

static void ConvertRGBtoBGRA(const unsigned char* rgb, int pixel_width,
                             unsigned char* bgra) {
  for (int x = 0; x < pixel_width; x++) {
    const unsigned char* pixel_in = &rgb[x * 3];
    unsigned char* pixel_out = &bgra[x * 4];
    pixel_out[0] = pixel_in[2];
    pixel_out[1] = pixel_in[1];
    pixel_out[2] = pixel_in[0];
    pixel_out[3] = 0xff;
  }
}


#if defined(PNG_WRITE_SUPPORTED)
// Encoder
// TODO(cprince): Add additional PNG_WRITE_* checks inside this block (as we
// have in the Decoder) if/when we want to support minimal-code PNG writing.

// Passed around as the io_ptr in the png structs so our callbacks know where
// to write data.
struct PngEncodeState {
  PngEncodeState(std::vector<unsigned char>* o) : out(o) {}
  std::vector<unsigned char>* out;
};

// Called by libpng to flush its internal buffer to ours.
static void EncoderWriteCallback(png_structp png, png_bytep data,
                                 png_size_t size) {
  PngEncodeState* state = static_cast<PngEncodeState*>(png_get_io_ptr(png));
  assert(state->out);

  size_t old_size = state->out->size();
  state->out->resize(old_size + size);
  memcpy(&(*state->out)[old_size], data, size);
}

// Automatically destroys the given write structs on destruction to make
// cleanup and error handling code cleaner.
class PngWriteStructDestroyer {
 public:
  PngWriteStructDestroyer(png_struct** ps, png_info** pi) : ps_(ps), pi_(pi) {
  }
  ~PngWriteStructDestroyer() {
    png_destroy_write_struct(ps_, pi_);
  }
 private:
  png_struct** ps_;
  png_info** pi_;

  DISALLOW_EVIL_CONSTRUCTORS(PngWriteStructDestroyer);
};

// static
bool PngUtils::Encode(const unsigned char* input, ColorFormat format,
                      int w, int h, int row_byte_width,
                      bool discard_transparency,
                      std::vector<unsigned char>* output) {
  // Run to convert an input row into the output row format, NULL means no
  // conversion is necessary.
  void (*converter)(const unsigned char* in, int w, unsigned char* out) = NULL;

  int input_color_components, output_color_components;
  int png_output_color_type;
  switch (format) {
    case FORMAT_RGB:
      input_color_components = 3;
      output_color_components = 3;
      png_output_color_type = PNG_COLOR_TYPE_RGB;
      discard_transparency = false;
      break;

    case FORMAT_RGBA:
      input_color_components = 4;
      if (discard_transparency) {
        output_color_components = 3;
        png_output_color_type = PNG_COLOR_TYPE_RGB;
        converter = ConvertRGBAtoRGB;
      } else {
        output_color_components = 4;
        png_output_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        converter = NULL;
      }
      break;

    case FORMAT_BGRA:
      input_color_components = 4;
      if (discard_transparency) {
        output_color_components = 3;
        png_output_color_type = PNG_COLOR_TYPE_RGB;
        converter = ConvertBGRAtoRGB;
      } else {
        output_color_components = 4;
        png_output_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        converter = ConvertBetweenBGRAandRGBA;
      }
      break;

    default:
      assert(false);
      return false;
  }

  // Row stride should be at least as long as the length of the data.
  assert(input_color_components * w <= row_byte_width);

  png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                png_voidp_NULL,
                                                png_error_ptr_NULL,
                                                png_error_ptr_NULL);
  if (!png_ptr)
    return false;
  png_info* info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, NULL);
    return false;
  }
  PngWriteStructDestroyer destroyer(&png_ptr, &info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    // The destroyer will ensure that the structures are cleaned up in this
    // case, even though we may get here as a jump from random parts of the
    // PNG library called below.
    return false;
  }

  // Set our callback for libpng to give us the data.
  PngEncodeState state(output);
  png_set_write_fn(png_ptr, &state, EncoderWriteCallback, NULL);

  png_set_IHDR(png_ptr, info_ptr, w, h, 8, png_output_color_type,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  if (!converter) {
    // No conversion needed, give the data directly to libpng.
    for (int y = 0; y < h; y ++)
      png_write_row(png_ptr,
                    const_cast<unsigned char*>(&input[y * row_byte_width]));
  } else {
    // Needs conversion using a separate buffer.
    unsigned char* row = new unsigned char[w * output_color_components];
    for (int y = 0; y < h; y ++) {
      converter(&input[y * row_byte_width], w, row);
      png_write_row(png_ptr, row);
    }
    delete[] row;
  }

  png_write_end(png_ptr, info_ptr);
  return true;
}
#endif  // defined(PNG_WRITE_SUPPORTED)


#if defined(PNG_READ_SUPPORTED)
// Decoder

class PngDecodeState {
 public:
  PngDecodeState(PngUtils::ColorFormat ofmt, std::vector<unsigned char>* o)
      : output_format(ofmt),
        output_channels(0),
        output(o),
        row_converter(NULL),
        width(0),
        height(0),
        done(false) {
  }

  PngUtils::ColorFormat output_format;
  int output_channels;

  std::vector<unsigned char>* output;

  // Called to convert a row from the library to the correct output format.
  // When NULL, no conversion is necessary.
  void (*row_converter)(const unsigned char* in, int w, unsigned char* out);

  // Size of the image, set in the info callback.
  int width;
  int height;

  // Set to true when we've found the end of the data.
  bool done;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(PngDecodeState);
};

// Called when the png header has been read.
static void DecodeInfoCallback(png_struct* png_ptr, png_info* info_ptr) {
  PngDecodeState* state = static_cast<PngDecodeState*>(
      png_get_progressive_ptr(png_ptr));

  int bit_depth, color_type, interlace_type, compression_type;
  int filter_type, channels;
  png_uint_32 w, h;
  png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);

  // Bounds check. When the image is unreasonably big, we'll error out and
  // end up back at the setjmp call when we set up decoding.
  if (w > kMaxSize || h > kMaxSize)
    longjmp(png_ptr->jmpbuf, 1);
  state->width = static_cast<int>(w);
  state->height = static_cast<int>(h);

  // Expand to ensure we use 24-bit for RGB and 32-bit for RGBA.
  if (color_type == PNG_COLOR_TYPE_PALETTE ||
      (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8))
#ifdef PNG_READ_EXPAND_SUPPORTED
    png_set_expand(png_ptr);
#else
    return;
#endif

  // Transparency for paletted images.
#if defined(PNG_tRNS_SUPPORTED) && defined(PNG_READ_EXPAND_SUPPORTED)
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);
#endif

  // Convert 16-bit to 8-bit.
  if (bit_depth == 16)
#ifdef PNG_READ_16_TO_8_SUPPORTED
    png_set_strip_16(png_ptr);
#else
    return;
#endif

  // Expand grayscale to RGB.
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
    png_set_gray_to_rgb(png_ptr);
#else
    return;
#endif

#if defined(PNG_gAMA_SUPPORTED) && defined(PNG_READ_GAMMA_SUPPORTED)
  // Deal with gamma and keep it under our control.
  double gamma;
  if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
    if (gamma <= 0.0 || gamma > kMaxGamma) {
      gamma = kInverseGamma;
      png_set_gAMA(png_ptr, info_ptr, gamma);
    }
    png_set_gamma(png_ptr, kDefaultGamma, gamma);
  } else {
    png_set_gamma(png_ptr, kDefaultGamma, kInverseGamma);
  }
#endif

  // Tell libpng to send us rows for interlaced pngs.
  if (interlace_type == PNG_INTERLACE_ADAM7)
#if defined(PNG_READ_INTERLACING_SUPPORTED)
    png_set_interlace_handling(png_ptr);
#else
    return;
#endif

  // Update our info now
  png_read_update_info(png_ptr, info_ptr);
  channels = png_get_channels(png_ptr, info_ptr);

  // Pick our row format converter necessary for this data.
  if (channels == 3) {
    switch (state->output_format) {
      case PngUtils::FORMAT_RGB:
        state->row_converter = NULL;  // no conversion necessary
        state->output_channels = 3;
        break;
      case PngUtils::FORMAT_RGBA:
        state->row_converter = &ConvertRGBtoRGBA;
        state->output_channels = 4;
        break;
      case PngUtils::FORMAT_BGRA:
        state->row_converter = &ConvertRGBtoBGRA;
        state->output_channels = 4;
        break;
      default:
        assert(false);
        break;
    }
  } else if (channels == 4) {
    switch (state->output_format) {
      case PngUtils::FORMAT_RGB:
        state->row_converter = &ConvertRGBAtoRGB;
        state->output_channels = 3;
        break;
      case PngUtils::FORMAT_RGBA:
        state->row_converter = NULL;  // no conversion necessary
        state->output_channels = 4;
        break;
      case PngUtils::FORMAT_BGRA:
        state->row_converter = &ConvertBetweenBGRAandRGBA;
        state->output_channels = 4;
        break;
      default:
        assert(false);
        break;
    }
  } else {
    assert(false);
    longjmp(png_ptr->jmpbuf, 1);
  }

  size_t new_size = state->width * state->output_channels * state->height;
  state->output->resize(new_size);

  // sanity-check that we were able to allocate all the memory we asked for.
  if (state->output->size() != new_size) {
    longjmp(png_ptr->jmpbuf, 1);
  }
}

static void DecodeRowCallback(png_struct* png_ptr, png_byte* new_row,
                              png_uint_32 row_num, int pass) {
  PngDecodeState* state = static_cast<PngDecodeState*>(
      png_get_progressive_ptr(png_ptr));

  // libpng should not be giving us interlaced data since that option is turned
  // off.
  assert(pass == 0);

  if (static_cast<int>(row_num) > state->height) {
    assert(false);
    return;
  }

  unsigned char* dest = &(*state->output)[
      state->width * state->output_channels * row_num];
  if (state->row_converter)
    state->row_converter(new_row, state->width, dest);
  else
    memcpy(dest, new_row, state->width * state->output_channels);
}

static void DecodeEndCallback(png_struct* png_ptr, png_info* info) {
  PngDecodeState* state = static_cast<PngDecodeState*>(
      png_get_progressive_ptr(png_ptr));

  // Mark the image as complete, this will tell the Decode function that we
  // have successfully found the end of the data.
  state->done = true;
}

// Automatically destroys the given read structs on destruction to make
// cleanup and error handling code cleaner.
class PngReadStructDestroyer {
 public:
  PngReadStructDestroyer(png_struct** ps, png_info** pi) : ps_(ps), pi_(pi) {
  }
  ~PngReadStructDestroyer() {
    png_destroy_read_struct(ps_, pi_, NULL);
  }
 private:
  png_struct** ps_;
  png_info** pi_;
};

// static
bool PngUtils::Decode(const unsigned char* input, size_t input_size,
                      ColorFormat format, std::vector<unsigned char>* output,
                      int* w, int* h) {
  if (input_size < 8)
    return false;  // Input data too small to be a png

  // Have libpng check the signature, it likes the first 8 bytes.
  if (png_sig_cmp(const_cast<unsigned char*>(input), 0, 8) != 0)
    return false;

  png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                               png_voidp_NULL,
                                               png_error_ptr_NULL,
                                               png_error_ptr_NULL);
  if (!png_ptr)
    return false;

  png_info* info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return false;
  }

  PngReadStructDestroyer destroyer(&png_ptr, &info_ptr);
  if (setjmp(png_jmpbuf(png_ptr))) {
    // The destroyer will ensure that the structures are cleaned up in this
    // case, even though we may get here as a jump from random parts of the
    // PNG library called below.
    return false;
  }

  PngDecodeState state(format, output);

  png_set_progressive_read_fn(png_ptr, &state, &DecodeInfoCallback,
                              &DecodeRowCallback, &DecodeEndCallback);
  png_process_data(png_ptr, info_ptr, const_cast<unsigned char*>(input),
                   input_size);

  if (!state.done) {
    // Fed it all the data but the library didn't think we got all the data, so
    // this file must be truncated.
    output->clear();
    return false;
  }

  *w = state.width;
  *h = state.height;
  return true;
}
#endif  // defined(PNG_READ_SUPPORTED)


// Divides a length of X units into Y spans.  The numbers need not be evenly
// divisible.  The class will return spans of length floor(X/Y) and ceil(X/Y),
// chosen to distribute any error evenly along the length.
//
// The computation is efficient, requiring only one division at initialization
// and no divisions per span.  This makes it suitable for things like
// drawing lines using Bresenham, or shrinking images with a box filter.
class IntegralSplit {
 public:
  // 'length' is the total distance to split into spans.
  IntegralSplit(int length, int num_spans)
      : length_(length), num_spans_(num_spans) {
        small_span_ = length / num_spans;  // integer division
        step_error_ = length % num_spans;
        total_error_ = num_spans >> 1;  // init to spans/2 to 'center' any error
      }

  // Returns the length of the next span.
  int NextSpan() {
    total_error_ += step_error_;
    if (total_error_ >= num_spans_) {
      total_error_ -= num_spans_;
      return small_span_ + 1;
    } else {
      return small_span_;
    }
  }

 private:
  int length_;  // total distance to split
  int num_spans_;

  int total_error_;
  int step_error_;  // amount to add at each step
  int small_span_;  // spans are always (small) or (small + 1)
};

void PngUtils::ShrinkImage(const unsigned char *input, int width, int height,
                          int new_width, int new_height,
                          std::vector<unsigned char> *output) {
  assert(new_width <= width);
  assert(new_height <= height);

  output->reserve(new_width * new_height * 4);

  IntegralSplit height_splitter(height, new_height);
  int base_y = 0;
  for (int i = 0; i < new_height; ++i) {
    IntegralSplit width_splitter(width, new_width);
    int local_height = height_splitter.NextSpan();
    int base_x = 0;
    for (int j = 0; j < new_width; ++j) {
      int local_width = width_splitter.NextSpan();
      unsigned int red = 0;
      unsigned int green = 0;
      unsigned int blue = 0;
      unsigned int alpha = 0;

      // A box of pixels with colours c1...cN and alpha a1...aN is averaged as
      // colour (c1a1+...+cNaN)/N and alpha (a1+...+aN)/N.
      for (int y = base_y; y < base_y + local_height; ++y) {
        for (int x = base_x; x < base_x + local_width; ++x) {
          int offset = ((y * width) + x) * 4;

          // Colour values are scaled by the alpha, so that translucent pixels
          // don't impact the final colour more than they should.
          red += input[offset] * input[offset + 3];
          green += input[offset + 1] * input[offset + 3];
          blue += input[offset + 2] * input[offset + 3];

          // Alpha is just the average value from the input area.
          alpha += input[offset + 3];
        }
      }
      output->push_back(red / (local_height * local_width * 0xFF));
      output->push_back(green / (local_height * local_width * 0xFF));
      output->push_back(blue / (local_height * local_width * 0xFF));
      output->push_back(alpha / (local_height * local_width));

      base_x += local_width;
    }
    base_y += local_height;
  }
}
