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
#include "Set_8u_C1R_test.h"

//ippiSet_<mod>(Ipp<datatype> value, Ipp<datatype>* pDst, int dstStep, IppiSize roiSize);

int ippiSet_8u_C1R_daf(Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize roiSize){

  for(;roiSize.height;roiSize.height--){

    memset(pDst,value,roiSize.width);
    pDst += dstStep;
  }
  return 0;
}

//TDOO - this is jus a seleton
int test_ippiSet_8u_C1R_replacement(AVFrame *frame, double* ncc_val){

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

  roi.width = wr;
  roi.height = hr;

  //this allocates pSrc
  int r = convert_to_GRAY8(frame, pSrc , src_stride,extra_add_to_src_width);

  IppStatus st = ippStsNoErr;

  unsigned char* src_offst_pp = pSrc[0]+(offset_h*srcStep) + offset_w;

  unsigned char* src_ffmpeg = (unsigned char*)malloc(src_stride[0]*h);

  unsigned char* src_offst_ff;

  char rect_filename[256] = {0};
  if(r<0){
    Output("error: test_ippiSet_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    goto end;
  }
  
  if(!src_ffmpeg){
    Output("error: test_ippiSet_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
      
  }
  src_offst_ff = src_ffmpeg+(offset_h*srcStep) + offset_w;
  memcpy(src_ffmpeg,pSrc[0],src_stride[0]*h);
  


  roi.height    = hr;
  roi.width     = wr;
  
  //IppStatus ippiSet_8u_C1R(Ipp8u value, Ipp8u pDst, int dstStep, IppiSize roiSize);

  Output("seting %dx%d out of %dx%d\n",wr,hr,w,h);
  st = ippiSet_8u_C1R(255,src_offst_pp,srcStep,roi);

  if(st != ippStsNoErr){
    Output("Set_8u_C1R_test: st IS ERROR: %d\n",st);
    r = -1;
    goto end;
  }
  r = ippiSet_8u_C1R_daf(255,src_offst_ff,srcStep,roi);

  if(r<0){
    Output("Set_8u_C1R_test: ippiSet_8u_C1R_daf failed\n");
    goto end;
  }

  snprintf(rect_filename, sizeof(rect_filename), "srectf%02d.yuv", 1);
  pgm_save(src_offst_ff, src_stride[0], w, h,rect_filename);
  //Output("rect size is %dx%d\n",srcRect.width,srcRect.height);

  //print_img("ipp",&src_offst,w,h,1);
  //print_img2("ipp",&src_offst_pp,srcStep,34,50,1);
  //print_img2("set_ipp",&dst_ipp,dstStep,wr,hr,1);
  //ippiMirror_8u_C1R_daf(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);

  //print_img2("mirror_ffmpeg",&dst_ffmpeg,dstStep,wr,hr,1);

  *ncc_val = ncc2(src_offst_pp,srcStep,src_offst_ff,srcStep,w-offset_w,h-offset_h);
  Output("ncc is %f\n",*ncc_val);
  
 end:
  if(pSrc[0])
    free(pSrc[0]);
  if(src_ffmpeg)
    free(src_ffmpeg);

  exit(1);
  //return r;
 
}
