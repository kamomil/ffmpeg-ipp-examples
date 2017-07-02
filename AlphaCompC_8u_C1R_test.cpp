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
  
  double xfactor, yfactor;
  int interpolation;

  int w = frame->width;
  int h = frame->height;
  
  dstSize.width = srcSize.width = w;
  dstSize.height = srcSize.height = h;
  
  int extra_add_to_dst_width = 0;//rand_in_range(1,100);
  int extra_add_to_src_width = 0;//rand_in_range(1,100);

  
  int srcStep = w+extra_add_to_src_width;
  int dstStep = dstSize.width+extra_add_to_dst_width;
  char comp_filename[256] = {0};
  char rect_resized_filename[256] = {0};

  //pSrc and src_stride will be filled by convert_to_GRAY8
  unsigned char* pSrc[4]={NULL,NULL,NULL,NULL};
  int src_stride[4]={0,0,0,0};

  //this allocates pSrc
  int r = convert_to_GRAY8(frame, pSrc , src_stride,extra_add_to_src_width);

  unsigned char *dst_ipp =    (unsigned char*)malloc(h*dstStep);
  unsigned char *dst_ffmpeg = (unsigned char*)malloc(h*dstStep);

  IppStatus st = ippStsNoErr;

  //unsigned char* src_offst = pSrc[0]+(srcRect.y*srcStep) + srcRect.x;

  unsigned char* watermark = (unsigned char*)malloc(10000);

  int sz = 0;

  int water_w = 72;
  int water_h = 74;

  FILE* f;

  roi.width =  72;//rand_in_range(water_w*0.8,water_w);
  roi.height = 74;//rand_in_range(water_h*0.8,water_h);
  
  if(r<0){
    Output("error: test_ippiAlphaCompC_8u_C1R_replacement: convert_to_GRAY8 failed\n");
    goto end;
  }

  if(!dst_ipp || !dst_ffmpeg || !watermark){
    Output("error: test_ippiAlphaCompC_8u_C1R_replacement: allocation failed\n");
    r = -1;
    goto end;
      
  }

  f = fopen("small_apple72x74_gray.yuv", "rb");
  if (!f) {
    fprintf(stderr, "Could not open %s\n", "small_apple72x74_gray.yuv");
    exit(1);
  }

  
  sz = fread(watermark, 1, 100000, f);
  
  pgm_save(pSrc[0], src_stride[0], src_stride[0], h,"alph_src1.yuv");

  
  
  memset(dst_ipp,0,h*dstStep);
  memset(dst_ffmpeg,0,h*dstStep);

  //roi.height    = hr;
  //roi.width     = wr;

  //Alpha plus = 48
  //Alpha over = 64
  st = ippiAlphaCompC_8u_C1R(watermark,water_w,48,pSrc[0], srcStep, 255, pSrc[0], dstStep, roi, ippAlphaPlus);

  snprintf(comp_filename, sizeof(comp_filename), "comp%02d.yuv", idx);
  pgm_save(pSrc[0], dstStep, w, h,comp_filename);
  Output("roi size is %dx%d\n",roi.width,roi.height);


    

  //(const Ipp<datatype>* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp<datatype>* pDst, int dstStep, IppiSize dstRoiSize, double xFactor, double yFactor, int interpolation);
  
  //Output("rect_resize size is %dx%d\n",dstSize.width, dstSize.height);

  /*
  IppStatus ippiAlphaCompC_<mod>(const Ipp<datatype>* pSrc1, int src1Step, Ipp<datatype> alpha1, const Ipp<datatype>* pSrc2, int src2Step, Ipp<datatype> alpha2, Ipp<datatype>* pDst, int dstStep, IppiSize roiSize, IppiAlphaType alphaType);
  */
 
#ifdef WIN32
  st = ippiResize_8u_C1R( pSrc[0],srcSize,srcStep,srcRect,dst_ipp,dstStep,dstSize,xfactor,yfactor,interpolation);
  if(st != ippStsNoErr){
    Output("AlphaCompC_8u_C1R_test: st IS ERROR: %d\n",st);
    r = -1;
    goto end;
  } 		  
#endif  

  //r = ippiAlphaCompC_8u_C1R_daf(...)
  /*
  if(r<0){
    Output("Resize_8u_C1R_test: ippiMirror_8u_C1R_daf failed\n");
    goto end;
  }
  */
  
  //snprintf(rect_resized_filename, sizeof(rect_resized_filename), "rect_resized%02d.yuv", idx);
  //pgm_save(dst_ffmpeg, dstStep, dstSize.width, dstSize.height,rect_resized_filename);
  
  
  //print_img("ffmpeg",&(pSrc[0]),w,h,1);
  //print_img2("ipp",&src_offst,srcStep,wr,hr,1);
  //print_img2("resize_ffmpeg",&(pSrc[0]),srcStep,srcRect.width,srcRect.height,1);
  //ippiMirror_8u_C1R_daf(pSrc[0]+(offset_h*srcStep)+offset_w,srcStep,dst_ffmpeg,dstStep,roi);

  //print_img2("resize_ffmpeg",&dst_ffmpeg,dstStep,wr,hr,1);

  //*ncc_val = ncc2(dst_ipp,dstStep,dst_ffmpeg,dstStep,w,h);
  
 end:
  if(pSrc[0])
    free(pSrc[0]);
  if(dst_ipp)
    free(dst_ipp);
  if(dst_ffmpeg)
    free(dst_ffmpeg);
  if(watermark)
    free(watermark);
  exit(1);
  return r;
  
}
