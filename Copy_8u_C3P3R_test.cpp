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
int ippiCopy_8u_C3P3R_daf_buggy(const Ipp8u* pSrc, int srcStep, Ipp8u* const pDst[3], int dstStep, IppiSize roiSize){

  const unsigned char* sptr = pSrc;
  unsigned char* dptr0 = pDst[0];
  unsigned char* dptr1 = pDst[1];
  unsigned char* dptr2 = pDst[2];
  int src_extra = srcStep-(3*roiSize.width);
  int dst_extra = dstStep-roiSize.width;
    
  for(;roiSize.height;roiSize.height--){
    const unsigned char* row_end = sptr+(3*roiSize.width);
    while(sptr<row_end){
      *(dptr0++) = *(sptr++);
      *(dptr1++) = *(sptr++);
      *(dptr2++) = *(sptr++);
    }
    sptr  += src_extra;
    dptr0 += dst_extra;
    dptr1 += dst_extra;
    dptr2 += dst_extra;
    
  }
  return 0;  
}

int ippiCopy_8u_C3P3R_daf(const Ipp8u* pSrc, int srcStep, Ipp8u* const pDst[3], int dstStep, IppiSize roiSize){

  const unsigned char* start_of_srow_ptr = pSrc;
  const unsigned char* sptr = pSrc;
  
  unsigned char* start_of_drow_ptr0 = pDst[0];
  unsigned char* start_of_drow_ptr1 = pDst[1];
  unsigned char* start_of_drow_ptr2 = pDst[2];
  
  unsigned char* dptr0 = pDst[0];
  unsigned char* dptr1 = pDst[1];
  unsigned char* dptr2 = pDst[2];
   
  for(;roiSize.height;roiSize.height--){
    const unsigned char* row_end = start_of_srow_ptr+(3*roiSize.width);
    while(sptr<row_end){
      *(dptr0++) = *(sptr++);
      *(dptr1++) = *(sptr++);
      *(dptr2++) = *(sptr++);
    }

    start_of_srow_ptr +=srcStep;

    start_of_drow_ptr0 += dstStep;
    start_of_drow_ptr1 += dstStep;
    start_of_drow_ptr2 += dstStep;

    
    sptr  = start_of_srow_ptr;
    dptr0 = start_of_drow_ptr0;
    dptr1 = start_of_drow_ptr1;
    dptr2 = start_of_drow_ptr2;
    
  }
  return 0;  
}

int test_ippiCopy_8u_C3P3R_replacement(AVFrame *frame, double ncc_plain_val[3]){

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
  int r = convert_to_RGB24(frame, pSrc , src_stride,extra_add_to_src_width);

  if(r<0){
    Output("error: test_ippiCopy_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    if(pSrc[0])
      free(pSrc[0]);
    return -1;
  }

  unsigned char *dst_ipp[3]={NULL,NULL,NULL};
  dst_ipp[0] = (unsigned char*)malloc(h*dstStep);
  dst_ipp[1] = (unsigned char*)malloc(h*dstStep);
  dst_ipp[2] = (unsigned char*)malloc(h*dstStep);

  unsigned char *dst_daf[3]={NULL,NULL,NULL};
  dst_daf[0] = (unsigned char*)malloc(h*dstStep);
  dst_daf[1] = (unsigned char*)malloc(h*dstStep);
  dst_daf[2] = (unsigned char*)malloc(h*dstStep);

  unsigned char* src = NULL;
  unsigned char* src_end = NULL;

  if(!dst_ipp[0] || !dst_ipp[1] || !dst_ipp[2] || !dst_daf[0] || !dst_daf[1] || !dst_daf[2]){
    Output("error: test_ippiCopy_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
  }
  
  memset(dst_ipp[0],0,h*dstStep);
  memset(dst_ipp[1],0,h*dstStep);
  memset(dst_ipp[2],0,h*dstStep);
  memset(dst_daf[0],0,h*dstStep);
  memset(dst_daf[1],0,h*dstStep);
  memset(dst_daf[2],0,h*dstStep);

  roi.height    = hr;
  roi.width     = wr;

  //ippiCopy_<mod>(const Ipp<datatype>* pSrc, int srcStep, Ipp<datatype>* const pDst[3], int dstStep, IppiSize roiSize);
    
  //ippiCopy_8u_C3P3R    (pSrc[0]+(offset_h*srcStep)+3*offset_w,srcStep,dst_ipp,dstStep,roi);
  //ippiCopy_8u_C3P3R_daf(pSrc[0]+(offset_h*srcStep)+3*offset_w,srcStep,dst_daf,dstStep,roi);

  src = pSrc[0]+(offset_h*srcStep)+3*offset_w;
  src_end = src+srcStep*(roi.height-1);
  
  r = ippiCopy_8u_C3P3R(src_end,0-srcStep,dst_ipp,dstStep,roi);
  if(r != ippStsNoErr){
    Output("error:  ippiCopy_8u_C3P3R failed with val %d\n",r);
    r = -1;
    goto end;
  }
  r = ippiCopy_8u_C3P3R_daf(src_end,0-srcStep,dst_daf,dstStep,roi);

  if(r < 0){
    Output("error:  ippiCopy_8u_C3P3R_daf failed with val\n");
    r = -1;
    goto end;
  }


  //print_img("copy_ipp",dst_ipp,10,10,1);
  //
  //print_img("copy_daf",dst_daf,10,10,1);
  ncc_plain_val[0] = ncc2(dst_ipp[0],dstStep,dst_daf[0],dstStep,w,h);
  ncc_plain_val[1] = ncc2(dst_ipp[1],dstStep,dst_daf[1],dstStep,w,h);
  ncc_plain_val[2] = ncc2(dst_ipp[2],dstStep,dst_daf[2],dstStep,w,h);

 end:
  if(pSrc[0])
    free(pSrc[0]);
  
  if(dst_ipp[0])
    free(dst_ipp[0]);   
  if(dst_ipp[1])
    free(dst_ipp[1]);
  if(dst_ipp[2])
    free(dst_ipp[2]);
  
  if(dst_daf[0])
    free(dst_daf[0]);
  if(dst_daf[1])
    free(dst_daf[1]);
  if(dst_daf[2])
    free(dst_daf[2]);
  return r;
}
