
#include "stdafx.h"
#include "test_utils.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/intreadwrite.h>
}


int rgb_is_gbr(AVFrame *frame,double* ncc_val){

  int w = frame->width;
  int h = frame->height;
 
  unsigned char *dst_gbrp[4];
  int dst_stride_gbrp[4];

  unsigned char *dst_rgb24[4];
  int dst_stride_rgb24[4];

  
  convert_to_GBRP(frame, dst_gbrp ,dst_stride_gbrp);
  convert_to_RGB24(frame, dst_rgb24 ,dst_stride_rgb24);

  
  unsigned char* packed = (unsigned char *)malloc(w*h*3);
  unsigned char* packed2 = (unsigned char *)malloc(w*h*3);
  unsigned char* rgb_planar[3] = {dst_gbrp[2],dst_gbrp[0],dst_gbrp[1]};
  
  planar_to_packed(rgb_planar,packed,frame->width,frame->height,3);

 
  SwsContext * ctx = sws_getContext(w, h,AV_PIX_FMT_GBRP,w, h ,AV_PIX_FMT_RGB24, 0, NULL, NULL, NULL);
  if (!ctx) {
    Output("error: convert_to_format failed: ctx is null\n");
    return -1;
  }
  int r = 0;
  r = sws_scale(ctx, dst_gbrp ,dst_stride_gbrp, 0, h, &packed2, dst_stride_rgb24);

  if (!ctx) {
    Output("error: sws scale failed: \n");
    return -1;
  }

  //print_img("p1", &packed,  w*3,  5,  1);
  //print_img("p2", &packed2,  w*3,  5,  1);
  
  *ncc_val = ncc(packed,packed2,w*h*3);
    
  free(dst_gbrp[0]);
  free(dst_gbrp[1]);
  free(dst_gbrp[2]);

  free(dst_rgb24[0]);
  free(packed);
  free(packed2);
  
  return 0;

}
