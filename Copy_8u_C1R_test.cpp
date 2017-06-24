#include "stdafx.h"
extern "C"
{
#include <ipp.h>
#include "ippcc.h"
#include "ippi.h"
}
//#include "media_io.h"
#ifdef WIN32
#include <windows.h>
#include <random>
std::random_device rd;
std::mt19937 gen;
//#include <ctime>//this cause compilation errors
#endif

//#include <random>
#include <stdlib.h>
//#include <stdio.h>
#include <time.h>
#include <math.h>
#include "test_utils.h"
#include "Copy_8u_C1R_test.h"

int ippiCopy_8u_C1R_ffmpeg(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize){

  //Output(" w=%d h=%d  pSrc=%p dstStep=%d srcStep=%d\n",roiSize.width,roiSize.height,pSrc, dstStep,srcStep);
  av_image_copy_plane(pDst, dstStep,pSrc, srcStep,roiSize.width,roiSize.height);  
}
  
int test_ippiCopy_8u_C1R_replacement(AVFrame* frame, double* ncc_val){

  IppiSize roi;

  int w = frame->width;
  int h = frame->height;

  int extra_add_to_dst_width = rand_in_range(1,100);
  int extra_add_to_src_width = rand_in_range(1,100);

  int offset_w = rand_in_range(0,w-1);
  int offset_h = rand_in_range(0,h-1);
  
  int wr = rand_in_range(1,w - offset_w);
  int hr = rand_in_range(1,h - offset_h);

  unsigned char* pSrc[4]={NULL,NULL,NULL,NULL};
  int src_stride[4]={0,0,0,0};

  int srcStep = w+extra_add_to_src_width;
  int dstStep = w+extra_add_to_dst_width;
  

  //this allocates pSrc
  int r = convert_to_GRAY8(frame, pSrc , src_stride,extra_add_to_src_width);

  if(r<0){
    Output("error: test_ippiCopy_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    if(pSrc[0])
      free(pSrc[0]);
    return -1;
  }

  unsigned char *dst_ipp =    (unsigned char*)malloc(h*dstStep);
  unsigned char *dst_ffmpeg = (unsigned char*)malloc(h*dstStep);
  if(!dst_ipp || !dst_ffmpeg){
    Output("error: test_ippiCopy_8u_C1R_replacement: allocation failed\n");
    if(pSrc[0])
      free(pSrc[0]);
    return -1;
  }
  
  memset(dst_ipp,0,h*dstStep);
  memset(dst_ffmpeg,0,h*dstStep);

  roi.height    = hr;
  roi.width     = wr;
  
  //ippiCopy_<mod>(const Ipp<datatype>* pSrc, int srcStep, Ipp<datatype>* pDst, int dstStep, IppiSize roiSize);
  ippiCopy_8u_C1R( pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ipp,dstStep,roi);

  //print_img("copy_ipp",&dst_ipp,10,10,1);
  ippiCopy_8u_C1R_ffmpeg(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);
  //print_img("copy_ffmpeg",&dst_ffmpeg,10,10,1);
  *ncc_val = ncc2(dst_ipp,dstStep,dst_ffmpeg,dstStep,w,h);

  return 0;
}
