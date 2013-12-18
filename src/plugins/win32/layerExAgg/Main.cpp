#include <stdio.h>
#include <windows.h>
#include <list>
#include <map>
using namespace std;

#include "tp_stub.h"

static const char *copyright = 
"----- AntiGrainGeometry Copyright START -----\n"
"Anti-Grain Geometry - Version 2.3\n"
"Copyright (C) 2002-2005 Maxim Shemanarev (McSeem)\n"
"\n"
"Permission to copy, use, modify, sell and distribute this software\n"
"is granted provided this copyright notice appears in all copies. \n"
"This software is provided \"as is\" without express or implied\n"
"warranty, and with no claim as to its suitability for any purpose.\n"
"----- AntiGrainGeometry Copyright END -----\n";

/**
 * ログ出力用
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "LayerExBase.h"
#include "Primitive.hpp"

/**
 * Anti-Grain Geometry プリミティブネイティブインスタンス
 */
class NI_AGGPrimitive : public tTJSNativeInstance // ネイティブインスタンス
{
	friend class NI_AntiGrainGeometry;
protected:
	iTJSDispatch2 * _layer;
public:
	AGGPrimitive * _primitive;
	NI_AGGPrimitive();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();
	void redraw();
};

void AGGPrimitive::redraw()
{
	owner->redraw();
}

/*
 * レイヤに直結した Anti-Grain Geometry 情報を保持するためのネイティブインスタンス
 */
class NI_AntiGrainGeometry : public tTJSNativeInstance
{
	friend class NI_AGGPrimitive;
	
protected:
	iTJSDispatch2 * _layerobj;

protected:
	/// 再描画
	bool _redraw;
public:
	/**
	 * 再描画要請
	 */
	void redraw() {
		if (!_redraw) {
			_redraw = true;
			NI_LayerExBase *base;
			if ((base = NI_LayerExBase::getNative(_layerobj))) {
				base->redraw(_layerobj);
			}
		}
	}

	//-------------------------------------------
	// 表示位置
	//-------------------------------------------
protected:
	/// 表示位置X
	double _x;
	/// 表示位置Y
	double _y;
	/// 拡大
	double _scale;
	/// 回転
	double _rotate;

public:
	inline double getX() { return _x; };
	inline void setX(double x) { _x = x; redraw(); };

	inline double getY() { return _y; };
	inline void setY(double y) { _y = y; redraw(); };

	inline double getRotate() { return _rotate; };
	inline void setRotate(double rotate) { _rotate = rotate; redraw(); };

	double getScale() { return _scale; };
	inline void setScale(double scale) { _scale = scale; redraw(); };
	
	/*
	 * 座標指定
	 * @param x X移動量
	 * @param y Y移動量
	 */
	void setPos(double x, double y) {
		_x = x;
		_y = y;
		redraw();
	}

	//-------------------------------------------
	// 描画要素
	//-------------------------------------------
protected:
	list<NI_AGGPrimitive*> _primitives;
public:
	void addPrimitive(NI_AGGPrimitive *primitive) {
		_primitives.push_back(primitive);
		redraw();
	}
	void removePrimitive(NI_AGGPrimitive *primitive) {
		_primitives.remove(primitive);
		redraw();
	}
	
public:
	/**
	 * 塗りなおし処理
	 */
	void onPaint() {

		if (_redraw) {

			NI_LayerExBase *base;
			if ((base = NI_LayerExBase::getNative(_layerobj))) {
				base->reset(_layerobj);

				// AGG 用に先頭位置に補正
				unsigned char *buffer = base->_buffer;
				if (base->_pitch < 0) {
					buffer += int(base->_height - 1) * base->_pitch;
				}
				
				/// レンダリング用バッファ
				agg::rendering_buffer rbuf(buffer, base->_width, base->_height, base->_pitch);
				
				// レンダラの準備
				pixfmt pixf(rbuf);
				renderer_base rb(pixf);
				
				// 一度消去する XXX フラグをたてたら消去しなくする？
				rb.clear(color_type(0,0,0,0));
				
				// 変形処理
				agg::trans_affine mtx;
				mtx *= agg::trans_affine_translation((base->_width) * -0.5, (base->_height) * -0.5);
				mtx *= agg::trans_affine_scaling(_scale);
				mtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
				mtx *= agg::trans_affine_translation(base->_width * 0.5 + _x, base->_height * 0.5 + _y);
				
				// プリミティブの再描画
				{
					list<NI_AGGPrimitive*>::iterator i =  _primitives.begin();
					while (i != _primitives.end()) {
						(*i)->_primitive->paint(rb, mtx);
						i++;
					}
				}
			}
			
			_redraw = false;
		}
	}

public:
	/**
	 * コンストラクタ
	 */
	NI_AntiGrainGeometry(iTJSDispatch2 *layerobj) {
		_layerobj = layerobj; // no addRef
		_redraw = false;
		_x = 0;
		_y = 0;
		_scale = 1.0;
		_rotate = 0;
	}
};

