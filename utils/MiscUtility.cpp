
#include "tjsCommHead.h"
#include "CharacterSet.h"

//---------------------------------------------------------------------------
bool TVPEncodeUTF8ToUTF16( tjs_string &output, const std::string &source )
{
	tjs_int len = TVPUtf8ToWideCharString( source.c_str(), NULL );
	if(len == -1) return false;
	std::vector<tjs_char> outbuf( len+1, 0 );
	tjs_int ret = TVPUtf8ToWideCharString( source.c_str(), &(outbuf[0]));
	if( ret ) {
		outbuf[ret] = TJS_W('\0');
		output = tjs_string( &(outbuf[0]) );
		return true;
	}
	return false;
}
