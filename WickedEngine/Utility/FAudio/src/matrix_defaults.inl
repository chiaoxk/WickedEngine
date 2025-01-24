/* This was generated by making 8 sources and 8 submixes, then assigning each
 * submix to each voice and dumping the output matrix. Terrible, but it worked!
 */
/*
int main(int argc, char **argv)
{
	CoInitialize(NULL);

	IXAudio2 *engine;
	XAudio2Create(&engine);

	IXAudio2MasteringVoice *master;
	engine->CreateMasteringVoice(&master);

	FILE *fileOut = fopen("matrix_defaults.inl", "w");
	for (int srcChans = 1; srcChans < 9; srcChans += 1)
	{
		fprintf(fileOut, "{\n");
		for (int dstChans = 1; dstChans < 9; dstChans += 1)
		{
			IXAudio2SubmixVoice *submix;
			engine->CreateSubmixVoice(&submix, dstChans, 48000);

			XAUDIO2_SEND_DESCRIPTOR sendDesc;
			sendDesc.Flags = 0;
			sendDesc.pOutputVoice = submix;

			XAUDIO2_VOICE_SENDS sends;
			sends.SendCount = 1;
			sends.pSends = &sendDesc;

			WAVEFORMATEX fmt;
			fmt.wFormatTag = 1;
			fmt.nChannels = srcChans;
			fmt.nSamplesPerSec = 48000;
			fmt.wBitsPerSample = 16;
			fmt.nBlockAlign = srcChans * (fmt.wBitsPerSample / 8);
			fmt.nAvgBytesPerSec = fmt.nBlockAlign * fmt.nSamplesPerSec;
			fmt.cbSize = 0;

			IXAudio2SourceVoice *source;
			engine->CreateSourceVoice(&source, &fmt, 0, 2.0f, NULL, &sends);

			float matrix[8 * 8];
			source->GetOutputMatrix(submix, srcChans, dstChans, matrix);
			fprintf(fileOut, "\t{ ");
			for (int i = 0; i < srcChans * dstChans; i += 1)
			{
				fprintf(fileOut, "%.9f%s ", matrix[i], (i == ((srcChans * dstChans) - 1)) ? "" : ",");
			}
			fprintf(fileOut, "}%s\n", (dstChans == 8) ? "" : ",");

			source->DestroyVoice();
			submix->DestroyVoice();
		}
		fprintf(fileOut, "}%s\n", (srcChans == 8) ? "" : ",");
	}
	fclose(fileOut);

	master->DestroyVoice();
	engine->Release();
	CoUninitialize();
	return 0;
}
*/
{
	/* 1 x 1 */
	{ 1.000000000f },
	/* 1 x 2 */
	{ 1.000000000f, 1.000000000f },
	/* 1 x 3 */
	{ 1.000000000f, 1.000000000f, 0.000000000f },
	/* 1 x 4 */
	{ 1.000000000f, 1.000000000f, 0.000000000f, 0.000000000f },
	/* 1 x 5 */
	{ 1.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 1 x 6 */
	{ 1.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 1 x 7 */
	{ 1.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 1 x 8 */
	{ 1.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 2 x 1 */
	{ 0.500000000f, 0.500000000f },
	/* 2 x 2 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 2 x 3 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f },
	/* 2 x 4 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 2 x 5 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 2 x 6 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 2 x 7 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 2 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 3 x 1 */
	{ 0.333333343f, 0.333333343f, 0.333333343f },
	/* 3 x 2 */
	{ 0.800000012f, 0.000000000f, 0.200000003f, 0.000000000f, 0.800000012f, 0.200000003f },
	/* 3 x 3 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 3 x 4 */
	{ 0.888888896f, 0.000000000f, 0.111111112f, 0.000000000f, 0.888888896f, 0.111111112f, 0.000000000f, 0.000000000f, 0.111111112f, 0.000000000f, 0.000000000f, 0.111111112f },
	/* 3 x 5 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 3 x 6 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 3 x 7 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 3 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 4 x 1 */
	{ 0.250000000f, 0.250000000f, 0.250000000f, 0.250000000f },
	/* 4 x 2 */
	{ 0.421000004f, 0.000000000f, 0.358999997f, 0.219999999f, 0.000000000f, 0.421000004f, 0.219999999f, 0.358999997f },
	/* 4 x 3 */
	{ 0.421000004f, 0.000000000f, 0.358999997f, 0.219999999f, 0.000000000f, 0.421000004f, 0.219999999f, 0.358999997f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 4 x 4 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 4 x 5 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 4 x 6 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 4 x 7 */
	{ 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.500000000f, 0.500000000f, 0.000000000f, 0.000000000f, 0.796000004f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.796000004f },
	/* 4 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 5 x 1 */
	{ 0.200000003f, 0.200000003f, 0.200000003f, 0.200000003f, 0.200000003f },
	/* 5 x 2 */
	{ 0.374222219f, 0.000000000f, 0.111111112f, 0.319111109f, 0.195555553f, 0.000000000f, 0.374222219f, 0.111111112f, 0.195555553f, 0.319111109f },
	/* 5 x 3 */
	{ 0.421000004f, 0.000000000f, 0.000000000f, 0.358999997f, 0.219999999f, 0.000000000f, 0.421000004f, 0.000000000f, 0.219999999f, 0.358999997f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f },
	/* 5 x 4 */
	{ 0.941176474f, 0.000000000f, 0.058823530f, 0.000000000f, 0.000000000f, 0.000000000f, 0.941176474f, 0.058823530f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.058823530f, 0.941176474f, 0.000000000f, 0.000000000f, 0.000000000f, 0.058823530f, 0.000000000f, 0.941176474f },
	/* 5 x 5 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 5 x 6 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 5 x 7 */
	{ 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.500000000f, 0.500000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.796000004f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.796000004f },
	/* 5 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 6 x 1 */
	{ 0.166666672f, 0.166666672f, 0.166666672f, 0.166666672f, 0.166666672f, 0.166666672f },
	/* 6 x 2 */
	{ 0.294545442f, 0.000000000f, 0.208181813f, 0.090909094f, 0.251818180f, 0.154545456f, 0.000000000f, 0.294545442f, 0.208181813f, 0.090909094f, 0.154545456f, 0.251818180f },
	/* 6 x 3 */
	{ 0.324000001f, 0.000000000f, 0.229000002f, 0.000000000f, 0.277000010f, 0.170000002f, 0.000000000f, 0.324000001f, 0.229000002f, 0.000000000f, 0.170000002f, 0.277000010f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f },
	/* 6 x 4 */
	{ 0.558095276f, 0.000000000f, 0.394285709f, 0.047619049f, 0.000000000f, 0.000000000f, 0.000000000f, 0.558095276f, 0.394285709f, 0.047619049f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.047619049f, 0.558095276f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.047619049f, 0.000000000f, 0.558095276f },
	/* 6 x 5 */
	{ 0.586000025f, 0.000000000f, 0.414000005f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.586000025f, 0.414000005f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.586000025f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.586000025f },
	/* 6 x 6 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 6 x 7 */
	{ 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.939999998f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.500000000f, 0.500000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.796000004f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.796000004f },
	/* 6 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
},
{
	/* 7 x 1 */
	{ 0.143142849f, 0.143142849f, 0.143142849f, 0.142857149f, 0.143142849f, 0.143142849f, 0.143142849f },
	/* 7 x 2 */
	{ 0.247384623f, 0.000000000f, 0.174461529f, 0.076923080f, 0.174461529f, 0.226153851f, 0.100615382f, 0.000000000f, 0.247384623f, 0.174461529f, 0.076923080f, 0.174461529f, 0.100615382f, 0.226153851f },
	/* 7 x 3 */
	{ 0.268000007f, 0.000000000f, 0.188999996f, 0.000000000f, 0.188999996f, 0.245000005f, 0.108999997f, 0.000000000f, 0.268000007f, 0.188999996f, 0.000000000f, 0.188999996f, 0.108999997f, 0.245000005f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 7 x 4 */
	{ 0.463679999f, 0.000000000f, 0.327360004f, 0.040000003f, 0.000000000f, 0.168960005f, 0.000000000f, 0.000000000f, 0.463679999f, 0.327360004f, 0.040000003f, 0.000000000f, 0.000000000f, 0.168960005f, 0.000000000f, 0.000000000f, 0.000000000f, 0.040000003f, 0.327360004f, 0.431039989f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.040000003f, 0.327360004f, 0.000000000f, 0.431039989f },
	/* 7 x 5 */
	{ 0.483000010f, 0.000000000f, 0.340999991f, 0.000000000f, 0.000000000f, 0.175999999f, 0.000000000f, 0.000000000f, 0.483000010f, 0.340999991f, 0.000000000f, 0.000000000f, 0.000000000f, 0.175999999f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.340999991f, 0.449000001f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.340999991f, 0.000000000f, 0.449000001f },
	/* 7 x 6 */
	{ 0.611000001f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.223000005f, 0.000000000f, 0.000000000f, 0.611000001f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.223000005f, 0.000000000f, 0.000000000f, 0.611000001f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.432000011f, 0.568000019f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.432000011f, 0.000000000f, 0.568000019f },
	/* 7 x 7 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f },
	/* 7 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.707000017f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.707000017f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f }
},
{
	/* 8 x 1 */
	{ 0.125125006f, 0.125125006f, 0.125125006f, 0.125000000f, 0.125125006f, 0.125125006f, 0.125125006f, 0.125125006f },
	/* 8 x 2 */
	{ 0.211866662f, 0.000000000f, 0.150266662f, 0.066666670f, 0.181066677f, 0.111066669f, 0.194133341f, 0.085866667f, 0.000000000f, 0.211866662f, 0.150266662f, 0.066666670f, 0.111066669f, 0.181066677f, 0.085866667f, 0.194133341f },
	/* 8 x 3 */
	{ 0.226999998f, 0.000000000f, 0.160999998f, 0.000000000f, 0.194000006f, 0.119000003f, 0.208000004f, 0.092000000f, 0.000000000f, 0.226999998f, 0.160999998f, 0.000000000f, 0.119000003f, 0.194000006f, 0.092000000f, 0.208000004f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f },
	/* 8 x 4 */
	{ 0.466344833f, 0.000000000f, 0.329241365f, 0.034482758f, 0.000000000f, 0.000000000f, 0.169931039f, 0.000000000f, 0.000000000f, 0.466344833f, 0.329241365f, 0.034482758f, 0.000000000f, 0.000000000f, 0.000000000f, 0.169931039f, 0.000000000f, 0.000000000f, 0.000000000f, 0.034482758f, 0.466344833f, 0.000000000f, 0.433517247f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.034482758f, 0.000000000f, 0.466344833f, 0.000000000f, 0.433517247f },
	/* 8 x 5 */
	{ 0.483000010f, 0.000000000f, 0.340999991f, 0.000000000f, 0.000000000f, 0.000000000f, 0.175999999f, 0.000000000f, 0.000000000f, 0.483000010f, 0.340999991f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.175999999f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.483000010f, 0.000000000f, 0.449000001f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.483000010f, 0.000000000f, 0.449000001f },
	/* 8 x 6 */
	{ 0.518000007f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.188999996f, 0.000000000f, 0.000000000f, 0.518000007f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.188999996f, 0.000000000f, 0.000000000f, 0.518000007f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.518000007f, 0.000000000f, 0.481999993f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.518000007f, 0.000000000f, 0.481999993f },
	/* 8 x 7 */
	{ 0.541000009f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.541000009f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.541000009f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.287999988f, 0.287999988f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.458999991f, 0.000000000f, 0.541000009f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.458999991f, 0.000000000f, 0.541000009f },
	/* 8 x 8 */
	{ 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f }
}
