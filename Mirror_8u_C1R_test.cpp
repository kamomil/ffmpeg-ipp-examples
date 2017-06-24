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
#include "Mirror_8u_C1R_test.h"


int ippiMirror_8u_C1R_daf(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiAxis flip){

  unsigned char* dst_ptr = pDst+dstStep*(roiSize.height-1);

  if(flip != ippAxsHorizontal){
    Output("error: ippiMirror_8u_c1R_daf: only ippAxsHorizontal is supported\n");
    return -1;
  }

  for(;roiSize.height;roiSize.height--){

    memcpy(dst_ptr,pSrc,roiSize.width);
    pSrc += srcStep;
    dst_ptr -= dstStep;
  }
  return 0;
}

int test_ippiMirror_8u_C1R_replacement(AVFrame *frame, double* ncc_val){

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

  unsigned char *dst_ipp =    (unsigned char*)malloc(h*dstStep);
  unsigned char *dst_ffmpeg = (unsigned char*)malloc(h*dstStep);

  IppStatus st = ippStsNoErr;

  unsigned char* src_offst = pSrc[0]+(offset_h*srcStep) + offset_w;
    
  if(r<0){
    Output("error: test_ippiCopy_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    goto end;
  }

  
  if(!dst_ipp || !dst_ffmpeg){
    Output("error: test_ippiCopy_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
      
  }
  
  memset(dst_ipp,0,h*dstStep);
  memset(dst_ffmpeg,0,h*dstStep);

  roi.height    = hr;
  roi.width     = wr;
  
  //IppStatus ippiMirror_8u_c1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiAxis flip){
  //ippiMirror_8u_C1R( pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ipp,dstStep,roi,ippAxsHorizontal);

  st = ippiMirror_8u_C1R(src_offst,srcStep,dst_ipp,dstStep,roi,ippAxsHorizontal);

  if(st != ippStsNoErr){
    Output("Mirror_8u_C1R_test: st IS ERROR: %d\n",st);
    r = -1;
    goto end;
  }
  r = ippiMirror_8u_C1R_daf(src_offst,srcStep,dst_ffmpeg,dstStep,roi,ippAxsHorizontal);

  if(r<0){
    Output("Mirror_8u_C1R_test: ippiMirror_8u_C1R_daf failed\n");
    goto end;
  }

  //print_img("ipp",&src_offst,w,h,1);
  print_img2("ipp",&src_offst,srcStep,wr,hr,1);
  print_img2("mirror_ipp",&dst_ipp,dstStep,wr,hr,1);
  //ippiMirror_8u_C1R_daf(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);

  print_img2("mirror_ffmpeg",&dst_ffmpeg,dstStep,wr,hr,1);

  *ncc_val = ncc2(dst_ipp,dstStep,dst_ffmpeg,dstStep,w,h);
  
 end:
  if(pSrc[0])
    free(pSrc[0]);
  if(dst_ipp)
    free(dst_ipp);
  if(dst_ffmpeg)
    free(dst_ffmpeg);
  return r;
 
}
