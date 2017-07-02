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
#endif

//#include <random>
#include <stdlib.h>
//#include <stdio.h>
#include <time.h>
#include <math.h>
#include "test_utils.h"
#include "AlphaCompC_8u_C1R_test.h"


/*
  IppStatus ippiAlphaCompC_<mod>(const Ipp<datatype>* pSrc1, int src1Step, Ipp<datatype> alpha1, const Ipp<datatype>* pSrc2, int src2Step, Ipp<datatype> alpha2, Ipp<datatype>* pDst, int dstStep, IppiSize roiSize, IppiAlphaType alphaType);
  */

int ippiAlphaCompC_8u_C1R_daf(const Ipp8u* pSrc1, int src1Step, Ipp8u alpha1, const Ipp8u pSrc2, int src2Step, Ipp8u alpha2, Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiAlphaType alphaType){

  return 0;
}



//void test_ippiResize_8u_C1R_replacement(unsigned char* src,int w,int h){

int test_ippiAlphaCompC_8u_C1R_replacement(AVFrame *frame, double* ncc_val,int idx){

  IppiSize roi;

  IppiSize srcSize,dstSize;
  IppiRect srcRect;
  double xfactor, yfactor;
  int interpolation;

  int w = frame->width;
  int h = frame->height;
  
  srcSize.width = w;
  srcSize.height = h;
  
  int extra_add_to_dst_width = rand_in_range(1,100);
  int extra_add_to_src_width = rand_in_range(1,100);

  srcRect.x = rand_in_range(0,w/2);
  srcRect.y = rand_in_range(0,h/h);
  
  srcRect.width  = rand_in_range(FFMIN(40,w - srcRect.x),w - srcRect.x);
  srcRect.height = rand_in_range(FFMIN(40,h - srcRect.y),w - srcRect.y);

  xfactor = rand_in_range_double(0.1, 1.5);
  yfactor = rand_in_range_double(0.1, 1.5);
  interpolation = (xfactor >= 1.0 || yfactor >= 1.0) ? IPPI_INTER_LINEAR : IPPI_INTER_SUPER;
  
  dstSize.width = (int)(xfactor * srcRect.width);
  dstSize.height = (int)(yfactor * srcRect.height);
  
  int srcStep = w+extra_add_to_src_width;
  int dstStep = dstSize.width+extra_add_to_dst_width;
  char rect_filename[256] = {0};
  char rect_resized_filename[256] = {0};

  //pSrc and src_stride will be filled by convert_to_GRAY8
  unsigned char* pSrc[4]={NULL,NULL,NULL,NULL};
  int src_stride[4]={0,0,0,0};

  //this allocates pSrc
  int r = convert_to_GRAY8(frame, pSrc , src_stride,extra_add_to_src_width);

  unsigned char *dst_ipp =    (unsigned char*)malloc(h*dstStep);
  unsigned char *dst_ffmpeg = (unsigned char*)malloc(h*dstStep);

  IppStatus st = ippStsNoErr;

  unsigned char* src_offst = pSrc[0]+(srcRect.y*srcStep) + srcRect.x;
    
  if(r<0){
    Output("error: test_ippiResize_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    goto end;
  }

  if(!dst_ipp || !dst_ffmpeg){
    Output("error: test_ippiResize_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
      
  }


  pgm_save(pSrc[0], src_stride[0], w, h,"orig.yuv");

  
  snprintf(rect_filename, sizeof(rect_filename), "rect%02d.yuv", idx);
  pgm_save(src_offst, src_stride[0], srcRect.width, srcRect.height,rect_filename);
  Output("rect size is %dx%d\n",srcRect.width,srcRect.height);
  
  memset(dst_ipp,0,h*dstStep);
  memset(dst_ffmpeg,0,h*dstStep);

  //roi.height    = hr;
  //roi.width     = wr;
  

  //(const Ipp<datatype>* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp<datatype>* pDst, int dstStep, IppiSize dstRoiSize, double xFactor, double yFactor, int interpolation);
  
  Output("rect_resize size is %dx%d\n",dstSize.width, dstSize.height);

  /*
  IppStatus ippiAlphaCompC_<mod>(const Ipp<datatype>* pSrc1, int src1Step, Ipp<datatype> alpha1, const Ipp<datatype>* pSrc2, int src2Step, Ipp<datatype> alpha2, Ipp<datatype>* pDst, int dstStep, IppiSize roiSize, IppiAlphaType alphaType);
  */
#ifdef WIN32
  st = ippiResize_8u_C1R( pSrc[0],srcSize,srcStep,srcRect,dst_ipp,dstStep,dstSize,xfactor,yfactor,interpolation);
  if(st != ippStsNoErr){
    Output("Resize_8u_C1R_test: st IS ERROR: %d\n",st);
    r = -1;
    goto end;
  } 		  
#endif  

  //r = ippiAlphaCompC_8u_C1R_daf(...)

  if(r<0){
    Output("Resize_8u_C1R_test: ippiMirror_8u_C1R_daf failed\n");
    goto end;
  }

  snprintf(rect_resized_filename, sizeof(rect_resized_filename), "rect_resized%02d.yuv", idx);
  pgm_save(dst_ffmpeg, dstStep, dstSize.width, dstSize.height,rect_resized_filename);
  
  
  //print_img("ffmpeg",&(pSrc[0]),w,h,1);
  //print_img2("ipp",&src_offst,srcStep,wr,hr,1);
  //print_img2("resize_ffmpeg",&(pSrc[0]),srcStep,srcRect.width,srcRect.height,1);
  //ippiMirror_8u_C1R_daf(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);

  //print_img2("resize_ffmpeg",&dst_ffmpeg,dstStep,wr,hr,1);

  *ncc_val = ncc2(dst_ipp,dstStep,dst_ffmpeg,dstStep,w,h);
  
 end:
  if(pSrc[0])
    free(pSrc[0]);
  if(dst_ipp)
    free(dst_ipp);
  if(dst_ffmpeg)
    free(dst_ffmpeg);
  exit(1);
  return r;
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /*
  IppiSize srcSize,dstSize;
  IppiRect srcRect;
  unsigned char *dst_ipp, *dst_ffmpeg;
  double xfactor, yfactor;
  int interpolation;

  srcSize.width = w;
  srcSize.height = h;

  if (w < 30 || h < 30) {
	  Output("warning: width or height too small\n");
	  return;
  }
  do {
	  srcRect.x = rand_in_range(w / 3, w - 1);
	  srcRect.y = rand_in_range(h / 3, h - 1);

	  srcRect.width = rand_in_range((w - srcRect.x) / 3, w - srcRect.x);
	  srcRect.height = rand_in_range((h - srcRect.y) / 3, h - srcRect.y);

	  xfactor = rand_in_range_double(0.1, 1.5);
	  yfactor = rand_in_range_double(0.1, 1.5);
	  interpolation = (xfactor >= 1.0 || yfactor >= 1.0) ? IPPI_INTER_LINEAR : IPPI_INTER_SUPER;

	  dstSize.width = (int)(xfactor * srcRect.width);
	  dstSize.height = (int)(yfactor * srcRect.height);
  }
  while (srcRect.x < 3 || srcRect.y < 3 || srcRect.width < 3 || srcRect.height < 3 || dstSize.width < 3 || dstSize.height < 3);


  dst_ipp = (uint8_t*)malloc(dstSize.height * dstSize.width);
  memset(dst_ipp,0, dstSize.height * dstSize.width);
  
  //                                         1              2            3                 4              5         6        7        7                   8              9            10   
  //IppStatus ippiResize_8u_C1R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, double xFactor, double yFactor, int interpolation);
  //
#ifdef WIN32
  ippiResize_8u_C1R( src,    srcSize,w,srcRect,
	                 dst_ipp,dstSize.width,dstSize,
	                 xfactor,yfactor,interpolation);
    				 
 // print_img("dst_ipp",&dst_ipp, dstSize.width, dstSize.height, 1);
#endif  
  dst_ffmpeg = (uint8_t*)malloc(dstSize.height * dstSize.width+100);
  memset(dst_ffmpeg,0, dstSize.height * dstSize.width);

  ippiResize_8u_C1R_ffmpeg(src, srcSize, w, srcRect,
	  dst_ffmpeg, dstSize.width, dstSize,
	  xfactor, yfactor, interpolation);
	  
  //print_img("dst_ffmpeg", &dst_ffmpeg, dstSize.width, dstSize.height, 1);
  double n  = ncc(dst_ipp,dst_ffmpeg,dstSize.width*dstSize.height);
  int x = 1;
  //double n  = ncc(dst_ipp,dst_ffmpeg,dstSize.width*dstSize.height);
  //Output("ncc is %d\n",n*1000);
  //exit(1);
  return;
  */
}
