
#ifndef __ICON_FILE_H__
#define __ICON_FILE_H__

#include <windows.h>
#include <assert.h>

// 参考 https://msdn.microsoft.com/en-us/library/ms997538.aspx
typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
//    ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD   dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

// 参考 http://www.atmarkit.co.jp/bbs/phpBB/viewtopic.php?topic=26073&forum=7&start=16
class IconFile {
public:
	enum {
		ERR_ICON_SUCCESS = 1,
		ERR_ICON_FILE_OPEN_ERROR = -1,
		ERR_ICON_FILE_READ_ERROR = -2,
		ERR_ICON_INVALID_FILE_ERROR = -3,
	};

private:
	ICONDIR			dir_;
	ICONDIRENTRY*	entry_;
	BYTE**			image_;
	BYTE*			groupData_;

public:
	IconFile() : entry_(NULL), image_(NULL), groupData_(NULL) {
		memset(&dir_, 0, sizeof(dir_));
	}
	virtual ~IconFile() {
		if( image_ ) {
			for( int i = 0; i < dir_.idCount; i++ ){
				delete image_[i];
			}
		}
		delete image_;
		delete entry_;
		delete groupData_;
	}

	int GetImageCount() const { return dir_.idCount; }

	BYTE* GetImageData(int index) const {
		if( (0 <= index && index < GetImageCount()) && (image_ != NULL) ) {
			return image_[index];
		} else {
			return NULL;
		}
	}

	DWORD GetImageSize(int index) const {
		if( (0 <= index && index < GetImageCount()) && (image_ != NULL) ) {
			return entry_[index].dwBytesInRes;
		} else {
			return 0;
		}
	}

	int Load(LPCTSTR pszFileName);

	int SizeOfIconGroupData() const {
		return sizeof(ICONDIR) + sizeof(GRPICONDIRENTRY) * GetImageCount();
	}

	BYTE* CreateIconGroupData( int nBaseID );
};

#endif // __ICON_FILE_H__
