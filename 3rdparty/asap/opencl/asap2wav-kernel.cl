typedef struct {
	long wav_offset;
	int wav_len;
	int filename_offset;
	int module_offset;
	int module_len;
	int song;
	int duration;
} ASAP_Workload;

kernel void asap2wav(constant char *filenames, global const uchar *modules, global uchar *wavs, constant ASAP_Workload *workloads)
{
	constant ASAP_Workload *workload = workloads + get_global_id(0);
	global uchar *wav = wavs + workload->wav_offset;
	ASAP asap;
	ASAP_Construct(&asap);
	if (ASAP_Load(&asap, filenames + workload->filename_offset, modules + workload->module_offset, workload->module_len)
	 && ASAP_PlaySong(&asap, max(workload->song, 0), workload->duration)) {
		int header_len = ASAP_GetWavHeader(&asap, wav, ASAPSampleFormat_S16_L_E, false);
		ASAP_Generate(&asap, wav + header_len, workload->wav_len - header_len, ASAPSampleFormat_S16_L_E);
	}
	else
		wav[0] = '\0';
}
