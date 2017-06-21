//#ifdef WIN32
#include "stdafx.h"
//#endif
#include "test_utils.h"
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

//ippiCopy_8u_C1R( src,w,dst_ipp,w,roi);
void ippiCopy_8u_C1R_ffmpeg(unsigned char* src , int ws, unsigned char *dst ,int wd, IppiSize roi){
  /*
void av_image_copy_plane 	( 	uint8_t *  	dst,
		int  	dst_linesize,
		const uint8_t *  	src,
		int  	src_linesize,
		int  	bytewidth,
		int  	height 
	) 		
  */
  
  av_image_copy_plane(dst,wd,src,ws,roi.width,roi.height); 
	
}
  
void test_ippiCopy_8u_C1R_replacement(){

  IppiSize roi;

  int w = rand_in_range(20,40);
  int h = rand_in_range(20,40);
  int offset = rand_in_range(0,w*h-1);
  int wr = rand_in_range(1,w - (offset % w));
  int hr = rand_in_range(1, h- (offset/w));

    
  unsigned char* src;
  unsigned char *dst_ipp,*dst_ffmpeg;

  src = (uint8_t*)malloc(h*w);
    
  fill_img(&src, w, h, 1,1);
  //print_img("src",&src, w, h, 1);
  
  dst_ipp = (uint8_t*)malloc(h*w);
  memset(dst_ipp,0,h*w);

  dst_ffmpeg = (uint8_t*)malloc(h*w);
  memset(dst_ffmpeg,0,h*w);

  roi.height    = hr;
  roi.width     = wr;
  
  
  ippiCopy_8u_C1R( src+offset,w,dst_ipp,w,roi);
  //print_img("dst_ipp",&dst_ipp, w, h, 1);

  ippiCopy_8u_C1R_ffmpeg(src+offset ,w,dst_ffmpeg ,w,roi);
  //print_img("dst_ffmpeg",&dst_ffmpeg, w, h, 1);

  double d[1] = {0};
  l2_dist_img(d,&dst_ipp,&dst_ffmpeg,w,h,1);
  double  n = ncc(dst_ipp,dst_ffmpeg,w*h);

  //Output("distances are %f  img = (%d X %d) offset=%d , roi=%d X %d )\n",d[0],w,h,offset,wr,hr);
  Output("ncc is %f  img = (%d X %d) offset=%d , roi=%d X %d )\n",n,w,h,offset,wr,hr);
  
    return;
}

/*
Parameters

pSrc	Pointer to the source image origin. An array of separate pointers to each plane in case of data in planar format.

srcSize	Size in pixels of the source image.

srcStep	Distance in bytes between starts of consecutive lines in the source image buffer.

srcRoi	Region of interest in the source image (of the IppiRect type).

pDst	Pointer to the destination image ROI. An array of pointers to separate ROI in each planes for planar image.

dstStep	Distance in bytes between starts of consecutive lines in the destination image buffer.

dstRoiSize	Size of the destination ROI in pixels.

xFactor, yFactor Factors by which the x and y dimensions of the source ROI are changed. The factor value greater than 1 corresponds to increasing the image size in that dimension.

interpolationSpecifies the interpolation mode. Use one of the following values:
 */

/*
57 // values for the flags, the stuff on the command line is different 
58 #define SWS_FAST_BILINEAR     1
59 #define SWS_BILINEAR          2
60 #define SWS_BICUBIC           4
61 #define SWS_X                 8
62 #define SWS_POINT          0x10
63 #define SWS_AREA           0x20
64 #define SWS_BICUBLIN       0x40
65 #define SWS_GAUSS          0x80
66 #define SWS_SINC          0x100
67 #define SWS_LANCZOS       0x200
68 #define SWS_SPLINE        0x400
*/
/*
IPPI_INTER_NN nearest neighbor interpolation
IPPI_INTER_LINEAR linear interpolation
IPPI_INTER_CUBIC cubic interpolation
IPPI_INTER_SUPER supersampling interpolation, cannot be applied for image enlarging
IPPI_INTER_LANCZOS interpolation with Lanczos window.
*/

