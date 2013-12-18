#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "tp_stub.h"

/**
 * ログ出力用
 */
void
message_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf(msg, 1024, format, args);
	TVPAddLog(ttstr(msg));
	va_end(args);
}

/**
 * エラーログ出力用
 */
void
error_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf(msg, 1024, format, args);
	TVPAddImportantLog(ttstr(msg));
	va_end(args);
}

#include "LayerExSWF.hpp"
#include "SWFMovie.hpp"

////////////////////////////////////////////////////////////////////////////////
/// ncBind 用マクロ

#include "ncbind/ncbind.hpp"

/**
 * ファイル名変換用 proxy
 */
void swfload(SWFMovie *swf, const char *name)
{
	ttstr path(name);
	TVPGetLocalName(path);
	int len = path.GetNarrowStrLen() + 1;
	char *filename = new char[len];
	path.ToNarrowStr(filename, len);
	swf->load(filename);
	delete filename;
}

NCB_REGISTER_CLASS(SWFMovie) {
	NCB_CONSTRUCTOR(());
	NCB_METHOD_PROXY(load, swfload);
	NCB_METHOD(update);
	NCB_METHOD(notifyMouse);
	NCB_METHOD(play);
	NCB_METHOD(stop);
	NCB_METHOD(restart);
	NCB_METHOD(back);
	NCB_METHOD(next);
	NCB_METHOD(gotoFrame);
}

NCB_GET_INSTANCE_HOOK(layerExSWF)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		if (obj) obj->reset();						// メソッドを呼ぶ前に必ず呼ばれる
		return (_obj = obj);						//< デストラクタで使用したい場合はプライベート変数に保存
	}

	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
		if (_obj) _obj->redraw();					// メソッドを呼んだ後に必ず呼ばれる
	}

private:
	ClassT *_obj;
}; // 実体は class 定義なので ; を忘れないでね

// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(layerExSWF, Layer) {
	NCB_METHOD(drawSWF);
}

extern void initSWFMovie();
extern void destroySWFMovie();

NCB_POST_REGIST_CALLBACK(initSWFMovie);
NCB_PRE_UNREGIST_CALLBACK(destroySWFMovie);
