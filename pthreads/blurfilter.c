/*
  File: blurfilter.c

  Implementation of blurfilter function.
    
 */

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#include "ppmio.h"
#include "blurfilter.h"


pixel* pix(pixel* image, const int xx, const int yy, const int xsize)
{
  register int off = xsize*yy + xx;

#ifdef DBG
  if(off >= MAX_PIXELS) {
    fprintf(stderr, "\n Ooops: %d %d %d\n",xx,yy,xsize);
  }
#endif
  return (image + off);
}


void* blurfilterX(void* tParam){
  struct threadDataBlurfilter* threadData = (threadDataBlurfilter*)tParam;
  const int xsize = threadData->xsize;

  pixel* src = threadData->src;
  pixel* dst = threadData->dst;
  const int radius = threadData->radius;
  const double* w = threadData->w;

  const int yStart = threadData->yStart;
  const int yEnd = threadData->yEnd;

  int x,y,x2, wi;
  double r,g,b,n, wc;
  
  for (y=yStart; y<yEnd; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix(src, x, y, xsize)->r;
      g = w[0] * pix(src, x, y, xsize)->g;
      b = w[0] * pix(src, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
      	wc = w[wi];
      	x2 = x - wi;
      	if(x2 >= 0) {
      	  r += wc * pix(src, x2, y, xsize)->r;
      	  g += wc * pix(src, x2, y, xsize)->g;
      	  b += wc * pix(src, x2, y, xsize)->b;
      	  n += wc;
      	}
      	x2 = x + wi;
      	if(x2 < xsize) {
      	  r += wc * pix(src, x2, y, xsize)->r;
      	  g += wc * pix(src, x2, y, xsize)->g;
      	  b += wc * pix(src, x2, y, xsize)->b;
      	  n += wc;
      	}
      }
      pix(dst,x,y, xsize)->r = r/n;
      pix(dst,x,y, xsize)->g = g/n;
      pix(dst,x,y, xsize)->b = b/n;
    }
  }
  
  pthread_exit(0);
}

void* blurfilterY(void* tParam){
  struct threadDataBlurfilter* threadData = (threadDataBlurfilter*)tParam;
  const int xsize = threadData->xsize;
  const int ysize = threadData->ysize;

  pixel* src = threadData->src;
  pixel* dst = threadData->dst;  
  const int radius = threadData->radius;
  const double* w = threadData->w;
  
  const int yStart = threadData->yStart;
  const int yEnd = threadData->yEnd;

  int x,y,y2, wi;
  double r,g,b,n, wc;
  
  for (y=yStart; y<yEnd; y++) {
    for (x=0; x<xsize; x++) {
      r = w[0] * pix(dst, x, y, xsize)->r;
      g = w[0] * pix(dst, x, y, xsize)->g;
      b = w[0] * pix(dst, x, y, xsize)->b;
      n = w[0];
      for ( wi=1; wi <= radius; wi++) {
      	wc = w[wi];
      	y2 = y - wi;
      	if(y2 >= 0) {
      	  r += wc * pix(dst, x, y2, xsize)->r;
      	  g += wc * pix(dst, x, y2, xsize)->g;
      	  b += wc * pix(dst, x, y2, xsize)->b;
      	  n += wc;
      	}
      	y2 = y + wi;
      	if(y2 < ysize) {
      	  r += wc * pix(dst, x, y2, xsize)->r;
      	  g += wc * pix(dst, x, y2, xsize)->g;
      	  b += wc * pix(dst, x, y2, xsize)->b;
      	  n += wc;
      	}
      }
      pix(src,x,y, xsize)->r = r/n;
      pix(src,x,y, xsize)->g = g/n;
      pix(src,x,y, xsize)->b = b/n;
    }
  }
  
  //for(y=y_start; y<y_end; y++)
  //  printf("%d", pix(src, xsize-1, y, xsize)->r);
  
  pthread_exit(0);
}

