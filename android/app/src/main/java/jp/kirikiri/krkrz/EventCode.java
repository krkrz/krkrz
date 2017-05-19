package jp.kirikiri.krkrz;

public class EventCode {
	public static final int AM_START	= 0x010001;
	public static final int AM_RESTART	= 0x010002;
	public static final int AM_RESUME	= 0x010003;
	public static final int AM_PAUSE	= 0x010004;
	public static final int AM_STOP		= 0x010005;
	public static final int AM_DESTROY	= 0x010006;

	public static final int AM_SURFACE_CHANGED		= 0x010011;
	public static final int AM_SURFACE_CREATED		= 0x010012;
	public static final int AM_SURFACE_DESTORYED	= 0x010013;
	public static final int AM_SURFACE_PAINT_REQUEST	= 0x010014;

	public static final int AM_MOVIE_ENDED			= 0x010021;
	public static final int AM_MOVIE_PLAYER_ERROR	= 0x010022;
	public static final int AM_MOVIE_LOAD_ERROR	= 0x010023;
	public static final int AM_MOVIE_BUFFERING	= 0x010024;
	public static final int AM_MOVIE_IDLE			= 0x010025;
	public static final int AM_MOVIE_READY			= 0x010026;
	public static final int AM_MOVIE_PLAY			= 0x010027;

	public static final int AM_KEY_DOWN 		= 0x010031;
	public static final int AM_KEY_UP			= 0x010032;
};
