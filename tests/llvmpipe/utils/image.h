/*
 * Copyright © 2012-2021 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include "piglit-ext.h"

class Image {
   #pragma pack(push,2)
   struct bmp_file_header {
      uint16_t bfType;
      uint32_t bfSize;
      uint16_t bfReserved1;
      uint16_t bfReserved2;
      uint32_t bfOffBits;
   };
   #pragma pack(pop)

   struct bmp_info_header {
      uint32_t biSize;
      int32_t biWidth;
      int32_t biHeight;
      uint16_t biPlanes;
      uint16_t biBitCount;
      uint32_t biCompression;
      uint32_t biSizeImage;
      int32_t biXPelsPerMeter;
      int32_t biYPelsPerMeter;
      uint32_t biClrUsed;
      uint32_t biClrImportant;
   };

   struct bmp_rgb_quad {
      uint8_t rgbBlue;
      uint8_t rgbGreen;
      uint8_t rgbRed;
      uint8_t rgbAlpha;
   };

public:
   static bool savePPM(const char* filename, int w, int h, const float* image)
   {
      FILE* fh = fopen(filename, "wb");
      if (!fh)
         return false;

      fprintf(fh, "P6\n%d %d\n%d\n", w, h, 255);

      for (int y = 0; y < h; y++) {
         for (int x = 0; x < w; x++) {
            const float* pixel = &image[(y*w + x) * 4];

            unsigned char r = float_to_ubyte(pixel[0]);
            unsigned char g = float_to_ubyte(pixel[1]);
            unsigned char b = float_to_ubyte(pixel[2]);

            fwrite(&r, 1, 1, fh);
            fwrite(&g, 1, 1, fh);
            fwrite(&b, 1, 1, fh);
         }
      }


      fclose(fh);
      return true;
   }

   static bool saveBMP(const char *filename, unsigned width, unsigned height, const float *rgba){
      FILE *fh;
      struct bmp_file_header bmfh;
      struct bmp_info_header bmih;
      unsigned x, y;

      if(!rgba)
         return false;

      bmfh.bfType = 0x4d42;
      bmfh.bfSize = 14 + 40 + height*width*4;
      bmfh.bfReserved1 = 0;
      bmfh.bfReserved2 = 0;
      bmfh.bfOffBits = 14 + 40;

      bmih.biSize = 40;
      bmih.biWidth = width;
      bmih.biHeight = height;
      bmih.biPlanes = 1;
      bmih.biBitCount = 32;
      bmih.biCompression = 0;
      bmih.biSizeImage = height*width*4;
      bmih.biXPelsPerMeter = 0;
      bmih.biYPelsPerMeter = 0;
      bmih.biClrUsed = 0;
      bmih.biClrImportant = 0;

      fh = fopen(filename, "wb");
      if(!fh)
         return false;

      fwrite(&bmfh, 14, 1, fh);
      fwrite(&bmih, 40, 1, fh);

      y = height;
      while(y--) {
         const float *ptr = rgba + (width * y * 4);
         for(x = 0; x < width; ++x)
         {
            struct bmp_rgb_quad pixel;
            pixel.rgbRed   = float_to_ubyte(ptr[x*4 + 0]);
            pixel.rgbGreen = float_to_ubyte(ptr[x*4 + 1]);
            pixel.rgbBlue  = float_to_ubyte(ptr[x*4 + 2]);
            pixel.rgbAlpha = float_to_ubyte(ptr[x*4 + 3]);
            fwrite(&pixel, 1, 4, fh);
         }
      }

      fclose(fh);
      return true;
   }
};
