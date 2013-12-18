#include "ncbind/ncbind.hpp"

static const char *copyright = 
"----- CxImage Copyright START -----\n"
"CxImage version 7.0.2 07/Feb/2011\n"
"CxImage : Copyright (C) 2001 - 2011, Davide Pizzolato\n"
"Original CImage and CImageIterator implementation are:\n"
"Copyright (C) 1995, Alejandro Aguilar Sierra (asierra(at)servidor(dot)unam(dot)mx)\n"
"----- CxImage Copyright END -----\n";

#include "LayerExImage.h"

// ----------------------------------- クラスの登録

NCB_GET_INSTANCE_HOOK(layerExImage)
{
	// インスタンスゲッタ
	NCB_INSTANCE_GETTER(objthis) { // objthis を iTJSDispatch2* 型の引数とする
		ClassT* obj = GetNativeInstance(objthis);	// ネイティブインスタンスポインタ取得
		if (!obj) {
			obj = new ClassT(objthis);				// ない場合は生成する
			SetNativeInstance(objthis, obj);		// objthis に obj をネイティブインスタンスとして登録する
		}
		obj->reset();
		return obj;
	}
	// デストラクタ（実際のメソッドが呼ばれた後に呼ばれる）
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// フックつきアタッチ
NCB_ATTACH_CLASS_WITH_HOOK(layerExImage, Layer) {
	NCB_METHOD(light);
	NCB_METHOD(colorize);
	NCB_METHOD(modulate);
	NCB_METHOD(noise);
	NCB_METHOD(generateWhiteNoise);
	NCB_METHOD(gaussianBlur);
}

void init()
{
	TVPAddImportantLog(ttstr(copyright));
}

NCB_PRE_REGIST_CALLBACK(init);
