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

#include <stdlib.h>
#include <time.h>
#include "test_utils.h"
#include "Mirror_8u_C1R_test.h"

//in ffmpeg this function is done in O(1) time by setting the pointer to the last line and changing the stepSize to negative.
// see "filter_frame" func in  libavfilter/vf_hflip.c

int ippiMirror_8u_C1IR_daf(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip){

  
  unsigned char* tmp = (unsigned char*) malloc(roiSize.width);
  unsigned char* end_ptr = pSrcDst+srcDstStep*(roiSize.height-1);

  int h = roiSize.height/2;

  if(!tmp){
    Output("error: ippiMirror_8u_c1R_daf: allocation failure\n");
    return -1;
  }

  if(flip != ippAxsHorizontal){
    Output("error: ippiMirror_8u_c1R_daf: only ippAxsHorizontal is supported\n");
    free(tmp);
    return -1;
  }

  for(;h;h--){

    //save
    memcpy(tmp,pSrcDst,roiSize.width);
  
    memcpy(pSrcDst,end_ptr,roiSize.width);

    memcpy(end_ptr,tmp,roiSize.width);
    
    pSrcDst += srcDstStep;
    end_ptr -= srcDstStep;
  }
  free(tmp);
  return 0;
}

int test_ippiMirror_8u_C1IR_replacement(AVFrame *frame, double* ncc_val){

    IppiSize roi;

  int w = frame->width;
  int h = frame->height;

  int extra_add_to_width = rand_in_range(1,100);

  int offset_w = rand_in_range(0,w-1);
  int offset_h = rand_in_range(0,h-1);
  
  int wr = rand_in_range(1,w - offset_w);
  int hr = rand_in_range(1,h - offset_h);

  unsigned char* pSrc[4]={NULL,NULL,NULL,NULL};
  int src_stride[4]={0,0,0,0};

  int step = w+extra_add_to_width;
  
  //this allocates pSrc
  int r = convert_to_GRAY8(frame, pSrc , src_stride,extra_add_to_width);

  unsigned char *dst_ipp =    (unsigned char*)malloc(h*step);
  unsigned char *dst_ffmpeg = (unsigned char*)malloc(h*step);

  IppStatus st = ippStsNoErr;

  unsigned char* src_offst = pSrc[0]+(offset_h*step) + offset_w;
    
  if(r<0){
    Output("error: test_ippiCopy_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    goto end;
  }

  
  if(!dst_ipp || !dst_ffmpeg){
    Output("error: test_ippiCopy_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
      
  }
  
  memset(dst_ipp,0,h*step);
  memset(dst_ffmpeg,0,h*step);

  roi.height    = hr;
  roi.width     = wr;
  
  //IppStatus ippiMirror_8u_c1R(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiAxis flip){
  //ippiMirror_8u_C1R( pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ipp,dstStep,roi,ippAxsHorizontal);

  /*
  st = ippiMirror_8u_C1R(src_offst,srcStep,dst_ipp,dstStep,roi,ippAxsHorizontal);

  if(st != ippStsNoErr){
    Output("Mirror_8u_C1R_test: st IS ERROR: %d\n",st);
    r = -1;
    goto end;
  }
  */
  
  print_img2("src before mirror:",&src_offst,step,wr/2,hr,1);

  //int ippiMirror_8u_C1IR_daf(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip){
  r = ippiMirror_8u_C1IR_daf(src_offst,step,roi,ippAxsHorizontal);

  if(r<0){
    Output("Mirror_8u_C1R_test: ippiMirror_8u_C1R_daf failed\n");
    goto end;
  }

  print_img2("src after mirror:",&src_offst,step,wr/2,hr,1);
  
  //print_img("ipp",&src_offst,w,h,1);

  //print_img2("mirror_ipp",&dst_ipp,dstStep,wr,hr,1);
  //ippiMirror_8u_C1R_daf(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);


  
  //print_img2("mirror_ffmpeg",&dst_ffmpeg,dstStep,wr,hr,1);

  *ncc_val = 0;//ncc2(dst_ipp,dstStep,dst_ffmpeg,dstStep,w,h);
  
 end:
  if(pSrc[0])
    free(pSrc[0]);
  if(dst_ipp)
    free(dst_ipp);
  if(dst_ffmpeg)
    free(dst_ffmpeg);
  return r;
}
