

#ifndef __BITMAP_INFOMATION_H__
#define __BITMAP_INFOMATION_H__

class BitmapInfomation {
	tjs_uint bpp_;
	tjs_int width_;
	tjs_int height_;

	tjs_uint image_size_;
	tjs_uint bitmap_width_;

public:
	BitmapInfomation( tjs_uint width, tjs_uint height, int bpp );
	~BitmapInfomation();

	inline unsigned int GetBPP() const { return bpp_; }
	inline bool Is32bit() const { return GetBPP() == 32; }
	inline bool Is8bit() const { return GetBPP() == 8; }
	inline int GetWidth() const { return width_; }
	inline int GetHeight() const { return height_; }
	inline tjs_uint GetImageSize() const { return image_size_; }
	inline int GetPitchBytes() const { return GetImageSize()/GetHeight(); }
	BitmapInfomation& operator=(BitmapInfomation& r) {
		bpp_ = r.bpp_;
		width_ = r.width_;
		height_ = r.height_;
		image_size_ = r.image_size_;
		bitmap_width_ = r.bitmap_width_;
		return *this;
	}
};

#endif // __BITMAP_INFOMATION_H__

