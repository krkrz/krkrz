
#include "tjsCommHead.h"

#include "tjsError.h"

void TVPThrowWindowsErrorException() {
	ttstr mes;
	LPVOID lpMsgBuf;
	::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
	mes = ttstr( (LPCWSTR)lpMsgBuf );
	::LocalFree(lpMsgBuf);
	// GetLastError ÇÃÉRÅ[ÉhÇ‡í«â¡ÇµÇΩï˚Ç™Ç¢Ç¢Ç©Ç»
	throw new TJS::eTJSError( mes );
}

void TVPOutputWindowsErrorToDebugMessage() {
	LPVOID lpMsgBuf;
	::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
	::OutputDebugString( (LPCWSTR)lpMsgBuf );
	::LocalFree(lpMsgBuf);
}
