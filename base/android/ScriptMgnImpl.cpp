//---------------------------------------------------------------------------
// TJS2 Script Managing
//---------------------------------------------------------------------------
#include "tjsCommHead.h"
#include <memory>
#include <android/asset_manager.h>
#include "Application.h"

//---------------------------------------------------------------------------
// Hash Map Object を書き出すためのサブプロセスとして起動しているかどうか
// チェックする
// Windows 以外では、ないものとして扱ってもいいか
bool TVPCheckProcessLog() { return false; }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Script system initialization script
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ttstr TVPGetSystemInitializeScript()
{
	ttstr origname(TJS_W("SysInitScript.tjs"));
	AAssetManager* mgr = Application->getAssetManager();
	AAsset* asset = AAssetManager_open( mgr, "SysInitScript.tjs", AASSET_MODE_RANDOM );
	if( asset == nullptr ) {
		TVPThrowExceptionMessage(TVPCannotOpenStorage, origname);
	}
	AAsset_seek64( asset, 0, SEEK_SET );
	tjs_uint64 len = AAsset_getLength64( asset );
	std::unique_ptr<tjs_char[]> tmp(new tjs_char[len+1]);
	AAsset_read( asset, tmp.get(), len );
	tmp[len] = TJS_W('\0');
	return ttstr( tmp.get() );
	//return ttstr(TVPInitTJSScript);
}
//---------------------------------------------------------------------------
