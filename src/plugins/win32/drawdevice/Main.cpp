//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include "PassThroughDrawDevice.h"

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class NI_PluggedDrawDevice : public tTJSNativeInstance // ネイティブインスタンス
{
	iTVPDrawDevice * Device;
public:
	NI_PluggedDrawDevice()
	{
		// コンストラクタ
		Device = new tTVPPassThroughDrawDevice();
	}

	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
	{
		// TJS2 オブジェクトが作成されるときに呼ばれる

		return S_OK;
	}

	void TJS_INTF_METHOD Invalidate()
	{
		// オブジェクトが無効化されるときに呼ばれる
	}
};
//---------------------------------------------------------------------------
/*
	これは NI_PluggedDrawDevice のオブジェクトを作成して返すだけの関数です。
	後述の TJSCreateNativeClassForPlugin の引数として渡します。
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_PluggedDrawDevice()
{
	return new NI_PluggedDrawDevice();
}
//---------------------------------------------------------------------------
/*
	TJS2 のネイティブクラスは一意な ID で区別されている必要があります。
	これは後述の TJS_BEGIN_NATIVE_MEMBERS マクロで自動的に取得されますが、
	その ID を格納する変数名と、その変数をここで宣言します。
	初期値には無効な ID を表す -1 を指定してください。
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_PluggedDrawDevice
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 用の「クラス」を作成して返す関数です。
*/
static iTJSDispatch2 * Create_NC_PluggedDrawDevice()
{
	/*
		まず、クラスのベースとなるクラスオブジェクトを作成します。
		これには TJSCreateNativeClassForPlugin を用います。
		TJSCreateNativeClassForPlugin の第１引数はクラス名、第２引数は
		ネイティブインスタンスを返す関数を指定します。
		作成したオブジェクトを一時的に格納するローカル変数の名前は
		classobj である必要があります。
	*/
	tTJSNativeClassForPlugin * classobj =
		TJSCreateNativeClassForPlugin(TJS_W("PluggedDrawDevice"), Create_NI_PluggedDrawDevice);


	/*
		TJS_BEGIN_NATIVE_MEMBERS マクロです。引数には TJS2 内で使用するクラス名
		を指定します。
		このマクロと TJS_END_NATIVE_MEMBERS マクロで挟まれた場所に、クラスの
		メンバとなるべきメソッドやプロパティの記述をします。
	*/
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/PluggedDrawDevice)

		/*
			空の finalize メソッドを宣言します。finalize に相当する処理は
			tTJSNativeInstance::Invalidate をオーバーライドすることでも実装でき
			ますので、通常は空のメソッドで十分です。
		*/
		TJS_DECL_EMPTY_FINALIZE_METHOD

		/*
			(TJSの) コンストラクタを宣言します。TJS でクラスを書くとき、
			クラス内でクラスと同名のメソッドを宣言している部分に相当します。

			TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL マクロの１番目の引数はネイティブ
			インスタンスに割り当てる変数名で、２場面目の引数はその変数の型名です。
			この例でのこのブロック内では NI_PluggedDrawDevice * _this という変数が利用可能で、
			ネイティブインスタンスにアクセスすることができます。
			マクロの３番目の引数は、TJS 内で使用するクラス名を指定します。
			TJS_END_NATIVE_CONSTRUCTOR_DECL マクロの引数も同様です。
			ここも、コンストラクタに相当する処理は tTJSNativeInstance::Construct
			をオーバーライドする事で実装できるので、ここでは何もせずに S_OK を返
			します。
		*/
		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_PluggedDrawDevice,
			/*TJS class name*/PluggedDrawDevice)
		{
			// NI_PluggedDrawDevice::Construct にも内容を記述できるので
			// ここでは何もしない
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/PluggedDrawDevice)

		/*
			print メソッドを宣言します。メソッド名は
			TJS_BEGIN_NATIVE_METHOD_DECL と TJS_END_NATIVE_METHOD_DECL の両マク
			ロに同じものを指定する必要があります。このマクロ内で使用可能な変数に
			tjs_int numparams と tTJSVariant **param があって、それぞれ、渡され
			た引数の数と引数を示しています。このメソッドではそれらは使用していま
			せん。20～21行目は、オブジェクトからネイティブインスタンスを取り出す
			ためのマクロです。この例では _this という NI_PluggedDrawDevice * 型の変数にネイ
			ティブインスタンスを取り出す、という意味になります。以降、_this とい
			う変数でネイティブインスタンスにアクセスできます。23行目で、その
			ネイティブインスタンスの Print メソッドを呼び出しています。
		*/

		TJS_BEGIN_NATIVE_PROP_DECL(interface)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
				*result = reinterpret_cast<tjs_int64>(_this->GetDevice());
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(interface)



	TJS_END_NATIVE_MEMBERS

	/*
		この関数は classobj を返します。
	*/
	return classobj;
}
//---------------------------------------------------------------------------
/*
	TJS_NATIVE_CLASSID_NAME は一応 undef しておいたほうがよいでしょう
*/
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------






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

	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();


	//-----------------------------------------------------------------------
	// 1 まずクラスオブジェクトを作成
	iTJSDispatch2 * tjsclass = Create_NC_PluggedDrawDevice();

	// 2 tjsclass を tTJSVariant 型に変換
	val = tTJSVariant(tjsclass);

	// 3 すでに val が tjsclass を保持しているので、tjsclass は
	//   Release する
	tjsclass->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("PluggedDrawDevice"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------


	// - global を Release する
	global->Release();

	// もし、登録する関数が複数ある場合は 1 ～ 4 を繰り返す


	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


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


	/*
		ただし、クラスの場合、厳密に「オブジェクトが使用中である」ということを
		知るすべがありません。基本的には、Plugins.unlink によるプラグインの解放は
		危険であると考えてください (いったん Plugins.link でリンクしたら、最後ま
		でプラグインを解放せず、プログラム終了と同時に自動的に解放させるのが吉)。
	*/

	// TJS のグローバルオブジェクトに登録した PluggedDrawDevice クラスなどを削除する

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("PluggedDrawDevice"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

