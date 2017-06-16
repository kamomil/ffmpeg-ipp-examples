#include "stdafx.h"
//#include "CppUnitTest.h"
#include "ipp.h"
//#include "media_io.h"
#include <windows.h>
//#include <random>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
void Output(const char* szFormat, ...)
{
	char szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnprintf(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}
void print_img(uint8_t* img[], int w, int h, int c) {

	Output("Printing img %p:\n", img);
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


void fill_img(uint8_t* img[],int w,int h,int c){

  for(int ci=0;ci<c;ci++){
    for(int i=0;i<w*h;i++){
      img[ci][i] = (uint8_t)i; 
    }
  }
  return;
}

static uint64_t getSSD(uint8_t *src1, uint8_t *src2, int stride1, int stride2, int w, int h) {
	int x, y;
	uint64_t ssd = 0;

	//printf("%d %d\n", w, h);

	for (y = 0; y<h; y++) {
		for (x = 0; x<w; x++) {
			int d = src1[x + y*stride1] - src2[x + y*stride2];
			ssd += d*d;
			//printf("%d", abs(src1[x + y*stride1] - src2[x + y*stride2])/26 );
		}
		//printf("\n");
	}
	return ssd;
}

// test by ref -> src -> dst -> out & compare out against ref
// ref & out are YV12
static int doTest(uint8_t *ref[3], int refStride[3], int w, int h, AVPixelFormat srcFormat, AVPixelFormat dstFormat,
	int srcW, int srcH, int dstW, int dstH, int flags) {
	uint8_t *src[3];
	uint8_t *dst[3];
	uint8_t *out[3];
	int srcStride[3], dstStride[3];
	int i;
	uint64_t ssdY, ssdU, ssdV;
	struct SwsContext *srcContext, *dstContext, *outContext;
	int res;

	res = 0;
	for (i = 0; i<3; i++) {
		// avoid stride % bpp != 0
		if (srcFormat == AV_PIX_FMT_RGB24 || srcFormat == AV_PIX_FMT_BGR24)
			srcStride[i] = srcW * 3;
		else
			srcStride[i] = srcW * 4;

		if (dstFormat == AV_PIX_FMT_RGB24 || dstFormat == AV_PIX_FMT_BGR24)
			dstStride[i] = dstW * 3;
		else
			dstStride[i] = dstW * 4;

		src[i] = (uint8_t*)malloc(srcStride[i] * srcH);
		dst[i] = (uint8_t*)malloc(dstStride[i] * dstH);
		out[i] = (uint8_t*)malloc(refStride[i] * h);
		if (!src[i] || !dst[i] || !out[i]) {
			perror("Malloc");
			res = -1;

			goto end;
		}
	}

	dstContext = outContext = NULL;
	srcContext = sws_getContext(w, h, AV_PIX_FMT_YUV420P, srcW, srcH, srcFormat, flags, NULL, NULL, NULL);
	if (!srcContext) {
		Output("sws_getContext failed\n");
		res = -1;

		goto end;
	}
	dstContext = sws_getContext(srcW, srcH, srcFormat, dstW, dstH, dstFormat, flags, NULL, NULL, NULL);
	if (!dstContext) {
		Output("sws_getContext failed\n");
		res = -1;

		goto end;
	}
	outContext = sws_getContext(dstW, dstH, dstFormat, w, h, AV_PIX_FMT_YUV420P, flags, NULL, NULL, NULL);
	if (!outContext) {
		Output("sws_getContext failed\n");
		res = -1;

		goto end;
	}
	//    printf("test %X %X %X -> %X %X %X\n", (int)ref[0], (int)ref[1], (int)ref[2],
	//        (int)src[0], (int)src[1], (int)src[2]);

	sws_scale(srcContext, ref, refStride, 0, h, src, srcStride);
	sws_scale(dstContext, src, srcStride, 0, srcH, dst, dstStride);
	sws_scale(outContext, dst, dstStride, 0, dstH, out, refStride);

	ssdY = getSSD(ref[0], out[0], refStride[0], refStride[0], w, h);
	ssdU = getSSD(ref[1], out[1], refStride[1], refStride[1], (w + 1) >> 1, (h + 1) >> 1);
	ssdV = getSSD(ref[2], out[2], refStride[2], refStride[2], (w + 1) >> 1, (h + 1) >> 1);

	if (srcFormat == AV_PIX_FMT_GRAY8 || dstFormat == AV_PIX_FMT_GRAY8) ssdU = ssdV = 0; //FIXME check that output is really gray

	ssdY /= w*h;
	ssdU /= w*h / 4;
	ssdV /= w*h / 4;

	

end:

	sws_freeContext(srcContext);
	sws_freeContext(dstContext);
	sws_freeContext(outContext);

	for (i = 0; i<3; i++) {
		free(src[i]);
		free(dst[i]);
		free(out[i]);
	}

	return res;
}

static void selfTest(uint8_t *src[3], int stride[3], int w, int h) {
	enum AVPixelFormat srcFormat, dstFormat;
	int srcFormati, dstFormati;
	int srcW, srcH, dstW, dstH;
	int flags;

	for (srcFormati = AV_PIX_FMT_NONE+1; srcFormati < AV_PIX_FMT_NB; srcFormati++) {
		srcFormat = static_cast<AVPixelFormat>(srcFormati);
		for (dstFormati = AV_PIX_FMT_NONE + 1; dstFormati < AV_PIX_FMT_NB; dstFormati++) {
			
			dstFormat = static_cast<AVPixelFormat>(dstFormati);
			srcW = w;
			srcH = h;
			for (dstW = w - w / 3; dstW <= 4 * w / 3; dstW += w / 3) {
				for (dstH = h - h / 3; dstH <= 4 * h / 3; dstH += h / 3) {
					for (flags = 1; flags<33; flags *= 2) {
						int res;

						res = doTest(src, stride, w, h, srcFormat, dstFormat,
							srcW, srcH, dstW, dstH, flags);
						if (res < 0) {
							dstW = 4 * w / 3;
							dstH = 4 * h / 3;
							flags = 33;
						}
					}
				}
			}
		}
	}
}

#define W 24
#define H 24
int main() {

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
	/*
	int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
	const int srcStride[], int srcSliceY, int srcSliceH,
	uint8_t *const dst[], const int dstStride[]);

	*/
	sws_scale(ctx, &rgb24Data, inLinesize, 0, h, dst, dst_stride);
	print_img(dst, w, h, 3);
	OutputDebugString("hjghjghjg\n");

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
	


}
