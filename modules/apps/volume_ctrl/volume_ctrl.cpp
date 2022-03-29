
/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */
#include <audiotk/audio_vol_ctrl.h>

static void print_usage(const char *ap_name)
{
	/*
 	* Note: In Pesaro platform, please set volume=0 for Mute!!
 	*/
	fprintf(stderr, "Usage:\n"
			"    %s [-i input_type] -v volume [-M mode] [-m][-h]\n"
			"Options:\n"
			"    -i                 Input type of audio. (0: MicIn, 1: LineIn, 2: ByPass)\n"
			"    -M                 mode (0: The range of volume is 0 ~ 100, 1: The volume is used as dB value).\n"
			"    -v                 Volume (0 ~ 100) or dB value. It depends on mode (-M option)\n"
			"    -m                 Mute.\n"
			"    -h                 This help.\n", ap_name);
}

int main(int argc, char **argv)
{
	int opt;
	int input_type = 1;
#ifdef MOZART3S_PLATFORM
	int is_mute = 0;
#endif
	long vol = 100;
	//long pga = 15;
	bool is_input = false;
	int mode = 0;

	while ((opt = getopt(argc, argv, "Dhmi:v:M:p:")) != -1)
	{
		switch(opt)
		{
			case 'i':
				is_input = true;
				input_type = atoi(optarg);
				break;
			case 'v':
				vol = atoi(optarg);
				break;
			case 'm':

		#ifdef MOZART3S_PLATFORM
				is_mute = 1;
		#else
				printf("In Vienna platform, please set volume=0 for Mute!! \n");
		#endif
				break;
			case 'M':
				mode = atoi(optarg);
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if(is_input)
	{
		switch(input_type)
		{
			case 0:
				ATK_Audio_InputSelection(kTKAudioMicIn);
			#ifdef MOZART3S_PLATFORM
				ATK_Audio_SetCaptureMute(is_mute, kTKAudioMicIn);
			#endif
				break;
			case 1:
				ATK_Audio_InputSelection(kTKAudioLineIn);
			#ifdef MOZART3S_PLATFORM
				ATK_Audio_SetCaptureMute(is_mute, kTKAudioLineIn);
			#endif
				break;
			case 2:
				ATK_Audio_InputSelection(kTKAudioByPass);
			#ifdef MOZART3S_PLATFORM
				ATK_Audio_SetCaptureMute(is_mute, kTKAudioByPass);
			#endif
				break;
			default:
				exit(EXIT_FAILURE);
		}

		if(mode)
		{
			ATK_Audio_SetCaptureVolume_dB(vol);
		}
		else
		{
			ATK_Audio_SetCaptureVolume(vol);
		}
	}
	else
	{
#ifdef MOZART3S_PLATFORM
		ATK_Audio_SetPlaybackMute(is_mute);
#endif
		if(mode)
		{
			ATK_Audio_SetPlaybackVolume_dB(vol);
		}
		else
		{
			ATK_Audio_SetPlaybackVolume(vol);
		}
	}

	return 0;
}

