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

double rand_in_range_double(double min, double max);

int rand_in_range(int min,int max);

void print_img(char* name, unsigned char* img[], int w, int h, int c);

void print_img2(char* name, unsigned char* img[], int stride, int w, int h, int c);

void l2_dist_img(double distances[], unsigned char* img1[],unsigned char* img2[], int w, int h, int c);

double ncc2(const unsigned char* im1 ,int im1_linesz, const unsigned char* im2,int im2_linesz, int w,int h);
  
double ncc(const unsigned char* im1 , const unsigned char* im2,int sz);

void fill_img(unsigned char* img[],int w,int h,int c,char with_rand);

void planar_to_packed(unsigned char* src[] , unsigned char *dst ,int w,int h, int c);

void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,char *filename);

int convert_to_format(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[], AVPixelFormat dst_format);

int convert_to_GBRP(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[]);

int convert_to_RGB24(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[],int extra_width);

int convert_to_GRAY8(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[],int extra_width);

int convert_to_RGBA(AVFrame* f_in, unsigned char *dst[] ,int dst_stride[],int extra_width);
#endif
