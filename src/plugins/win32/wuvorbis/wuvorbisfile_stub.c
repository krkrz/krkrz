extern int  _cdecl wu_ov_clear (wu_OggVorbis_File *vf){
	return (int) ov_clear((OggVorbis_File *)vf);
}
extern int  _cdecl wu_ov_open_callbacks (void *datasource, wu_OggVorbis_File *vf,		char *initial, long ibytes, wu_ov_callbacks callbacks){
	return (int) ov_open_callbacks((void *)datasource,(OggVorbis_File *)vf,(char *)initial,(long)ibytes,*(ov_callbacks*)&callbacks);
}
extern int  _cdecl wu_ov_test_callbacks (void *datasource, wu_OggVorbis_File *vf,		char *initial, long ibytes, wu_ov_callbacks callbacks){
	return (int) ov_test_callbacks((void *)datasource,(OggVorbis_File *)vf,(char *)initial,(long)ibytes,*(ov_callbacks*)&callbacks);
}
extern int  _cdecl wu_ov_test_open (wu_OggVorbis_File *vf){
	return (int) ov_test_open((OggVorbis_File *)vf);
}
extern long  _cdecl wu_ov_bitrate (wu_OggVorbis_File *vf,int i){
	return (long) ov_bitrate((OggVorbis_File *)vf,(int)i);
}
extern long  _cdecl wu_ov_bitrate_instant (wu_OggVorbis_File *vf){
	return (long) ov_bitrate_instant((OggVorbis_File *)vf);
}
extern long  _cdecl wu_ov_streams (wu_OggVorbis_File *vf){
	return (long) ov_streams((OggVorbis_File *)vf);
}
extern long  _cdecl wu_ov_seekable (wu_OggVorbis_File *vf){
	return (long) ov_seekable((OggVorbis_File *)vf);
}
extern long  _cdecl wu_ov_serialnumber (wu_OggVorbis_File *vf,int i){
	return (long) ov_serialnumber((OggVorbis_File *)vf,(int)i);
}
extern wu_ogg_int64_t  _cdecl wu_ov_raw_total (wu_OggVorbis_File *vf,int i){
	return (wu_ogg_int64_t) ov_raw_total((OggVorbis_File *)vf,(int)i);
}
extern wu_ogg_int64_t  _cdecl wu_ov_pcm_total (wu_OggVorbis_File *vf,int i){
	return (wu_ogg_int64_t) ov_pcm_total((OggVorbis_File *)vf,(int)i);
}
extern double  _cdecl wu_ov_time_total (wu_OggVorbis_File *vf,int i){
	return (double) ov_time_total((OggVorbis_File *)vf,(int)i);
}
extern int  _cdecl wu_ov_raw_seek (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_raw_seek((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_pcm_seek (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_pcm_seek((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_pcm_seek_page (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_pcm_seek_page((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_time_seek (wu_OggVorbis_File *vf,double pos){
	return (int) ov_time_seek((OggVorbis_File *)vf,(double)pos);
}
extern int  _cdecl wu_ov_time_seek_page (wu_OggVorbis_File *vf,double pos){
	return (int) ov_time_seek_page((OggVorbis_File *)vf,(double)pos);
}
extern int  _cdecl wu_ov_raw_seek_lap (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_raw_seek_lap((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_pcm_seek_lap (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_pcm_seek_lap((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_pcm_seek_page_lap (wu_OggVorbis_File *vf,wu_ogg_int64_t pos){
	return (int) ov_pcm_seek_page_lap((OggVorbis_File *)vf,(ogg_int64_t)pos);
}
extern int  _cdecl wu_ov_time_seek_lap (wu_OggVorbis_File *vf,double pos){
	return (int) ov_time_seek_lap((OggVorbis_File *)vf,(double)pos);
}
extern int  _cdecl wu_ov_time_seek_page_lap (wu_OggVorbis_File *vf,double pos){
	return (int) ov_time_seek_page_lap((OggVorbis_File *)vf,(double)pos);
}
extern wu_ogg_int64_t  _cdecl wu_ov_raw_tell (wu_OggVorbis_File *vf){
	return (wu_ogg_int64_t) ov_raw_tell((OggVorbis_File *)vf);
}
extern wu_ogg_int64_t  _cdecl wu_ov_pcm_tell (wu_OggVorbis_File *vf){
	return (wu_ogg_int64_t) ov_pcm_tell((OggVorbis_File *)vf);
}
extern double  _cdecl wu_ov_time_tell (wu_OggVorbis_File *vf){
	return (double) ov_time_tell((OggVorbis_File *)vf);
}
extern wu_vorbis_info * _cdecl wu_ov_info (wu_OggVorbis_File *vf,int link){
	return (wu_vorbis_info *) ov_info((OggVorbis_File *)vf,(int)link);
}
extern wu_vorbis_comment * _cdecl wu_ov_comment (wu_OggVorbis_File *vf,int link){
	return (wu_vorbis_comment *) ov_comment((OggVorbis_File *)vf,(int)link);
}
extern long  _cdecl wu_ov_read_float (wu_OggVorbis_File *vf,float ***pcm_channels,int samples,			  int *bitstream){
	return (long) ov_read_float((OggVorbis_File *)vf,(float ***)pcm_channels,(int)samples,(int *)bitstream);
}
extern long  _cdecl wu_ov_read (wu_OggVorbis_File *vf,char *buffer,int length,		    int bigendianp,int word,int sgned,int *bitstream){
	return (long) ov_read((OggVorbis_File *)vf,(char *)buffer,(int)length,(int)bigendianp,(int)word,(int)sgned,(int *)bitstream);
}
extern int  _cdecl wu_ov_crosslap (wu_OggVorbis_File *vf1,wu_OggVorbis_File *vf2){
	return (int) ov_crosslap((OggVorbis_File *)vf1,(OggVorbis_File *)vf2);
}
extern int  _cdecl wu_ov_halfrate (wu_OggVorbis_File *vf,int flag){
	return (int) ov_halfrate((OggVorbis_File *)vf,(int)flag);
}
extern int  _cdecl wu_ov_halfrate_p (wu_OggVorbis_File *vf){
	return (int) ov_halfrate_p((OggVorbis_File *)vf);
}
extern void  _cdecl wu_SetCPUType (unsigned __int32 type){
	SetCPUType((unsigned __int32)type);
}
extern unsigned __int32  _cdecl wu_DetectCPU (void){
	return (unsigned __int32) DetectCPU();
}
extern void  _cdecl wu_ScaleOutput (float scale){
	ScaleOutput((float)scale);
}
