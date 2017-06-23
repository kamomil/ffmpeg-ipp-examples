#ifndef TEST_UTILS
#define TEST_UTILS

#ifdef WIN32
#include <windows.h>
#include <random>
#endif

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
#include "test_utils.h"

double rand_in_range_double(double min, double max) {
	double X= ((double)rand())/((double)RAND_MAX);//X is in [0,1]
	return (X*(max - min)) + min;
}

int rand_in_range(int min,int max){
  return rand() % (max + 1 - min) + min;
}

#ifdef WIN32
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


void print_img(char* name, unsigned char* img[], int w, int h, int c) {

  Output("Printing img '%s' (%p):\n",name, img);
  for (int ci = 0; ci<c; ci++) {
    if (!img[ci]) {
      Output("Channel %d is NULL\n", ci);
    }
    else {
      Output("Channel %d:\n", ci);
      for (int hi = 0; hi < h; hi++) {
	for (int wi = 0; wi < w; wi++) {
	  unsigned char p = img[ci][wi + hi*w];
	    Output("%u%s", p, p > 99 ? " " : (p > 10 ? "  " : "   "));
	}
	Output("\n");
			}
    }
  }
  return;
}

void l2_dist_img(double distances[], unsigned char* img1[],unsigned char* img2[], int w, int h, int c) {

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

double ncc2(const unsigned char* im1 ,int im1_linesz, const unsigned char* im2,int im2_linesz, int w,int h){

  int sz = h*w; 
  double* t1 = (double*)malloc(sizeof(double)*sz);
  double* t2 = (double*)malloc(sizeof(double)*sz);
  double mean1 = 0;
  double mean2 = 0;
  int i=0;
  for(int ih=0;ih<h;ih++){
    for(int iw=0;iw<w;iw++){
      //double X=(((double)rand()/(double)RAND_MAX)/100)-0.005;
      t1[i] = im1[ih*im1_linesz+iw]*1.0;//+X;
      mean1 += t1[i];
      t2[i] = im2[ih*im2_linesz+iw]*1.0;
      mean2 += t2[i];
      i++;
    }
    //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  }
  mean1 = mean1 / (1.0*sz);
  mean2 = mean2 / (1.0*sz);

  //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  //Output("ncc: sz = %d\n",sz);

  double norm1 = 0;
  double norm2 = 0;
  double n = 0;
  for(int i=0;i<sz;i++){
    t1[i] -= mean1;
    norm1 += t1[i]*t1[i];
    t2[i] -= mean2;
    norm2 += t2[i]*t2[i];
    n += t1[i]*t2[i];
    //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);
  }
  //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);

  if(norm1<0.01 || norm2<0.01){
    Output("nnc: WARNING: norms might be too small: norm1=%f norm2=%f\n",norm1,norm2);
  }
  
  free(t1);
  free(t2);
  return n/(sqrt(norm1*norm2));

}

double ncc(const unsigned char* im1 , const unsigned char* im2,int sz){

  double* t1 = (double*)malloc(sizeof(double)*sz);
  double* t2 = (double*)malloc(sizeof(double)*sz);
  double mean1 = 0;
  double mean2 = 0;
  for(int i=0;i<sz;i++){
    //double X=(((double)rand()/(double)RAND_MAX)/100)-0.005;
    t1[i] = im1[i]*1.0;//+X;
    mean1 += t1[i];
    t2[i] = im2[i]*1.0;
    mean2 += t2[i];
    //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  }
  mean1 = mean1 / (1.0*sz);
  mean2 = mean2 / (1.0*sz);

  //Output("mean1  = %f, mean2 = %f\n",mean1,mean2);
  //Output("ncc: sz = %d\n",sz);

  double norm1 = 0;
  double norm2 = 0;
  double n = 0;
  for(int i=0;i<sz;i++){    
    t1[i] -= mean1;
    norm1 += t1[i]*t1[i];
    t2[i] -= mean2;
    norm2 += t2[i]*t2[i];
    n += t1[i]*t2[i];
    //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);
  }
  //Output("n  = %f, norm1 = %f norm2 = %f\n",n,norm1,norm2);

  if(norm1<0.01 || norm2<0.01){
    Output("nnc: WARNING: norms might be too small: norm1=%f norm2=%f\n",norm1,norm2);
  }
  
  free(t1);
  free(t2);
  return n/(sqrt(norm1*norm2));
  
}


void fill_img(unsigned char* img[],int w,int h,int c,char with_rand){

  for(int ci=0;ci<c;ci++){
    if(img[ci]){
      for(int i=0;i<w*h;i++){
	img[ci][i] = with_rand ? (unsigned char)rand() : (unsigned char)i;
      }
    }
  }
  return;
}



void planar_to_packed(unsigned char* src[] , unsigned char *dst ,int w,int h, int c){

  int j =0;
  for(int ic=0;ic<c;ic++)
    if(!src[ic]){
      Output("planar_to_packed: %d plane is null\n",ic);
    }
  for(int i=0;i<w*h;i++)
    for(int ic=0;ic<c;ic++)
      if(src[ic])
	dst[j++] = src[ic][i];
}
void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,char *filename)
{
	FILE *f;
	int i;

	f = fopen(filename, "w");
	if (!f) {
		Output("failed opening %s\n",filename);
		return;
	}
	if (fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255) < 0) {
		Output("failed writing to %s\n", filename);
		return;
	}
	for (i = 0; i < ysize; i++) {
		int s = fwrite(buf + i * wrap, 1, xsize, f);
	}
	fclose(f);
}

int convert_to_format(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[], AVPixelFormat dst_format)
{
  //Output("error: convert_to_format failed: w=%d h=%d f->format=%d f->data=%p f->linesize=%p\n",f_in->width,f_in->height,f_in->format,f_in->data,f_in->linesize);
  
  SwsContext * ctx = sws_getContext(f_in->width, f_in->height,(AVPixelFormat)f_in->format,f_in->width, f_in->height ,dst_format, 0, NULL, NULL, NULL);
  if (!ctx) {
    Output("error: convert_to_format failed: ctx is null\n");
    return -1;
  }
  
  //Output("error: convert_to_format failed: w=%d h=%d f->format=%d f->data=%p f->linesize=%p\n",f_in->width,f_in->height,f_in->format,f_in->data,f_in->linesize);
  int r = sws_scale(ctx, f_in->data,f_in->linesize, 0, f_in->height, dst, dst_stride);

  if(r == 0){
    Output("error: convert_to_format failed\n");
    return -1;
  }
  return r;
}


int convert_to_GBRP(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[])
{

  int w = f_in->width;
  int h = f_in->height;
  
  AVPixelFormat dst_format;

  dst[0] = (unsigned char *)malloc(w*h+1000);
  dst[1] = (unsigned char *)malloc(w*h+1000);
  dst[2] = (unsigned char *)malloc(w*h+1000);
  dst[3] = NULL;
  
  if(!dst[0] || !dst[1] || !dst[2]){
    Output("convert_to_GBRP: allocation failure\n");
    return -1;
  }
  dst_stride[0] =  dst_stride[1] = dst_stride[2] = w;
  dst_stride[3] = 0;
  
  return convert_to_format(f_in, dst ,dst_stride, AV_PIX_FMT_GBRP);

 }

int convert_to_RGB24(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[])
{

  int w = f_in->width;
  int h = f_in->height;
  
  AVPixelFormat dst_format;

  dst[0] = (unsigned char *)malloc(w*h*3);
  dst[1] = dst[2] = dst[3] = NULL;

  if(!dst[0]){
    Output("convert_to_RGB24: allocation failure\n");
    return -1;
  }
  dst_stride[0] =  w*3;
  dst_stride[1] = dst_stride[2] = dst_stride[3] = 0;
  return convert_to_format(f_in, dst ,dst_stride, AV_PIX_FMT_RGB24);

  //print_img("rgb24", dst,  w*3,  5,  1);
  
}

int convert_to_GRAY8(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[],int extra_width)
{

  int w = f_in->width;
  int h = f_in->height;
  
  AVPixelFormat dst_format;

  dst[0] = (unsigned char *)malloc((w+extra_width)*h);
  dst[1] = dst[2] = dst[3] = NULL;

  if(!dst[0]){
    Output("convert_to_GRAY8: allocation failure\n");
    return -1;
  }
  dst_stride[0] =  w+extra_width;
  dst_stride[1] = dst_stride[2] = dst_stride[3] = 0;
  return convert_to_format(f_in, dst ,dst_stride, AV_PIX_FMT_GRAY8);

  //print_img("rgb24", dst,  w*3,  5,  1);
  
}

#endif
