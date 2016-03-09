
#include "IconFile.h"

int IconFile::Load(LPCTSTR pszFileName) {
	HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return ERR_ICON_FILE_OPEN_ERROR;

	DWORD dwRead;
	BOOL result = ::ReadFile(hFile, &dir_, sizeof(ICONDIR), &dwRead, NULL);
	if( result == FALSE ) return ERR_ICON_FILE_READ_ERROR;

	if( dir_.idReserved != 0 || dir_.idType != 1 || dir_.idCount <= 0 ) return ERR_ICON_INVALID_FILE_ERROR;

	entry_ = new ICONDIRENTRY[dir_.idCount];
	for( int i = 0; i < dir_.idCount; i++ ) {
		result = ::ReadFile(hFile, &(entry_[i]), sizeof(ICONDIRENTRY), &dwRead, NULL);
		if( result == FALSE ) return ERR_ICON_FILE_READ_ERROR;
	}

	image_ = new BYTE*[dir_.idCount];
	for( int i = 0; i < dir_.idCount; i++ ) {
		::SetFilePointer(hFile, entry_[i].dwImageOffset, NULL, FILE_BEGIN);

		image_[i] = new BYTE[entry_[i].dwBytesInRes];
		result = ::ReadFile(hFile, image_[i], entry_[i].dwBytesInRes, &dwRead, NULL);
		if( result == FALSE ) return ERR_ICON_FILE_READ_ERROR;
	}
	::CloseHandle(hFile);

	return ERR_ICON_SUCCESS;
}

BYTE* IconFile::CreateIconGroupData( int nBaseID ) {
	if( groupData_ ) delete groupData_;

	groupData_ = new BYTE[SizeOfIconGroupData()];
	memcpy(groupData_, &dir_, sizeof(ICONDIR));

	int offset = sizeof(ICONDIR);
	for (int i = 0; i < GetImageCount(); i++) {
		GRPICONDIRENTRY grpEntry;

		BITMAPINFOHEADER bitmapheader;
		memcpy(&bitmapheader, GetImageData(i), sizeof(BITMAPINFOHEADER));

		grpEntry.bWidth			= entry_[i].bWidth;
		grpEntry.bHeight		= entry_[i].bHeight;
		grpEntry.bColorCount	= entry_[i].bColorCount;
		grpEntry.bReserved		= entry_[i].bReserved;
		grpEntry.wPlanes		= bitmapheader.biPlanes;
		grpEntry.wBitCount		= bitmapheader.biBitCount;
		grpEntry.dwBytesInRes	= entry_[i].dwBytesInRes;
		grpEntry.nID			= nBaseID + i;
		memcpy(groupData_ + offset, &grpEntry, sizeof(GRPICONDIRENTRY));

		offset += sizeof(GRPICONDIRENTRY);
	}

	return groupData_;
}

