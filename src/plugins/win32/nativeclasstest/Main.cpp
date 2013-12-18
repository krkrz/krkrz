//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"

//---------------------------------------------------------------------------



/*
	プラグイン側でネイティブ実装されたクラスを提供し、吉里吉里側で使用できるように
	する例です。

	ネイティブクラスは tTJSNativeInstance を継承したクラス上に作成し、そのネイ
	ティブクラスとのインターフェースを tTJSNativeClassForPlugin をベースに作成し
	ます。

	「TJS2 リファレンス」の「組み込みの手引き」の「基本的な使い方」にある例と同じ
	クラスをここでは作成します。ただし、プラグインで実装する都合上、TJS2 リファ
	レンスにある例とは若干実装の仕方が異なることに注意してください。
*/



//---------------------------------------------------------------------------
// テストクラス
//---------------------------------------------------------------------------
/*
	各オブジェクト (iTJSDispatch2 インターフェース) にはネイティブインスタンスと
	呼ばれる、iTJSNativeInstance 型のオブジェクトを登録することができ、これを
	オブジェクトから取り出すことができます。
	まず、ネイティブインスタンスの実装です。ネイティブインスタンスを実装するには
	tTJSNativeInstance からクラスを導出します。tTJSNativeInstance は
	iTJSNativeInstance の基本的な動作を実装しています。
*/
class NI_Test : public tTJSNativeInstance // ネイティブインスタンス
{
public:
	NI_Test()
	{
		// コンストラクタ
		/*
			NI_Test のコンストラクタです。C++ クラスとしての初期化は 後述の
			Construct よりもここで済ませておき、Construct での初期化は最小限の物
			にすることをおすすめします。この例では、データメンバの Value に初期
			値として 0 を設定しています。
		*/
		Value = 0;
	}

	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
	{
		// TJS2 オブジェクトが作成されるときに呼ばれる
		/*
			TJS2 の new 演算子で TJS2 オブジェクトが作成されるときに呼ばれます。
			numparams と param 引数は new 演算子に渡された引数を表しています。
			tjs_obj 引数は、作成される TJS オブジェクトです。
			この例では、引数があれば (さらにそれが void で無ければ)、それを Value
			の初期値として設定しています。
		*/

		// 引数があればそれを初期値として Value に入れる
		if(numparams >= 1 && param[0]->Type() != tvtVoid)
			Value = (tjs_int)*param[0];

		return S_OK;
	}

	void TJS_INTF_METHOD Invalidate()
	{
		// オブジェクトが無効化されるときに呼ばれる
		/*
			オブジェクトが無効化されるときに呼ばれるメソッドです。ここに終了処理
			を書くと良いでしょう。この例では何もしません。
		*/
	}

	/*
		データメンバを操作するための公開メソッド群です。後述するネイティブクラス
		内で、これらを利用するコードを書きます。
	*/
	void SetValue(tjs_int n) { Value = n; }
	tjs_int GetValue() const { return Value; }

