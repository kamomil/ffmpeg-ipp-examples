#include "stdafx.h"
extern "C"
{
#include <ipp.h>
#include "ippcc.h"
#include "ippi.h"
}

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
#include "Copy_8u_C1R_test.h"
#include "Mirror_8u_C1R_test.h"
#include "Mirror_8u_C1IR_test.h"

#include "Copy_8u_C3P3R_test.h"
#include "Copy_8u_C4C1R_test.h"
#include "RGBToYCbCr_8u_P3R_test.h"
#include "Resize_8u_C1R_test.h"
#include "Set_8u_C1R_test.h"
#include "AlphaCompC_8u_C1R_test.h"
#include "YCbCrToRGB_8u_P3R_test.h"
static int decode_write_frame(AVFrame *frame, int idx, int last)
{
    
  int w = frame->width;
  int h = frame->height;

  Output("frame->data = %p\n",frame->data);
  for(int i=0;i<AV_NUM_DATA_POINTERS && frame->data[i];i++){
    Output("frame->data[%d] = %p\n",i,frame->data[i]);
    Output("linesize[%d]=%u frame->width = %u, frame->height = %u frame->format = %d\n",i,frame->linesize[i],frame->width, frame->height,frame->format);
    for(int j=0;j<10;j++){
      Output("%u ",frame->data[i][j]);
    }
    Output("\n");
	if (i == 0)
		break;
  }
  
  //test_ippiResize_8u_C1R_replacement(frame->data[0],frame->linesize[0],frame->height);

  double ncc_val[3]={0,0,0};

  //test_ippiRGBToYCbCr_8u_P3R_replacement(AVFrame *frame, double ncc_plain_val[3]);
  /*
  test_ippiRGBToYCbCr_8u_P3R_replacement(frame,ncc_val);
  Output("ippiRGBToYCbCr_8u_P3R compare:\n");
  Output("%f %f %f\n\n",ncc_val[0],ncc_val[1],ncc_val[2]);

  ncc_val[0] = 0;
  test_ippiCopy_8u_C1R_replacement(frame,ncc_val);
  Output("ippiCopy_8u_C1R compare:\n");
  Output("%f\n\n",ncc_val[0]);

  ///ncc_val[0] = 0;
  ///test_ippiMirror_8u_C1R_replacement(frame,ncc_val);
  ///Output("ippiMirror_8u_C1R compare:\n");
  ///Output("%f\n\n",ncc_val[0]);

  ncc_val[0] = 0;
  test_ippiMirror_8u_C1IR_replacement(frame,ncc_val);
  Output("ippiMirror_8u_C1IR compare:\n");
  Output("%f\n\n",ncc_val[0]);

  
  ncc_val[0] = ncc_val[1] = ncc_val[0] = 2;
  test_ippiCopy_8u_C3P3R_replacement(frame,ncc_val);
  Output("ippiCopy_8u_C3P3R compare:\n");
  Output("%f %f %f\n\n",ncc_val[0],ncc_val[1],ncc_val[2]);
  */
  /*
  ncc_val[0] = ncc_val[1] = ncc_val[0] = 2;
  test_ippiResize_8u_C1R_replacement(frame,ncc_val,idx);
  Output("ippiResize_8u_C1R compare:\n");
  Output("%f\n",ncc_val[0]);
  */
  /*
  ncc_val[0] = ncc_val[1] = ncc_val[0] = 2;
  test_ippiSet_8u_C1R_replacement(frame,ncc_val);
  Output("ippiSet_8u_C1R compare:\n");
  Output("%f\n",ncc_val[0]);
  */
  /*
  ncc_val[0] = ncc_val[1] = ncc_val[0] = 0;
  test_ippiAlphaCompC_8u_C1R_replacement(frame,ncc_val,idx);

  Output("ippiAlphaCompC_8u_C1R compare:\n");
  Output("%f\n",ncc_val[0]);

  ncc_val[0] = ncc_val[1] = ncc_val[0] = 0;
  test_ippiCopy_8u_C4C1R_replacement(frame,ncc_val);
  Output("ippiCopy_8u_C4C1R_daf compare:\n");
  Output("%f\n",ncc_val[0]);
  */

  ncc_val[0] = ncc_val[1] = ncc_val[0] = 0;
  test_ippiYCbCrToRGB_8u_P3R_replacement(frame,ncc_val);
  Output("ippiYCbCrToRGB_8u_P3R_ffmpeg compare:\n");
  Output("%f %f %f\n",ncc_val[0],ncc_val[1],ncc_val[2]);
  
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
	  
	  if (frame_count % 50 == 0) {
	    Output("\nframe %d:\n", frame_count);
	    if (decode_write_frame(frame, frame_count,0) < 0) {
	      Output("decode_write_frame failed\n");
	      exit(1);
	    }
	    char buf[1024];
	    /* the picture is allocated by the decoder, no need to free it */
	    //in VS the file will be written to UnitTest1 directory
	    snprintf(buf, sizeof(buf), "Sampl%02d.pgm", frame_count);
	    //Output("writing frame %d to file '%s'\n", frame_count, buf);
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
    //decode_write_frame(c, frame, &avpkt, 1);

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
	av_log_set_level(8);
	avcodec_register_all();
	av_register_all();
    if (argc < 2) {
      Output("usage: %s filename\n",argv[0]);
      return 1;
    }
    //char* filename = argv[1];   
    video_decode_example(argv[1]);
 
    return 0;
}