// クラスID
static tjs_int32 ClassID_AntiGrainGeometry = -1;

/**
 * Layer の onPaint 処理ののっとり用ファンクション
 */
class tOnPaintFunction : public tTJSDispatch
{
protected:
	/// 元のメソッド
	iTJSDispatch2 *original;
public:
	/// コンストラクタ
	tOnPaintFunction(iTJSDispatch2 *original) : original(original) {}
	
	/// デストラクタ
	~tOnPaintFunction() {
		if (original) {
			original->Release();
		}
	}

	/// 関数呼び出し
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
};

void
addMember(iTJSDispatch2 *dispatch, const tjs_char *memberName, iTJSDispatch2 *member)
{
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		memberName, // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&var, // 登録する値
		dispatch // コンテキスト
		);
}

/**
 * レイヤオブジェクトから Anti-Grain Geometry 用ネイティブインスタンスを取得する。
 * ネイティブインスタンスを持ってない場合は自動的に割り当てる
 * @param objthis レイヤオブジェクト
 * @return Anti-Grain Geometry 用ネイティブインスタンス。取得失敗したら NULL
 */
static NI_AntiGrainGeometry *
getAntiGrainGeometryNative(iTJSDispatch2 *layerobj)
{
	if (!layerobj) return NULL;

	NI_AntiGrainGeometry *_this;
	if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
												   ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&_this))) {

		// レイヤ拡張部生成
		if (NI_LayerExBase::getNative(layerobj) == NULL) {
			return NULL;
		}
	
		_this = new NI_AntiGrainGeometry(layerobj);
		if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_REGISTER,
													   ClassID_AntiGrainGeometry, (iTJSNativeInstance **)&_this))) {
			delete _this;
			return NULL;
		}

		// onPaint をのっとる
		{
			const tjs_char *memberName = TJS_W("onPaint");
			tTJSVariant var;
			if (layerobj->PropGet(0, memberName, NULL, &var, layerobj) == TJS_S_OK) {
				addMember(layerobj, memberName,  new tOnPaintFunction(var.AsObject()));
			} else {
				addMember(layerobj, memberName,  new tOnPaintFunction(NULL));
			}
		}		
	}

	return _this;
}

	/// 関数呼び出し
tjs_error TJS_INTF_METHOD
tOnPaintFunction::FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
						   tTJSVariant *result,
						   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	tjs_error ret;
	if (original) {
		ret = original->FuncCall(flag, membername, hint, result, numparams, param, objthis);
	} else {
		ret = TJS_S_OK;
	}
	if (ret == TJS_S_OK) {
		NI_AntiGrainGeometry *_this;
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		_this->onPaint();
	}
	return ret;
}


#define FUNC(funcname,pnum) \
class funcname : public tTJSDispatch\
{\
protected:\
public:\
	tjs_error TJS_INTF_METHOD FuncCall(\
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,\
		tTJSVariant *result,\
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;\
		if (numparams < pnum) return TJS_E_BADPARAMCOUNT;

#define FUNCEND \
		return TJS_S_OK;\
	}\
};