int  ippiResize_8u_C1R_ffmpeg(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, double xFactor, double yFactor, int interpolation) {
	
	/*
	srcW	the width of the source image
		srcH	the height of the source image
		srcFormat	the source image format
		dstW	the width of the destination image
		dstH	the height of the destination image
		dstFormat	the destination image format
		flags	specify which algorithm and options to use for rescaling
		*/
	int flag = -1;
	switch (interpolation) {
		case IPPI_INTER_LINEAR:
			flag = SWS_BILINEAR;
			break;
		case IPPI_INTER_SUPER:
			flag = SWS_BILINEAR;
			break;
		case IPPI_INTER_CUBIC:
			flag = IPPI_INTER_CUBIC;
			break;
		case IPPI_INTER_LANCZOS:
			flag = SWS_LANCZOS;
			break;
		case IPPI_INTER_NN:
			Output("failure: sws scale have no 'nearest neighbour' option\n");
			return -1;
		default:
			Output("failure: invalid interpolation value: %d\n", interpolation);
			return -1;
	}
	
	//   
	//SwsContext * ctx = sws_getContext(srcSize.width, srcSize.height, AV_PIX_FMT_RGB24, srcSize.width, srcSize.height, AV_PIX_FMT_YUV444P, 0, NULL, NULL, NULL);
	SwsContext *ctx = sws_getContext(srcRoi.width, srcRoi.height, AV_PIX_FMT_GRAY8,
		                                     dstRoiSize.width, dstRoiSize.height, AV_PIX_FMT_GRAY8,
		                                     flag, NULL, NULL, NULL);//flag, srcFilter, dstFilter and params
	/*
	c	the scaling context previously created with sws_getContext()
srcSlice	the array containing the pointers to the planes of the source slice
srcStride	the array containing the strides for each plane of the source image
srcSliceY	the position in the source image of the slice to process, that is the number (counted starting from zero) in the image of the first row of the slice
srcSliceH	the height of the source slice, that is the number of rows in the slice
dst	the array containing the pointers to the planes of the destination image
dstStride	the array containing the strides for each plane of the destination image 
	*/
	const uint8_t* srcSlice[4] = { pSrc+(srcStep*srcRoi.y+srcRoi.x),NULL,NULL,NULL};
	const int srcStride[4] = { srcStep,0,0,0 };
	int srcSliceY = 0;
	int srcSliceH = srcRoi.height;
	uint8_t* dst[4] = { pDst,NULL,NULL,NULL };
	const int  dstStride[4] = { dstRoiSize.width,0,0,0};

		
	int t = sws_scale(ctx, srcSlice, srcStride, srcSliceY, srcSliceH, &pDst, dstStride);
	if(t == 0)
	    Output("error: sws_scale failed\n");
	else
	  Output("sws_scale returned with t=%d\n",t);
	return 0;
}

void test_ippiResize_8u_C1R_replacement(unsigned char* src,int w,int h){
  
  IppiSize srcSize,dstSize;
  IppiRect srcRect;
  
  //int w = rand_in_range(50,70);
  //int h = rand_in_range(50,70);
  int offset = rand_in_range(0,w*h-1);
 
  
  srcRect.x = 10; //rand_in_range(w / 3, w - 1);
  srcRect.y = 10;// rand_in_range(h / 3, h - 1);
  
  srcRect.width = 30;// rand_in_range((w - srcRect.x) / 3, w - srcRect.x);
  srcRect.height = 30;// rand_in_range((h - srcRect.y) / 3, h - srcRect.y);
  
  for (int hi = 10; hi < 40; hi++) {
	  for (int wi = 10; wi < 40; wi++) {
		  Output("%d ", src[wi + w*hi]);
	  }
	  Output("\n");
  }
 
  double xfactor = 1.5;// rand_in_range_double(0.1, 1.5);
  double yfactor = 1.5;// rand_in_range_double(0.1, 1.5);
  int interpolation = ( xfactor >= 1.0 || yfactor >= 1.0 ) ? IPPI_INTER_LINEAR : IPPI_INTER_SUPER;

  unsigned char *dst_ipp,*dst_ffmpeg;

  srcSize.width  = w;
  srcSize.height = h;
  dstSize.width  = (int)( xfactor * srcRect.width);
  dstSize.height = (int)( yfactor * srcRect.height);
    
  //fill_img(&src, w, h, 1,1);
  //print_img("src",&src, w, h, 1);
  
  dst_ipp = (uint8_t*)malloc(dstSize.height * dstSize.width);
  memset(dst_ipp,0, dstSize.height * dstSize.width);
  
 

  /*                                     1              2            3                 4              5          6                7                   8              9            10   
IppStatus ippiResize_8u_C1R(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, double xFactor, double yFactor, int interpolation);
  */
  /*
  ippiResize_8u_C1R( src,    srcSize,w,srcRect,
	                 dst_ipp,dstSize.width,dstSize,
	                 xfactor,yfactor,interpolation);
					 
  print_img("dst_ipp",&dst_ipp, dstSize.width, dstSize.height, 1);
  */
  dst_ffmpeg = (uint8_t*)malloc(dstSize.height * dstSize.width);
  memset(dst_ffmpeg,0, dstSize.height * dstSize.width);

  ippiResize_8u_C1R_ffmpeg(src, srcSize, w, srcRect,
	  dst_ffmpeg, dstSize.width, dstSize,
	  xfactor, yfactor, interpolation);
	  
  print_img("dst_ffmpeg", &dst_ffmpeg, dstSize.width, dstSize.height, 1);
  //double n  = ncc(dst_ipp,dst_ffmpeg,dstSize.width*dstSize.height);
  //double n  = ncc(dst_ipp,dst_ffmpeg,dstSize.width*dstSize.height);
  //Output("ncc is %f\n",n);
  exit(1);
}

