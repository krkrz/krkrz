//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Definition of Messages and Message Related Utilities
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <android/asset_manager.h>
#include <string>
#include <iostream>
#include "MsgIntf.h"
#include "MsgImpl.h"
#include "Application.h"
#include "CharacterSet.h"

extern void TVPGetFileVersionOf( tjs_int& major, tjs_int& minor, tjs_int& release, tjs_int& build );
//---------------------------------------------------------------------------
// version retrieving
//---------------------------------------------------------------------------
void TVPGetVersion(void)
{
	static bool DoGet=true;
	if(DoGet)
	{
		DoGet = false;

		TVPVersionMajor = 0;
		TVPVersionMinor = 0;
		TVPVersionRelease = 0;
		TVPVersionBuild = 0;

		TVPGetFileVersionOf( TVPVersionMajor, TVPVersionMinor, TVPVersionRelease, TVPVersionBuild);
	}
}
//---------------------------------------------------------------------------
// about string retrieving
//---------------------------------------------------------------------------
extern const tjs_char* TVPCompileDate;
extern const tjs_char* TVPCompileTime;
ttstr TVPReadAboutStringFromResource() {
	AAsset* asset = AAssetManager_open( Application->getAssetManager(), "license.txt", AASSET_MODE_RANDOM );
	char *buf = nullptr;
	if( asset == nullptr ) return ttstr(TJS_W("Resource Read Error."));

	tjs_uint64 flen = AAsset_getLength64( asset );
	buf = new char[flen];
	if( buf == nullptr ) {
		AAsset_close( asset );
		return ttstr(TJS_W("Resource Read Error."));
	}
	tjs_uint rsize = AAsset_read( asset, buf, flen );
	AAsset_close( asset );
	if( flen != rsize ) {
		delete[] buf;
		return ttstr(TJS_W("Resource Read Error."));
	}

	// UTF-8 to UTF-16
	size_t len = TVPUtf8ToWideCharString( buf, NULL );
	if( len <= 0 ) {
		delete[] buf;
		return ttstr(TJS_W("Resource Read Error."));
	}
	tjs_char* tmp = new tjs_char[len+1];
	ttstr ret;
	if( tmp ) {
		try {
			len = TVPUtf8ToWideCharString( buf, tmp );
		} catch(...) {
			delete[] tmp;
			delete[] buf;
			throw;
		}
		tmp[len] = 0;

		size_t datelen = TJS_strlen( TVPCompileDate );
		size_t timelen = TJS_strlen( TVPCompileTime );

		// CR to CR-LF, %DATE% and %TIME% to compile data and time
		std::vector<tjs_char> tmp2;
		tmp2.reserve( len * 2 + datelen + timelen );
		for( size_t i = 0; i < len; i++ ) {
			if( tmp[i] == '%' && (i+6) < len && tmp[i+1] == 'D' && tmp[i+2] == 'A' && tmp[i+3] == 'T' && tmp[i+4] == 'E' && tmp[i+5] == '%' ) {
				for( size_t j = 0; j < datelen; j++ ) {
					tmp2.push_back( TVPCompileDate[j] );
				}
				i += 5;
			} else if( tmp[i] == '%' && (i+6) < len && tmp[i+1] == 'T' && tmp[i+2] == 'I' && tmp[i+3] == 'M' && tmp[i+4] == 'E' && tmp[i+5] == '%' ) {
				for( size_t j = 0; j < timelen; j++ ) {
					tmp2.push_back( TVPCompileTime[j] );
				}
				i += 5;
			} else if( tmp[i] != TJS_W('\n') ) {
				tmp2.push_back( tmp[i] );
			} else {
				tmp2.push_back( TJS_W('\r') );
				tmp2.push_back( TJS_W('\n') );
			}
		}
		tmp2.push_back( 0 );
		ret = ttstr( &(tmp2[0]) );
		delete[] tmp;
	}
	delete[] buf;
	return ret;
}


