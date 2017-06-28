




#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/intreadwrite.h>
#include <libavutil/pixfmt.h>

#define Output printf

/*
void convert_to_format(AVFrame* f_in, unsigned char *dst[] ,enum AVPixelFormat out_format)
{
  int w =0;
  int h = 0;
  int dst_ffmpeg_stride[3] = { w, w, w };

  unsigned char* src_packed = (unsigned char*)malloc(w*h*3);
  if(!src_packed){
    Output("failure allocating src_packed\n");
    return;
  }
  planar_to_packed(src,src_packed,w,h,3);
  //print_img("src_packed",&src_packed,3*w,h,1);

  //AV_PIX_FMT_YUV444P - //planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) 
  SwsContext * ctx = sws_getContext(w, h,AV_PIX_FMT_, w, h,AV_PIX_FMT_YUV444P, 0, NULL, NULL, NULL);
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
*/
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
  //test_ippiResize_8u_C1R_replacement(frame->data[0],frame->linesize[0],frame->height);

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

    AVFormatContext *pFormatCtx = NULL;

    
    // Open video file
    //int avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
    if(avformat_open_input(&pFormatCtx,             filename,        NULL, NULL)!=0){
      Output("avformat_open_input failed\n");
      return; // Couldn't open file
    }
    

    // Retrieve stream information
    //this function populates pFormatCtx->streams 
    if(avformat_find_stream_info(pFormatCtx, NULL)<0){
      Output("avformat_find_stream_info failed\n");
      return; // Couldn't find stream information
    }

    
    // Dump information about file onto standard error
    Output("dumping format:\n");
    av_dump_format(pFormatCtx, 0,filename, 0);
    
    AVCodecContext *pCodecCtxOrig = NULL;
    AVCodecContext *pCodecCtx = NULL;
    
    // Find the first video stream
    int videoStream=-1;
    for(int i=0; i<pFormatCtx->nb_streams; i++)
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	videoStream=i;
	break;
      }
    
    if(videoStream==-1){
      Output("no video stream\n");
      return; // Didn't find a video stream
    }
    Output("stream is %d\n",videoStream);
    
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;



    AVCodec *pCodec = NULL;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
      fprintf(stderr, "Unsupported codec!\n");
      return; // Codec not found
    }
    
    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    pCodecCtxOrig = avcodec_alloc_context3(pCodec);
    /*
    if(avcodec_copy_context(pCodecCtxOrig, pCodecCtx) != 0) {
      Output("Couldn't copy codec context");
      return; // Error copying codec context
    }
    */
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
      Output("avcodec_open2 failed\n");
      return; // Could not open codec
    }

    AVFrame *pFrame = NULL;

    // Allocate video frame
    pFrame=av_frame_alloc();

    AVPacket packet;

    int frame_idx = 0;
    unsigned char* video_buffer;
    while(av_read_frame(pFormatCtx, &packet)>=0) {
      // Is this a packet from the video stream?
      if(packet.stream_index==videoStream) {
	// Decode video frame
	int frameFinished = 0;

	int buffer_size = packet.size;
	video_buffer = (unsigned char *)av_malloc(packet.size);
	memset(video_buffer, NULL, packet.size);
	memcpy(video_buffer, packet.data, packet.size);
	Output("dumping first bytes out of %d of the row packet buffer:\n",buffer_size);
	av_hex_dump(0, video_buffer, FFMIN(buffer_size,30));
		
	avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
	
	// Did we get a video frame?
	if(frameFinished) {
	  Output("got frame %d\n",frame_idx);
	  frame_idx++;
	  Output("decoded frame data: format=%d width=%d height=%d linesize={%d,%d,%d,%d}\n",pFrame->format,
		 pFrame->width,pFrame->height,pFrame->linesize[0],pFrame->linesize[1],pFrame->linesize[2],pFrame->linesize[3]);
	  //av_hex_dump(0, pFrame->data[0], pFrame->width);//first param is FILE* (0 for stdout)
	}

      }
    }
    // Allocate an AVFrame structure
    //AVFrame* pFrameRGB=av_frame_alloc();
    //if(pFrameRGB==NULL)
    //  return;
    
}



int main(int argc, char **argv)
{

	//avcodec_register_all();
    av_register_all();
    if (argc < 2) {
      Output("usage: %s filename\n",argv[0]);
      return 1;
    }
    //char* filename = argv[1];   
    video_decode_example(argv[1]);
 
    return 0;
}
