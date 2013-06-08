
#include "tjsCommHead.h"
#include <mmsystem.h>
#include <windows.h>

//---------------------------------------------------------------------------
// TVPGetRoughTickCount
// 32bit値のtickカウントを得る
//---------------------------------------------------------------------------
tjs_uint32 TVPGetRoughTickCount32()
{
	return timeGetTime();
}
//---------------------------------------------------------------------------
bool TVPEncodeUTF8ToUTF16( std::wstring &output, const std::string &source )
{
	int len = ::MultiByteToWideChar( CP_UTF8, 0, source.c_str(), -1, NULL, 0);
	std::vector<wchar_t> outbuf( len+1, 0 );
	int ret = ::MultiByteToWideChar( CP_UTF8, 0, source.c_str(), -1, &(outbuf[0]), len );
	if( ret ) {
		outbuf[ret] = L'\0';
		output = std::wstring( &(outbuf[0]) );
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
tjs_uint TVPEncodeUnicodeToSJIS( tjs_char in ) {
	char local[4];
	wchar_t wide[2] = {in,0};
	int len = WideCharToMultiByte( CP_ACP, 0, wide, 1, local, 4, NULL, NULL );
	if( len > 0 ) {
		tjs_uint ret = 0;
		for( int i = 0; i < len; i++ ) {
			ret <<= 8;
			ret |= local[i];
		}
		return ret;
	}
}
//---------------------------------------------------------------------------
tjs_char TVPEncodeSJISToUnicode(tjs_uint in) {
	char local[5] = {0,0,0,0,0};
	tjs_uint mask = 0xff000000;
	int index = 0;
	for( int i = 0; i < 4; i++ ) {
		if( in & mask ) {
			local[index] =  (in & mask) >> (3-i*8);
			index++;
		}
	}
	wchar_t wide[5] = {0};
	int len = ::MultiByteToWideChar( CP_ACP, 0, local, index, wide, 5 );
	return wide[0];
}
//---------------------------------------------------------------------------

