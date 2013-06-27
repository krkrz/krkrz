
class BitmapInfomation {
public:
	BitmapInfomation( tjs_uint width, tjs_uint height, int bpp );
	~BitmapInfomation();

	inline unsigned int GetBPP() const;
	inline bool Is32bit() const;
	inline bool Is8bit() const;
	inline int GetWidth() const;
	inline int GetHeight() const;
	inline tjs_uint GetImageSize() const;
	inline int GetPitchBytes() const;

	BitmapInfomation& operator=(BitmapInfomation& r);
};