#define PROP(funcname) \
class funcname : public tTJSDispatch\
{\
protected:\
public:

#define GETTER \
	tjs_error TJS_INTF_METHOD PropGet(\
		tjs_uint32 flag,\
		const tjs_char * membername,\
		tjs_uint32 *hint,\
		tTJSVariant *result,\
		iTJSDispatch2 *objthis)	{\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;\
		

#define SETTER \
        return TJS_S_OK;\
    }\
	tjs_error TJS_INTF_METHOD PropSet(\
		tjs_uint32 flag,\
		const tjs_char *membername,\
		tjs_uint32 *hint,\
		const tTJSVariant *param,\
		iTJSDispatch2 *objthis)	{\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
#define PROPEND \
		return TJS_S_OK;\
	}\
};

FUNC(tSetPosFunction,2)
	_this->setPos(*param[0], *param[1]);
FUNCEND

PROP(tXProp)
GETTER
	*result = _this->getX();
SETTER
	_this->setX(*param);
FUNCEND

PROP(tYProp)
GETTER
	*result = _this->getY();
SETTER
	_this->setY(*param);
FUNCEND
	
PROP(tRotateProp)
GETTER
	*result = _this->getRotate();
SETTER
	_this->setRotate(*param);
FUNCEND

PROP(tScaleProp)
GETTER
	*result = _this->getScale();
SETTER
	_this->setScale(*param);
PROPEND

//---------------------------------------------------------------------------

// 型登録用
list<RegistType*> *typeList = NULL;
map<ttstr,RegistType*> typeMap;

void registType(RegistType *type)
{
	if (typeList == NULL) {
		typeList = new list<RegistType*>;
	}
	typeList->push_back(type);
}

/**
 * プリミティブネイティブインスタンスメソッド
 */
NI_AGGPrimitive::NI_AGGPrimitive()
{
	_layer = NULL;
	_primitive = NULL;
}
	
tjs_error TJS_INTF_METHOD
NI_AGGPrimitive::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;

	if (param[0]->Type() != tvtObject || param[0]->AsObjectNoAddRef()->IsInstanceOf(0,NULL,NULL,L"Layer",param[0]->AsObjectNoAddRef()) == false) {
		TVPThrowExceptionMessage(TJS_W("first parameter must Layer."));
	}
	
	// 親レイヤ
	_layer = param[0]->AsObject();
	try {
		NI_AntiGrainGeometry *agg;

		// 親レイヤから AGG 情報インスタンスを取得。無い場合は生成する。
		if ((agg = getAntiGrainGeometryNative(_layer)) == NULL) {
			TVPThrowExceptionMessage(TJS_W("failed to get AGG Instance from Layer."));
		}

		// オブジェクト生成
		map<ttstr,RegistType*>::const_iterator n = typeMap.find(*param[1]);
		if (n != typeMap.end()) {
			_primitive = n->second->create(this, numparams-2, param+2, tjs_obj);
		} else {
			TVPThrowExceptionMessage((ttstr(L"failed to create ") + (ttstr)*param[1]).c_str());
		}

		// 親に自分を登録
		agg->addPrimitive(this);

	} catch (...) {
		_layer->Release();
		_layer = NULL;
		throw;
	}

	return TJS_S_OK;
}

void TJS_INTF_METHOD
NI_AGGPrimitive::Invalidate()
{
	if (_layer) {
		NI_AntiGrainGeometry *agg;
		if (TJS_SUCCEEDED(_layer->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														  ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&agg))) {
			// 親から自分を消去
			agg->removePrimitive(this);
		} else {
			log(TJS_W("failed to get AGG Instance from Layer"));
		}
		_layer->Release();
	}
	delete _primitive;
};

void
NI_AGGPrimitive::redraw()
{
	if (_layer) {
		NI_AntiGrainGeometry *agg;
		if (TJS_SUCCEEDED(_layer->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														  ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&agg))) {
			// 親を再描画
			agg->redraw();
		} else {
			log(TJS_W("failed to get AGG Instance from Layer"));
		}
	}
}

