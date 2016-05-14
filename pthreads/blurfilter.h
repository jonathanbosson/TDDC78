/*
  File: blurfilter.h

  Declaration of pixel structure and blurfilter function.
    
 */

#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel;


struct threadDataBlurfilter{
  int threadId;
  int xsize;
  int ysize;
  pixel* src;
  pixel* dst;
  int radius;
  double* w;
  int yStart;
  int yEnd;
};


void* blurfilterX(void* tParam);
void* blurfilterY(void* tParam);

#endif