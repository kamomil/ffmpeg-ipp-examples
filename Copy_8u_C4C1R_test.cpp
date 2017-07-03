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

#include "Copy_8u_C3P3R_test.h"


//this version is wrong , it has a bug in case of a negative steps

//copy one chanlle out of four to dst.
//pSrc should be offseted to the channle we want to copy
int ippiCopy_8u_C4C1R_daf(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize){

  for(;roiSize.height;roiSize.height--){
    const unsigned char* row_end = pSrc+(4*roiSize.width);
    const unsigned char* sptr = pSrc;
    unsigned char* dptr = pDst;

    while(sptr<row_end){
      *dptr = *sptr;
      dptr++;
      sptr+=4;
    }

    pSrc +=srcStep;
    pDst +=dstStep;  
  }
  return 0;
    
}
//TODO - NOT yet implemented
int test_ippiCopy_8u_C4C1R_replacement(AVFrame *frame, double ncc_plain_val[3]){

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
  
  int srcStep = 3*w+extra_add_to_src_width;
  int dstStep = w+extra_add_to_dst_width;
  
  
  //this allocates pSrc
  int r = 0;//convert_to_RGBA(frame, pSrc , src_stride,extra_add_to_src_width);//TODO
  
  if(r<0){
    Output("error: test_ippiCopy_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    if(pSrc[0])
      free(pSrc[0]);
    return -1;
  }
  
  unsigned char *dst_ipp = NULL;
  dst_ipp = (unsigned char*)malloc(h*dstStep);
  
  unsigned char *dst_daf = NULL;
  dst_daf = (unsigned char*)malloc(h*dstStep);
  
  unsigned char* src = NULL;
  unsigned char* src_end = NULL;

  if(!dst_ipp || !dst_daf){
    Output("error: test_ippiCopy_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
  }
  
  memset(dst_ipp,0,h*dstStep);
  memset(dst_daf,0,h*dstStep);

  roi.height    = hr;
  roi.width     = wr;

  //ippiCopy_<mod>(const Ipp<datatype>* pSrc, int srcStep, Ipp<datatype>* const pDst[3], int dstStep, IppiSize roiSize);
    
  //ippiCopy_8u_C4C1R    (pSrc[0]+(offset_h*srcStep)+3*offset_w,srcStep,dst_ipp,dstStep,roi);
  //ippiCopy_8u_C4C1R_daf(pSrc[0]+(offset_h*srcStep)+3*offset_w,srcStep,dst_daf,dstStep,roi);

  src = pSrc[0]+(offset_h*srcStep)+3*offset_w;
  src_end = src+srcStep*(roi.height-1);
  
  r = ippiCopy_8u_C4C1R(src_end,0-srcStep,dst_ipp,dstStep,roi);
  if(r != ippStsNoErr){
    Output("error:  ippiCopy_8u_C3P3R failed with val %d\n",r);
    r = -1;
    goto end;
  }
  r = ippiCopy_8u_C4C1R_daf(src_end,0-srcStep,dst_daf,dstStep,roi);

  if(r < 0){
    Output("error:  ippiCopy_8u_C3P3R_daf failed with val\n");
    r = -1;
    goto end;
  }

  //print_img("copy_ipp",dst_ipp,10,10,1);
  //
  //print_img("copy_daf",dst_daf,10,10,1);
  *ncc_plain_val = ncc2(dst_ipp,dstStep,dst_daf,dstStep,w,h);



 end:
  if(pSrc)
    free(pSrc);
  if(dst_ipp)
    free(dst_ipp);  
  if(dst_daf)
    free(dst_daf);  
  return r;
}
