//#ifdef WIN32
#include "stdafx.h"
//#endif

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
#include "RGBToYCbCr_8u_P3R_test.h"

  
void RGBtoYCbCr_ipp(unsigned char* src[3] , unsigned char *dst[] , int width,int height)
{
       
  IppiSize Roi;
  Roi.height = height;
  Roi.width = width;
  
  //this is important in order to avoid a compilation error:
  // error: invalid conversion from ‘unsigned char**’ to ‘const Ipp8u** {aka const unsigned char**}’ [-fpermissive]
  //I think it's because src is an array so it is a const by def
  //https://stackoverflow.com/questions/2220916/why-isnt-it-legal-to-convert-pointer-to-pointer-to-non-const-to-a-pointer-to
  const unsigned char *source1[3];
  source1[0] = src[0];
  source1[1] = src[1];
  source1[2] = src[2];

  //IppStatus ippiYCbCrToRGB_<mod>(const Ipp8u* const pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize);
  //this is 4:4:4 - that is no subsmapling so the 444 is ommited //see https://software.intel.com/en-us/node/503874
  IppStatus s = ippiRGBToYCbCr_8u_P3R(source1, width,
				      dst,
				      width,
				      Roi);
  
  if(s){
    Output("ippiRGBToYCbCr_8u_P3R failed with error: %d\n",s);
  }	
}

void RGBtoYCbCr_ffmpeg(unsigned char* src[] , unsigned char *dst[] ,int w, int h)
{
  int dst_ffmpeg_stride[3] = { w, w, w };

  unsigned char* src_packed = (unsigned char*)malloc(w*h*3);
  if(!src_packed){
    Output("failure allocating src_packed\n");
    return;
  }
  planar_to_packed(src,src_packed,w,h,3);
  //print_img("src_packed",&src_packed,3*w,h,1);

  //AV_PIX_FMT_YUV444P - //planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) 
  SwsContext * ctx = sws_getContext(w, h,AV_PIX_FMT_RGB24, w, h,AV_PIX_FMT_YUV444P, 0, NULL, NULL, NULL);
  if (!ctx) {
    
    Output("ctx is null\n");
    return;
  }
  
  int src_strides[1] = { 3*w}; // RGB stride
  //
  //int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
  //const int srcStride[], int srcSliceY, int srcSliceH,
  //uint8_t *const dst_ffmpeg[], const int dstStride[]);
  //
  int ttt = sws_scale(ctx, &src_packed, src_strides, 0, h, dst, dst_ffmpeg_stride);
  
  Output("sws_scale returned %d\n",ttt);
  	
}

int test_ippiRGBToYCbCr_8u_P3R_replacement(){

  ////                                     //dst                                      //src                  
  //void RGBtoYCbCr_ipp(unsigned char *m_pucWorkingBuffer[3], unsigned char *m_pucUnpaddedReconstruction[3] ,int height, int width)
  
  int w = rand() % 100;
  int h = rand() % 100;
  unsigned char *src[3];
  unsigned char *dst_ipp[3];

  src[0] = (uint8_t*)malloc(h*w);
  src[1] = (uint8_t*)malloc(h*w);
  src[2] = (uint8_t*)malloc(h*w);
  
  fill_img(src, w, h, 3,1);
  //print_img("src",src, w, h, 3);
  
  dst_ipp[0] = (uint8_t*)malloc(h*w);
  dst_ipp[1] = (uint8_t*)malloc(h*w);
  dst_ipp[2] = (uint8_t*)malloc(h*w);

  memset(dst_ipp[0],0,h*w);
  memset(dst_ipp[1],0,h*w);
  memset(dst_ipp[2],0,h*w);

  ////////////////////////
  RGBtoYCbCr_ipp(src,dst_ipp,w,h);
  ///////////////////////
  
  //print_img("dst_ipp",dst_ipp, w, h, 1);
  
  uint8_t* dst_ffmpeg[3];// = (uint8_t* const)malloc(3 * imgHeight*imgWidth);
  dst_ffmpeg[0] = (uint8_t* const)malloc(h*w);
  dst_ffmpeg[1] = (uint8_t* const)malloc(h*w);
  dst_ffmpeg[2] = (uint8_t* const)malloc(h*w);

  memset(dst_ffmpeg[0],0,h*w);
  memset(dst_ffmpeg[1],0,h*w);
  memset(dst_ffmpeg[2],0,h*w);

  RGBtoYCbCr_ffmpeg(src, dst_ffmpeg , w,  h);
  //print_img("dst_ffmpeg",dst_ffmpeg, w, h, 3);

  double d[3] = {0,0,0};
  l2_dist_img(d,dst_ipp,dst_ffmpeg,w,h,3);
  Output("distances are %f %f % for %d X %d\n",d[0],d[1],d[2],w,h);
    
  return 0;
}
