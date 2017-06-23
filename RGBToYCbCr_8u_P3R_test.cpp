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

int ippiRGBToYCbCr_8u_P3R_ffmpeg(const Ipp8u* const pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize){

  int w = roiSize.width;
  int h = roiSize.height;
  
  //                               G         B      R
  const unsigned char *gbrp[4] = {pSrc[1],pSrc[2],pSrc[0],NULL};

  const int gbrp_stride[4] = { srcStep, srcStep, srcStep, 0};
  int dst_ffmpeg_stride[4] = { dstStep, dstStep, dstStep, 0};

  //AV_PIX_FMT_YUV444P - //planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) 
  SwsContext * ctx = sws_getContext(w, h,AV_PIX_FMT_GBRP, w, h,AV_PIX_FMT_YUV444P, 0, NULL, NULL, NULL);
  if (!ctx) {
    Output("ctx is null\n");
    return -1;
  }
  
  int r = sws_scale(ctx, gbrp, gbrp_stride, 0, h, pDst, dst_ffmpeg_stride);
  if(r != h){
    Output("ippiRGBToYCbCr_8u_P3R_ffmpeg: error: sws_scale returned %d and h is %d\n",r,h);
    return -1;
  }
  return r;	
}

int test_ippiRGBToYCbCr_8u_P3R_replacement(AVFrame *frame, double ncc_plain_val[3]){

  int w = frame->width;
  int h = frame->height;

  unsigned char *gbrp[4] ={NULL,NULL,NULL,NULL};
  int gbrp_stride[4];

  //int r;
  //Output(" w=%d h=%d f->format=%d f->data=%p f->linesize=%p\n",frame->width,frame->height,frame->format,frame->data,frame->linesize);
  int r = convert_to_GBRP(frame,gbrp ,gbrp_stride);

  if(r<0){
    Output("test_ippiRGBToYCbCr_8u_P3R_replacement: convert_to_GBRP failed\n");
    return -1;
  }

  const unsigned char *rgb_planar[4] =  {gbrp[2],gbrp[0],gbrp[1],NULL};
  int rgb_planar_stride[4] = {gbrp_stride[2],gbrp_stride[0],gbrp_stride[1],0};

  IppiSize roiSize;
  roiSize.width = w;
  roiSize.height = h;

  unsigned char *pDst_ipp[4];
  
  pDst_ipp[0] = (unsigned char *)malloc(w*h);
  pDst_ipp[1] = (unsigned char *)malloc(w*h);
  pDst_ipp[2] = (unsigned char *)malloc(w*h);
  pDst_ipp[3] = NULL;
  
  if(!pDst_ipp[0] || !pDst_ipp[1] || !pDst_ipp[2]){
    Output("test_ippiRGBToYCbCr_8u_P3R_replacement: allocation failure\n");
    for(int i=0;i<3;i++){
      if(pDst_ipp[i])
	free(pDst_ipp[i]);
      if(gbrp[i])
	free(gbrp[i]);
    }
    return -1;
  }

  //IppStatus ippiRGBToYCbCr_8u_P3R(const Ipp8u* const pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize);

  IppStatus s = ippiRGBToYCbCr_8u_P3R(rgb_planar, rgb_planar_stride[0], pDst_ipp, w , roiSize);
  if(s != ippStsNoErr){
    Output("test_ippiRGBToYCbCr_8u_P3R_replacement: ippiRGBToYCbCr_8u_P3R failure\n");
    for(int i=0;i<3;i++)free(pDst_ipp[i]);
    return -1;
  }

  unsigned char *pDst_ffmpeg[4];
  
  pDst_ffmpeg[0] = (unsigned char *)malloc(w*h);
  pDst_ffmpeg[1] = (unsigned char *)malloc(w*h);
  pDst_ffmpeg[2] = (unsigned char *)malloc(w*h);
  pDst_ffmpeg[3] = NULL;
  
  if(!pDst_ffmpeg[0] || !pDst_ffmpeg[1] || !pDst_ffmpeg[2]){
    Output("test_ippiRGBToYCbCr_8u_P3R_replacement: allocation failure\n");
    for(int i=0;i<3;i++){
      if(pDst_ipp[i])
	free(pDst_ipp[i]);
      if(pDst_ffmpeg[i])
	free(pDst_ffmpeg[i]);
      if(gbrp[i])
	free(gbrp[i]);
    }
    return -1;
  }


  r = ippiRGBToYCbCr_8u_P3R_ffmpeg(rgb_planar, rgb_planar_stride[0], pDst_ffmpeg, w , roiSize);

  if(r<0){
    Output("test_ippiRGBToYCbCr_8u_P3R_replacement: ippiRGBToYCbCr_8u_P3R_ffmpeg failure\n");
    for(int i=0;i<3;i++){
      if(pDst_ipp[i])
	free(pDst_ipp[i]);
      if(pDst_ffmpeg[i])
	free(pDst_ffmpeg[i]);
      if(gbrp[i])
	free(gbrp[i]);
    }
    return -1;
  }

  for(int i=0;i<3;i++){
    ncc_plain_val[i] =  ncc(pDst_ffmpeg[i],pDst_ipp[i],w*h);
    if(pDst_ipp[i])
      free(pDst_ipp[i]);
    if(pDst_ffmpeg[i])
      free(pDst_ffmpeg[i]);
    if(gbrp[i])
      free(gbrp[i]);
  }
  return r;
}
