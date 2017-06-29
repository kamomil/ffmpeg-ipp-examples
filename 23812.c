
/*///////////////////////////////////////////////////////////////////////////////////////////
  // This is a simple code to get started to use H.264 decoder. For more of the code sample, check
  // simpleplaeyr application in the UMC sample code 
*/

#include <stdio.h>
#include <stdlib.h>
#include "ipp.h"
#include "umc_defs.h"
#include "umc_video_decoder.h"
#include "umc_video_data.h"
#include "umc_h264_dec.h"
#include "umc_structures.h"

#define MAXFRAME 100000
#define MAXVIDEOSIZE  100000000
#define MAXYUVSIZE  200000000

void DecodeStream( Ipp8u *cVideoData,int VideoDataSize,
                   Ipp8u *cYUVData, int& imgWidth, int & imgHeight, int & frameNumber )
{
    UMC::Status status; UMC::MediaData  DataIn; UMC::VideoData  DataOut;
	UMC::VideoDecoderParams Params;
	UMC::H264VideoDecoder H264Decoder;
	int frameSize=0;
		 
    DataIn.SetBufferPointer(cVideoData,VideoDataSize);
	DataIn.SetDataSize(VideoDataSize);
	
    Params.m_pData = &DataIn;  Params.lFlags=0;  Params.numThreads=1; 
	if(status = H264Decoder.Init(&Params)!=UMC::UMC_OK)
	    return;
	    
	H264Decoder.GetInfo(&Params); 
	imgWidth=Params.info.clip_info.width; imgHeight=Params.info.clip_info.height;

    frameSize =  imgWidth*imgHeight*3/2;   
	DataOut.Init(imgWidth,imgHeight,UMC::YV12,8);  
	DataOut.SetBufferPointer(cYUVData,frameSize);  

   int exit_flag=0; frameNumber=0;       
    
	do{    status  = H264Decoder.GetFrame(&DataIn, &DataOut);
           if (status  == UMC::UMC_OK){ 
              cYUVData += (imgWidth*imgHeight*3/2);
    	      DataOut.SetBufferPointer(cYUVData,frameSize);
    	      frameNumber++;
  	       }  	 
	       if((status  !=UMC::UMC_OK)||(frameNumber >=MAXFRAME))
               exit_flag = 1;
             
       }while (exit_flag!=1);

       do{  status  = H264Decoder.GetFrame(NULL, &DataOut);
	        if (status  == UMC::UMC_OK)  { 
	            cYUVData += frameSize;
    	        DataOut.SetBufferPointer(cYUVData,frameSize);
    	        frameNumber++;
	    }
	}while(status  == UMC::UMC_OK);	
    return;
}



int ReadVideoData(char* strFilename,Ipp8u *cVideoData,int &VideoDataSize)
{
   FILE* fp = fopen(strFilename, "rb");
   if(fp==NULL)
       return 0;
   VideoDataSize=fread(cVideoData,1,MAXVIDEOSIZE,fp);
   fclose(fp);
}
void WriteYUVData(char* strFilename,Ipp8u * cYUVData,int imgWidth, int imgHeight, int frameNumber)
{
	FILE* outfp = fopen(strFilename, "wb");
	if(outfp==NULL)
	   return ;
	fwrite(cYUVData,frameNumber*imgWidth*imgHeight*3/2,1,outfp);
	fclose(outfp);
}

int main(int argc, vm_char* argv[])
 {
   Ipp8u *cVideoData = ippsMalloc_8u(MAXVIDEOSIZE);
   Ipp8u *cYUVData = ippsMalloc_8u(MAXYUVSIZE);
   int VideoDataSize,imgWidth, imgHeight, frameNumber;
   char *  InputVideofileName, *OutputYUVFileName;
   InputVideofileName = "teststream.h264";
   OutputYUVFileName  =  "testoutput.yuv";

   ReadVideoData(InputVideofileName,cVideoData,VideoDataSize);
   DecodeStream(cVideoData,VideoDataSize,cYUVData,imgWidth, imgHeight, frameNumber);
   WriteYUVData(OutputYUVFileName,cYUVData,imgWidth, imgHeight, frameNumber);
   return 0;
} 