//---------------------------------------------------------------------------
/*
	これは NI_AGGPrimitive のオブジェクトを作成して返すだけの関数です。
	後述の TJSCreateNativeClassForPlugin の引数として渡します。
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_AGGPrimitive()
{
	return new NI_AGGPrimitive();
}
//---------------------------------------------------------------------------
/*
	TJS2 のネイティブクラスは一意な ID で区別されている必要があります。
	これは後述の TJS_BEGIN_NATIVE_MEMBERS マクロで自動的に取得されますが、
	その ID を格納する変数名と、その変数をここで宣言します。
	初期値には無効な ID を表す -1 を指定してください。
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_AGGPrimitive
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 用の「クラス」を作成して返す関数です。
*/
static iTJSDispatch2 * Create_NC_AGGPrimitive()
{
	/// クラスオブジェクトの作成
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("AGGPrimitive"), Create_NI_AGGPrimitive);

	/// メンバ定義
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/AGGPrimitive)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_AGGPrimitive,
			/*TJS class name*/AGGPrimitive)
		{
			return TJS_S_OK;
		}
	    TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/AGGPrimitive)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setPos)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_AGGPrimitive);
			if (numparams < 2) return TJS_E_BADPARAMCOUNT;
			_this->_primitive->setPos(*param[0], *param[1]);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setPos)

		TJS_BEGIN_NATIVE_PROP_DECL(x)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getX();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setX(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(x)

		TJS_BEGIN_NATIVE_PROP_DECL(y)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getY();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setY(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(y)

		TJS_BEGIN_NATIVE_PROP_DECL(rotate)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getRotate();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setRotate(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(rotate)

		TJS_BEGIN_NATIVE_PROP_DECL(scale)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getScale();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setScale(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(scale)

	TJS_END_NATIVE_MEMBERS

	return classobj;
}

/**
 * レイヤオブジェクトから Anti-Grain Geometry 用ネイティブインスタンスを取得する。
 * ネイティブインスタンスを持ってない場合は自動的に割り当てる
 * @param objthis レイヤオブジェクト
 * @return 該当プリミティブの AGGPrimitive 。取得失敗したら NULL
 */
AGGPrimitive *
getAGGPrimitive(iTJSDispatch2 *obj)
{
	if (!obj) return NULL;
	NI_AGGPrimitive *_this;
	if (TJS_FAILED(obj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
											  TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
		return NULL;
	}
	return _this->_primitive;
}


#undef TJS_NATIVE_CLASSID_NAME


//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	TVPAddImportantLog(ttstr(copyright));
	
	// 型リストを型マップに変換
	{
		list<RegistType*>::iterator i = typeList->begin();
		while (i != typeList->end()) {
			typeMap[ttstr((*i)->getTypeName())] = *i;
			i++;
		}
	}

	// クラスオブジェクトチェック
	if ((NI_LayerExBase::classId = TJSFindNativeClassID(L"LayerExBase")) <= 0) {
		NI_LayerExBase::classId = TJSRegisterNativeClass(L"LayerExBase");
	}
	
	// クラスオブジェクト登録
	ClassID_AntiGrainGeometry = TJSRegisterNativeClass(TJS_W("AntiGrainGeometry"));

	{
		// TJS のグローバルオブジェクトを取得する
		iTJSDispatch2 * global = TVPGetScriptDispatch();

		// Layer クラスオブジェクトを取得
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Layer"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			// プロパティ初期化
			NI_LayerExBase::init(dispatch);

			// 専用メソッドの追加
			addMember(dispatch, L"aggSetPos", new tSetPosFunction());
			addMember(dispatch, L"aggRotate", new tRotateProp());
			addMember(dispatch, L"aggScale",  new tScaleProp());
			addMember(dispatch, L"aggX",      new tXProp());
			addMember(dispatch, L"aggY",      new tYProp());
		}

		// AGGPrimitive 型登録
		addMember(global, L"AGGPrimitive", Create_NC_AGGPrimitive());
		
		global->Release();
	}
			
	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で
	// 管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、
	// プラグインは解放できないと言うことになる。

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	// プロパティ開放
	NI_LayerExBase::unInit();
	
	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if (global) {
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする
		global->Release();
	}


	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
