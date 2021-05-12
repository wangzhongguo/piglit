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

#include "piglit-ext.h"
#include "framebuffer.h"
#include "image.h"

float ubyte_to_float(unsigned char ub)
{
  return (float) ub / 255.0f;
}

unsigned char float_to_ubyte(float f)
{
   return (unsigned char)(CLAMP(f, 0.0f, 1.0f) * 255.0f);
}

GLvoid piglit_draw_rect_colour(float x, float y, float w, float h, const float* colours)
{
   float verts[4][4];

   verts[0][0] = x;
   verts[0][1] = y;
   verts[0][2] = 0.0;
   verts[0][3] = 1.0;

   verts[1][0] = x + w;
   verts[1][1] = y;
   verts[1][2] = 0.0;
   verts[1][3] = 1.0;

   verts[2][0] = x + w;
   verts[2][1] = y + h;
   verts[2][2] = 0.0;
   verts[2][3] = 1.0;

   verts[3][0] = x;
   verts[3][1] = y + h;
   verts[3][2] = 0.0;
   verts[3][3] = 1.0;

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   glColorPointer(4, GL_FLOAT, 0, colours);
   glVertexPointer(4, GL_FLOAT, 0, verts);

   glDrawArrays(GL_QUADS, 0, 4);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
}

int piglit_probe_image_rgba_error_mask(int x, int y, int w, int h, const float* image, int channelMask, const unsigned char* errorMask, const char* dump_file_prefix, int* errX, int* errY)
{
   static int lastW = 0, lastH = 0;
   static GLfloat *pixels = 0;

   int i, j, p, c;
   GLfloat *probe;

   if (w != lastW || h != lastH) {
      if (pixels)
         free(pixels);

      pixels = (GLfloat*)malloc(w*h*4*sizeof(float));
   }

   int num_channels = 4;

   glReadPixels(x, y, w, h, GL_RGBA, GL_FLOAT, pixels);

   for (j = 0; j < h; j++) {
      for (i = 0; i < w; i++) {
         const float *expected = &image[(j*w+i)*4];
         probe = &pixels[(j*w+i)*num_channels];

         for (p = 0; p < num_channels; ++p) {
            if (((channelMask >> p) & 1) == 0)
               continue;

            if (fabs(probe[p] - expected[p]) > piglit_tolerance[p]) {
               if (errorMask && errorMask[j * w + i])
                  continue;

               if (errX)
                  *errX = x + i;

               if (errY)
                  *errY = y + j;

               printf("Probe at (%i,%i)\n", x+i, y+j);
               printf("  Expected:  ");
               for (c = 0; c < num_channels; ++c) {
                   printf(" %12.9g", expected[c]);
               }

               printf("\n  Observed:  ");
               for (c = 0; c < num_channels; ++c) {
                   printf(" %12.9g", probe[c]);
               }
               printf("\n  Difference:");
               for (c = 0; c < num_channels; ++c) {
                   printf(" %12.9g", fabs(probe[c] - expected[c]));
               }

               printf("\n  Tolerance: ");
               for (c = 0; c < num_channels; ++c) {
                   printf(" %12.9g", piglit_tolerance[c]);
               }

               printf("\n");


               if (dump_file_prefix) {
                  char filename[128];

                  sprintf(filename, "%s-expected.bmp", dump_file_prefix);
                  Image::saveBMP(filename, w, h, image);

                  sprintf(filename, "%s-observed.bmp", dump_file_prefix);
                  Image::saveBMP(filename, w, h, pixels);
               }

               return 0;
            }
         }
      }
   }

   return 1;
}
