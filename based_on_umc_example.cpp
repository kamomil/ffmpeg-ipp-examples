//////// dafna's code - the code is base on the examples from: https://software.intel.com/file/23812
//the inputbuffer is taken from ffmpeg's av_read_fram or somthing

#define MAXFRAME 100000
#define MAXVIDEOSIZE  100000000
#define MAXYUVSIZE  200000000


//int decoderManager::dafna_decoderInit(unsigned char* inputBuffer, int dataLen) {
int decoderManager::dafna_decoderInit(int w, int h) {
	
	d_cYUVData_cur = d_cYUVData = ippsMalloc_8u(MAXYUVSIZE);

	d_umc_dec_params = new UMC::VideoDecoderParams();
	d_umc_input = new UMC::MediaData();
	d_umc_output = new UMC::VideoData();
	d_umc_decoder = new UMC::H264VideoDecoder();

	
	d_umc_dec_params->lFlags = 0;
	d_umc_dec_params->numThreads = 1;
	d_umc_dec_params->info.clip_info.width = w;
	d_umc_dec_params->info.clip_info.height = h;
	if (d_umc_decoder->Init(d_umc_dec_params) != UMC::UMC_OK)
		return -1;

	d_umc_decoder->GetInfo(d_umc_dec_params);
	d_imgWidth = w;
	d_imgHeight = h;

	d_frameNumber = 0;
	return 0;
}



int decoderManager::dafna_decodeFrame(unsigned char* inputBuffer, int dataLen)
{
	
	UMC::Status status;
	UMC::MediaData  DataIn;
	UMC::VideoData  DataOut;

	int frameSize = d_imgWidth * d_imgHeight * 3 / 2;

	DataOut.Init(d_imgWidth, d_imgHeight, UMC::YV12, 8);
	
	int exit_flag = 0;
	if (inputBuffer) {
		DataIn.SetBufferPointer(inputBuffer, dataLen);
		DataIn.SetDataSize(dataLen);
		do {

			DataOut.SetBufferPointer(d_cYUVData_cur, frameSize);
			status = d_umc_decoder->GetFrame(&DataIn, &DataOut);

			if (status == UMC::UMC_OK) {
				d_cYUVData_cur += (frameSize);
				d_frameNumber++;
			}
			if ((status != UMC::UMC_OK) || ((d_frameNumber) >= MAXFRAME)) {
				printf("decoderManager::DecodeStream: status is %d\n", status);
				exit_flag = 1;
			}
		} while (exit_flag != 1);
	}
	else {
		do {
			status = d_umc_decoder->GetFrame(NULL, &DataOut);
			if (status == UMC::UMC_OK) {
				d_cYUVData_cur += frameSize;
				DataOut.SetBufferPointer(d_cYUVData_cur, frameSize);
				d_frameNumber++;
			}
		} while (status == UMC::UMC_OK);
	}
	return status;
}


int decoderManager::dafna_decoderWriteYUVData(char strFilename[]) {
	
	FILE* outfp = fopen(strFilename, "wb");
	if (outfp == NULL)
		return -1;
	int r = fwrite(d_cYUVData, d_frameNumber*d_imgWidth*d_imgHeight * 3 / 2, 1, outfp);
	if (fclose(outfp) != 0) {
		printf("WriteYUVData: error closing the file %s\n", strFilename);
		return -1;
	}
	return r;
}
