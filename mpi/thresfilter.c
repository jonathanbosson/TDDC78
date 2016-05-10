#include <pthread.h>
#include "thresfilter.h"

void* thresfilter(void* threadParam){
#define uint unsigned int 

  
  
  
  // Get parameters for computations from threadData
  struct threadDataThresfilter* threadData = (threadDataThresfilter*)threadParam;
  const int xsize = threadData->xsize;
  const int ysize = threadData->ysize;
  pixel* src = threadData->src;
  
  uint sum, i, psum, nump;

  nump = xsize * ysize;

  for(i = 0, sum = 0; i < nump; i++) {
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  sum /= nump;

  for(i = 0; i < nump; i++) {
    psum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
    if(sum > psum) {
      src[i].r = src[i].g = src[i].b = 0;
    }
    else {
      src[i].r = src[i].g = src[i].b = 255;
    }
  }
  pthread_exit(0);
}