	tjs_int GetSquare() const { return Value*Value; }
	void Add(tjs_int n) { Value += n; }
	void Print() const { TVPAddLog(ttstr(Value)); }

private:
	/*
		データメンバです。ネイティブインスタンスには、必要なデータメンバを自由に
		書くことができます。
	*/
	tjs_int Value; // 値
};
//---------------------------------------------------------------------------
/*
	これは NI_Test のオブジェクトを作成して返すだけの関数です。
	後述の TJSCreateNativeClassForPlugin の引数として渡します。
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Test()
{
	return new NI_Test();
}
//---------------------------------------------------------------------------
/*
	TJS2 のネイティブクラスは一意な ID で区別されている必要があります。
	これは後述の TJS_BEGIN_NATIVE_MEMBERS マクロで自動的に取得されますが、
	その ID を格納する変数名と、その変数をここで宣言します。
	初期値には無効な ID を表す -1 を指定してください。
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_Test
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 用の「クラス」を作成して返す関数です。
*/
static iTJSDispatch2 * Create_NC_Test()
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
		TJSCreateNativeClassForPlugin(TJS_W("Test"), Create_NI_Test);


	/*
		TJS_BEGIN_NATIVE_MEMBERS マクロです。引数には TJS2 内で使用するクラス名
		を指定します。
		このマクロと TJS_END_NATIVE_MEMBERS マクロで挟まれた場所に、クラスの
		メンバとなるべきメソッドやプロパティの記述をします。
	*/
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Test)

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
			この例でのこのブロック内では NI_Test * _this という変数が利用可能で、
			ネイティブインスタンスにアクセスすることができます。
			マクロの３番目の引数は、TJS 内で使用するクラス名を指定します。
			TJS_END_NATIVE_CONSTRUCTOR_DECL マクロの引数も同様です。
			ここも、コンストラクタに相当する処理は tTJSNativeInstance::Construct
			をオーバーライドする事で実装できるので、ここでは何もせずに S_OK を返
			します。
		*/
		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Test,
			/*TJS class name*/Test)
		{
			// NI_Test::Construct にも内容を記述できるので
			// ここでは何もしない
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Test)

		/*
			print メソッドを宣言します。メソッド名は
			TJS_BEGIN_NATIVE_METHOD_DECL と TJS_END_NATIVE_METHOD_DECL の両マク
			ロに同じものを指定する必要があります。このマクロ内で使用可能な変数に
			tjs_int numparams と tTJSVariant **param があって、それぞれ、渡され
			た引数の数と引数を示しています。このメソッドではそれらは使用していま
			せん。20～21行目は、オブジェクトからネイティブインスタンスを取り出す
			ためのマクロです。この例では _this という NI_Test * 型の変数にネイ
			ティブインスタンスを取り出す、という意味になります。以降、_this とい
			う変数でネイティブインスタンスにアクセスできます。23行目で、その
			ネイティブインスタンスの Print メソッドを呼び出しています。
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/print) // print メソッド
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Test);

			_this->Print();

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/print)


		/*
			add メソッドを宣言します。ここでは numparams と param を使用しています。
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/add) // add メソッド
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Test);

			if(numparams < 1) return TJS_E_BADPARAMCOUNT;

			_this->Add((tjs_int)*param[0]);

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/add)

		/*
			value プロパティを宣言します。TJS_BEGIN_NATIVE_PROP_DECL と
			TJS_END_NATIVE_PROP_DECL の両マクロには、メソッドの宣言と同じく、
			プロパティ名を指定します。

			TJS_BEGIN_NATIVE_PROP_GETTER と TJS_END_NATIVE_PROP_GETTER マクロで
			囲まれた場所には、ゲッターを記述することができます。ゲッター内では
			tTJSVariant 型である *result に値を設定するように記述します。
			同様に、TJS_BEGIN_NATIVE_PROP_SETTER と TJS_END_NATIVE_PROP_SETTER
			マクロで囲まれた場所にはセッターを記述することができます。
			セッター内では tTJSVariant 型である *param に設定されるべき値が格納
			されているので、それを使って処理をします。
		*/
		TJS_BEGIN_NATIVE_PROP_DECL(value) // value プロパティ
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
					/*var. type*/NI_Test);
				*result = (tTVInteger)_this->GetValue();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
					/*var. type*/NI_Test);
				_this->SetValue((tjs_int)*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(value)

		/*
			ここでは読み出し専用プロパティを宣言しています。セッターの代わりに
			TJS_DENY_NATIVE_PROP_SETTER を書くことにより、読み出し専用プロパティ
			を作ることができます。
		*/
		TJS_BEGIN_NATIVE_PROP_DECL(square) // square 読み出し専用プロパティ
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
					/*var. type*/NI_Test);

				*result = (tTVInteger)_this->GetSquare();

				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(square)

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
	iTJSDispatch2 * tjsclass = Create_NC_Test();

	// 2 tjsclass を tTJSVariant 型に変換
	val = tTJSVariant(tjsclass);

	// 3 すでに val が tjsclass を保持しているので、tjsclass は
	//   Release する
	tjsclass->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("Test"), // メンバ名 ( かならず TJS_W( ) で囲む )
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

	// TJS のグローバルオブジェクトに登録した Test クラスなどを削除する

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
			TJS_W("Test"), // メンバ名
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