#define W 24
#define H 24
static int decode_write_frame(AVCodecContext *avctx,
                              AVFrame *frame, AVPacket *pkt, int last)
{
    

  for(int i=0;i<AV_NUM_DATA_POINTERS && frame->data[i];i++){
    Output("frame->data[%d] = %p\n",i,frame->data[i]);
    Output("linesize[%d]=%u frame->width = %u, frame->height = %u\n",i,frame->linesize[i],frame->width, frame->height);
    for(int j=0;j<10;j++){
      Output("%u ",frame->data[i][j]);
    }
    Output("\n\n");
	if (i == 0)
		break;
  }
  test_ippiResize_8u_C1R_replacement(frame->data[0],frame->linesize[0],frame->height);
  

 
        
  
  
  return 0;
}

#define INBUF_SIZE 4096
static void video_decode_example(const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_count;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;

    av_init_packet(&avpkt);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    Output("Decode video file %s \n", filename);

    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
        c->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    frame_count = 0;
    int max_frame = 1;
	for (;;) {
		avpkt.size = fread(inbuf, 1, INBUF_SIZE, f);
		if (avpkt.size == 0)
			break;
		avpkt.data = inbuf;
		while (avpkt.size > 0) {

			int len, got_frame;
			len = avcodec_decode_video2(c, frame, &got_frame, &avpkt);
			if (len < 0) {
				fprintf(stderr, "Error while decoding frame %d\n", frame_count);
				return;// len;
			}
			if (got_frame) {

				if (frame_count % 100 == 0) {
					Output("frame %d:\n", frame_count);
					if (decode_write_frame(c, frame, &avpkt, 0) < 0) {
						Output("decode_write_frame failed\n");
						exit(1);
					}
					char buf[1024];
					/* the picture is allocated by the decoder, no need to free it */
					//in VS the file will be written to UnitTest1 directory
					snprintf(buf, sizeof(buf), "test%02d.pgm", frame_count);
					Output("writing frame %d to file '%s'\n", frame_count, buf);
					pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, buf);
				}

				

				frame_count++;
				//if(frame_count>max_frame)
				//  return;
			}
			if (avpkt.data) {
				avpkt.size -= len;
				avpkt.data += len;
			}


		}
	}
    
    /* some codecs, such as MPEG, transmit the I and P frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video */
    avpkt.data = NULL;
    avpkt.size = 0;
    decode_write_frame(c, frame, &avpkt, 1);

    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&frame);
    Output("\n");
}



int main(int argc, char **argv)
{
#ifdef WIN32 
	//std::random_device rd;	
	/*std::mt19937gen(rd());*/
	srand(54);
#else
	srand(time(NULL));
#endif

	avcodec_register_all();
    if (argc < 2) {
      Output("usage: %s filename\n",argv[0]);
      return 1;
    }
    //char* filename = argv[1];   
    video_decode_example(argv[1]);
 
    return 0;
}
