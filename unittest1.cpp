#ifdef windows
#include "stdafx.h"
#endif
//#include "CppUnitTest.h"
extern "C"
{
#include <ipp.h>
#include "ippcc.h"
#include "ippi.h"
}
//#include "media_io.h"
//#include <windows.h>
//#include <random>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/intreadwrite.h>
}
//#include <../um/debugapi.h>

/*
int sws_scale 	( 	struct SwsContext *  	c,
const uint8_t *const  	srcSlice[],
const int  	srcStride[],
int  	srcSliceY,
int  	srcSliceH,
uint8_t *const  	dst[],
const int  	dstStride[]
)
*/
#ifdef windows
void Output(const char* szFormat, ...)
{
	char szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnprintf(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}
#else
#define Output printf
#endif

void print_img(char* name, uint8_t* img[], int w, int h, int c) {

  Output("Printing img '%s' (%p):\n",name, img);
  for (int ci = 0; ci<c; ci++) {
    if (!img[ci]) {
      Output("Channel %d is NULL\n", ci);
    }
    else {
      Output("Channel %d:\n", ci);
      for (int hi = 0; hi < h; hi++) {
	for (int wi = 0; wi < w; wi++) {
	  Output("%u ", img[ci][wi + hi*w]);
	}
	Output("\n");
			}
    }
  }
  return;
}

void l2_dist_img(double distances[], uint8_t* img1[],uint8_t* img2[], int w, int h, int c) {

  for (int ci = 0; ci<c; ci++) {
    if ( (!img1[ci] && img2[ci]) || (img1[ci] && !img2[ci]) ) {
      Output("ERROR: channel %d: img1[ci]=%p img2[ci]=%p\n", ci,img1[ci],img2[ci]);
      return;
    }
  }
 
  for (int ci = 0; ci<c; ci++) {
    if (!img1[ci]) {
      Output("Channel %d is NULL\n", ci);
    }
    else {

      unsigned int s = 0;
      distances[ci]=0;
      for (int hi = 0; hi < h; hi++) {
	for (int wi = 0; wi < w; wi++) {
	  unsigned int m1 = (unsigned int)img1[ci][wi + hi*w];
	  unsigned int m2 = (unsigned int)img2[ci][wi + hi*w]; 
	  s += (m1-m2)*(m1-m2);
	}	
      }
      distances[ci] = sqrt(s);
    }
  }
  return;
}


void fill_img(uint8_t* img[],int w,int h,int c,char with_rand){

  for(int ci=0;ci<c;ci++){
    if(img[ci]){
      for(int i=0;i<w*h;i++){
	img[ci][i] = with_rand ? (uint8_t)rand() : (uint8_t)i;
      }
    }
  }
  return;
}



void RGBtoYCbCr_ipp(unsigned char* src[3] , unsigned char *dst[] , int width,int height)
{
       
  IppiSize Roi;
  Rect.height = Roi.height = height;
  Rect.width = Roi.width = width;
  
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

void planar_to_packed(unsigned char* src[] , unsigned char *dst ,int w,int h, int c){

  int j =0;
  for(int i=0;i<w*h;i++)
    for(int ic=0;ic<c;ic++)
      if(src[ic])
	dst[j++] = src[ic][i];
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

//ippiCopy_8u_C1R( src,w,dst_ipp,w,roi);
void ippiCopy_8u_C1R_ffmpeg(unsigned char* src , int ws, unsigned char *dst ,int wd, IppiSize roi){
  /*
  int av_image_copy_to_buffer 	( 	uint8_t *  	dst,
		int  	dst_size,
		const uint8_t *const  	src_data[4],
		const int  	src_linesize[4],
		enum AVPixelFormat  	pix_fmt,
		int  	width,
		int  	height,
		int  	align 
	) 		
  */
  
}
  
void test_ippiCopy_8u_C1R_replacement(){

  IppiSize roi;

  int w = 12;
  int h = 12;
  int wr = 6;
  int hr = 6;
    
  unsigned char* src;
  unsigned char *dst_ipp;

  src = (uint8_t*)malloc(h*w);
    
  fill_img(&src, w, h, 1,1);
  print_img("src",&src, w, h, 1);
  
  dst_ipp = (uint8_t*)malloc(h*w);
  memset(dst_ipp,0,h*w);

  roi.height    = hr;
  roi.width     = wr;
  
  
  ippiCopy_8u_C1R( src,w,dst_ipp,w,roi);
  print_img("dst_ipp",&dst_ipp, w, h, 1);
  
    return;
}

#define W 24
#define H 24
int main() {

  srand(time(NULL));
  //test_ippiRGBToYCbCr_8u_P3R_replacement();
  test_ippiCopy_8u_C1R_replacement();
  /*
	int w = 3;
	int h = 3;
	uint8_t* rgb24Data = (uint8_t*)malloc(3 * h*w);//3 is because every pixel is reperesented by 3 bytes
	fill_img(&rgb24Data, w, h, 1);
	print_img(&rgb24Data, w, h, 1);

	uint8_t* dst[3];// = (uint8_t* const)malloc(3 * imgHeight*imgWidth);
	SwsContext * ctx;
	dst[0] = (uint8_t* const)malloc(h*w);
	dst[1] = (uint8_t* const)malloc(h*w);
	dst[2] = (uint8_t* const)malloc(h*w);
	fill_img(dst, w, h, 3);
	print_img(dst, w, h, 3);

	int dst_stride[3] = { w, w, w };
	
	ctx = sws_getContext(w, h,AV_PIX_FMT_RGB24, w, h,AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);
	if (!ctx) {

		Output("ctx is null\n");
		return 1;
	}
	uint8_t * inData[1] = { rgb24Data }; // RGB24 have one plane
	int inLinesize[1] = { 3 * w }; // RGB stride
	//
	//int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
	//const int srcStride[], int srcSliceY, int srcSliceH,
	//uint8_t *const dst[], const int dstStride[]);
	//
	
	sws_scale(ctx, &rgb24Data, inLinesize, 0, h, dst, dst_stride);
	print_img(dst, w, h, 3);
	Output("hjghjghjg\n");

	//////////////////////////////////////////////////////
	
	uint8_t *rgb_data = (uint8_t *) malloc(W*H * 4);
	uint8_t *rgb_src3[3] = { rgb_data, NULL, NULL };
	int rgb_stride3[3] = { 4 * W, 0, 0 };
	uint8_t *rgb_src1[1] = { rgb_data };
	int rgb_stride1[1] = { 4*W };


	uint8_t *data = (uint8_t *) malloc(3 * W*H);
	uint8_t *dst3[3] = { data, data + W*H, data + W*H * 2 };
	int stride[3] = { W, W, W };
	
	
	struct SwsContext *sws;
	sws = sws_getContext(W , H , AV_PIX_FMT_RGB32, W, H, AV_PIX_FMT_YUV420P, 2, NULL, NULL, NULL);

	srand(0);
	for (int y = 0; y<H; y++) {
		for (int x = 0; x<W * 4; x++) {
			rgb_data[x + y * 4 * W] = rand();
		}
	}
	sws_scale(sws, rgb_src3, rgb_stride3, 0, H, dst3, stride);
	
	print_img(dst, W, H, 3);

	sws_scale(sws, rgb_src1, rgb_stride1, 0, H, dst3, stride);
	print_img(dst, W, H, 3);
	

	//selfTest(src, stride, W, H);
	/////////////////////////////////////////////////////////

	*/
  return 0;

}
