/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */

#ifndef __SQOBJECTINFO_H__
#define __SQOBJECTINFO_H__

#include <stdio.h>
#include <squirrel.h>
#include <sqstdstring.h>
#include <string>

#ifndef SQHEAPDEFINE
#define SQHEAPDEFINE \
	static void* operator new(size_t size) { return sq_malloc(size); }\
	static void* operator new[](size_t size) { return sq_malloc(size); }\
	static void  operator delete(void* p) { sq_free(p, 0); }\
	static void  operator delete[](void* p) { sq_free(p, 0); }
#endif

// ログ出力用
#define SQPRINT(v,msg) {\
	SQPRINTFUNCTION print = sq_getprintfunc(v);\
	if (print) {\
		print(v,msg);\
	}\
}

namespace sqobject {

/// 初期化
HSQUIRRELVM init();
/// 終了
extern void done();
/// 情報保持用グローバルVMの取得
extern HSQUIRRELVM getGlobalVM();

class ObjectInfo;

typedef std::basic_string<SQChar> sqstring;

// 値の push
void pushValue(HSQUIRRELVM v, bool value);
void pushValue(HSQUIRRELVM v, SQInteger value);
void pushValue(HSQUIRRELVM v, SQFloat value);
void pushValue(HSQUIRRELVM v, const SQChar *value);
void pushValue(HSQUIRRELVM v, SQUserPointer value);
void pushValue(HSQUIRRELVM v, const ObjectInfo &obj);
void pushValue(HSQUIRRELVM v, const sqstring &value);
void pushValue(HSQUIRRELVM v, SQFUNCTION func);
void pushValue(HSQUIRRELVM v, HSQOBJECT obj);


// 値の取得
SQRESULT getValue(HSQUIRRELVM v, bool *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQInteger *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQFloat *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, const SQChar **value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQUserPointer *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, sqstring *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, ObjectInfo *value, int idx=-1);


// 値の強制初期化
void clearValue(bool *value);
void clearValue(SQInteger *value);
void clearValue(SQFloat *value);
void clearValue(const SQChar **value);
void clearValue(SQUserPointer *value);
void clearValue(ObjectInfo *value);
void clearValue(sqstring *value);


// 値の取得：基本 getValue のコピペ。文字列は安全でない場合があるので排除する必要あり
SQRESULT getResultValue(HSQUIRRELVM v, bool *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQInteger *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQFloat *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQUserPointer *value);
SQRESULT getResultValue(HSQUIRRELVM v, ObjectInfo *value);
SQRESULT getResultValue(HSQUIRRELVM v, sqstring *value);

// ---------------------------------------------------------
// ObjectInfo
// ---------------------------------------------------------

/**
 * squirrel オブジェクト保持用クラス
 * 弱参照も保持可能
 */
class ObjectInfo {
public:
#ifdef SQOBJHEAP
	SQHEAPDEFINE;
#endif
	// roottable の取得
	static ObjectInfo getRoot();
	// 配列の作成
	static ObjectInfo createArray(SQInteger size=0);
	// 辞書の作成
	static ObjectInfo createTable();

	// 内容消去
	void clear();

	// スタックから取得
	void getStack(HSQUIRRELVM v, SQInteger idx);

    // スタックから弱参照として取得
	void getStackWeak(HSQUIRRELVM v, SQInteger idx);

	// オブジェクトを弱参照として取得
	void getWeak(const ObjectInfo &src);
	
	// コンストラクタ
	ObjectInfo();

	// コンストラクタ
	ObjectInfo(HSQOBJECT obj);
	
	// コンストラクタ
	ObjectInfo(HSQUIRRELVM v, SQInteger idx);

	// コピーコンストラクタ
	ObjectInfo(const ObjectInfo &orig);

	// 代入
	ObjectInfo & operator=(const ObjectInfo &orig);

	// 任意型のコンストラクタ
	template<typename T>
	ObjectInfo(T value) {
		sq_resetobject(&obj);
		setValue(value);
	}

	// 任意型の代入
	template<typename T>
	ObjectInfo & operator=(T value) {
		setValue(value);
		return *this;
	}

	// 任意型へのキャスト
	// 取得できなかった場合はクリア値になる
	template<typename T>
	operator T() const
	{
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		T value;
		if (SQ_FAILED(getValue(gv, &value))) {
			clearValue(&value);
		}
		sq_pop(gv, 1);
		return value;
	}
	
	// 値の設定
	template<typename T>
	void setValue(T value) {
		HSQUIRRELVM gv = getGlobalVM();
		pushValue(gv, value);
		getStack(gv, -1);
		sq_pop(gv, 1);
	}
	
	// デストラクタ
	virtual ~ObjectInfo();

	// 弱参照か？
	bool isWeak() {
		return sq_isweakref(obj);
	}
	
	// null か？
	bool isNull() const {
		return type() == OT_NULL;
	}
	
	// 同じスレッドか？
	bool isSameThread(const HSQUIRRELVM v) const;

	// スレッドを取得
	operator HSQUIRRELVM() const;
	
	// オブジェクトをPUSH
	void push(HSQUIRRELVM v) const;

	/// 複製を登録(失敗したらNULL)
	void pushClone(HSQUIRRELVM v) const;
	
	// ---------------------------------------------------
	// 比較関数群
	// ---------------------------------------------------

	template<typename T>
	bool operator ==(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) == 0;
		sq_pop(v,2);
		return cmp;
	}

	template<typename T>
	bool operator !=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) != 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator <(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) < 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator <=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) <= 0;
		sq_pop(v,2);
		return cmp;
	}

	template<typename T>
	bool operator >(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) > 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator >=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) >= 0;
		sq_pop(v,2);
		return cmp;
	}

	// ---------------------------------------------------
	// delegate 処理用
	// ---------------------------------------------------

	// delegate として機能するかどうか
	bool isDelegate() const;

	// bindenv させるかどうか
	bool isBindDelegate() const;

	// ---------------------------------------------------
	// データ取得
	// ---------------------------------------------------

	const SQChar *getString();
	
	// ---------------------------------------------------
	// 配列・辞書処理用
	// ---------------------------------------------------

	/// 配列として初期化
	void initArray(SQInteger size=0);
	
	/// @return 配列なら true
	bool isArray() const { return type() == OT_ARRAY; }

	/// 配列として初期化
	void initTable();

	/// @return 配列なら true
	bool isTable() const { return type() == OT_TABLE; }
	
	/// 配列に値を追加
	SQRESULT append(HSQUIRRELVM v, SQInteger idx);

	/// 配列に配列を追加
	SQRESULT appendArray(ObjectInfo &array);
	
	/// 配列に値を追加
	template<typename T>
	SQRESULT append(T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayappend(gv, -2);
		sq_pop(gv,1);
		return ret;
	}

	/// 配列に値を挿入
	template<typename T>
	SQRESULT insert(SQInteger index, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayinsert(gv, -2, index);
		sq_pop(gv,1);
		return ret;
	}

	/// 配列から指定されたインデックスの値を削除
	SQRESULT remove(SQInteger index) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		SQRESULT ret = sq_arrayremove(gv, -1, index);
		sq_pop(gv,1);
		return ret;
	}

	/// 配列から指定された値を削除
	template<typename T>
	SQRESULT removeValue(T value, bool all=false) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayremovevalue(gv, -2, all ? SQTrue : SQFalse);
		sq_pop(gv,1);
		return ret;
	}
	
	/// 配列/辞書に値を格納
	template<typename K, typename T>
	SQRESULT set(K key, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		pushValue(gv, value);
		SQRESULT ret = sq_set(gv, -3);
		sq_pop(gv,1);
		return ret;
	}

	/// 辞書に値を新規格納
	template<typename K, typename T>
	SQRESULT create(K key, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		pushValue(gv, value);
		SQRESULT ret = sq_newslot(gv, -3, SQFalse);
		sq_pop(gv,1);
		return ret;
	}

	/// 辞書から値を削除
	template<typename K>
	SQRESULT deleteslot(K key) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		SQRESULT ret = sq_deleteslot(gv, -2, SQFalse);
		sq_pop(gv,1);
		return ret;
	}

	/// @return 配列/辞書の中に key が存在するなら true を返す
	template<typename T>
	bool has(T key) const {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		if (sq_exists(gv, -2)) {
			sq_pop(gv, 1);
			return true;
		} else {
			sq_pop(gv, 1);
			return false;
		}
	}
	
	/// 配列/辞書の値を取得。存在しない場合は null を返す
	template<typename K>
	ObjectInfo get(K key) const {
		HSQUIRRELVM gv = getGlobalVM();
		pushData(gv, key);
		ObjectInfo ret(gv, -1);
		sq_pop(gv,1);
		return ret;
	}

	/**
	 * 配列参照用
	 */
	class ObjectInfoReference {

	public:
		// コンストラクタ
		ObjectInfoReference(HSQOBJECT obj, int key) : obj(obj), intKey(key), type(0) {};
		ObjectInfoReference(HSQOBJECT obj, const SQChar *key) : obj(obj), strKey(key), type(1) {};
		ObjectInfoReference(HSQOBJECT obj, sqstring &key) : obj(obj), strKey(key.c_str()), type(1) {};
		
		// 任意型の代入
		// 設定できなかった場合で辞書の場合は作成してしまう
		template<typename T>
		const T &operator=(const T &value) {
			ObjectInfo o(obj);
			if (o.type() == OT_ARRAY) {
				if (type == 0) {
					// サイズが届いてないので null でうめる
					if (o.len() <= intKey) {
						HSQUIRRELVM gv = getGlobalVM();
						o.push(gv);
						while (o.len() <= intKey) {
							sq_pushnull(gv);
							sq_arrayappend(gv, -2);
						}
						sq_pop(gv,1);
					}
					o.set(intKey, value);
				}
			} else {
				SQRESULT ret;
				if (type == 0) {
					ret = o.set(intKey, value);
				} else {
					ret = o.set(strKey, value);
				}
				if (SQ_FAILED(ret)) {
					if (o.type() == OT_TABLE || o.type() == OT_CLASS) {
						if (type == 0) {
							o.create(intKey, value);
						} else {
							o.create(strKey, value);
						}
					}
				}
			}
			return value;
		}
		
		// 任意型へのキャスト
		// 変換できなかった場合は初期値(0)でのクリアになる
		template<typename T>
		operator T() const {
			ObjectInfo o(obj);
			T value;
			SQRESULT ret;
			if (type == 0) {
				ret = o.get(intKey, &value);
			} else {
				ret = o.get(strKey, &value);
			}
			if (SQ_FAILED(ret)) {
				clearValue(&value);
			}
			return value;
		}

		template<typename K>
		ObjectInfoReference operator[](K key) const {
			HSQOBJECT target;
			HSQUIRRELVM gv = getGlobalVM();
			pushData(gv);
			sq_getstackobj(gv, -1, &target);
			ObjectInfoReference ret = ObjectInfoReference(target, key);
			sq_pop(gv, 1);
			return ret;
		}
		
		// データ取得用
		void pushData(HSQUIRRELVM v) const {
			ObjectInfo o(obj);
			if (type == 0) {
				o.pushData(v, intKey);
			} else {
				o.pushData(v, strKey);
			}
		}
		
	protected:
		HSQOBJECT obj;
		int intKey;
		const SQChar *strKey;
		int type;
	};
	
	// 値の設定
	// 参照情報を代入させる場合用
	void setValue(ObjectInfoReference &ref) {
		HSQUIRRELVM gv = getGlobalVM();
		ref.pushData(gv);
		getStack(gv, -1);
		sq_pop(gv, 1);
	}
	
	/**
	 * 配列/辞書参照用リファレンスを取得
	 * 辞書アクセスした場合は
	 * ・代入失敗したら自動で作成
	 * ・取得失敗したら初期値(0)を返す
	 * 配列アクセスした場合は
	 * ・サイズが小さかったら自動的にそのサイズまで増やす(nullをうめる)
	 * ・取得失敗したら初期値(0)を返す
	 * という処理が自動的に行われます
	 */
	template<typename K>
	ObjectInfoReference operator[](K key) const {
		return ObjectInfoReference(obj, key);
	}

	/// 配列/辞書の値を取得する
	template<typename K, typename T>
	SQRESULT get(K key, T *value) const {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		if (SQ_SUCCEEDED(ret = sq_get(gv, -2))) {
			ret = getValue(gv, value);
			sq_pop(gv,1);
		}
		sq_pop(gv,1);
		return ret;
	}
	
	/// 配列/辞書の値をpush。見つからなければnullをpush
	template<typename T>
	void pushData(HSQUIRRELVM v, T key) const {
		push(v);
		pushValue(v, key);
		if (SQ_FAILED(sq_get(v, -2))) {
			sq_pushnull(v);
		}
		sq_remove(v,-2);
	}
	
	/// 配列・辞書の中身をクリア
	void clearData() {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_clear(gv,-1);
		sq_pop(gv,1);
	}

	/// @return オブジェクトの型
	SQObjectType type() const {
		SQObjectType type;
		if (sq_isweakref(obj)) {
			HSQUIRRELVM gv = getGlobalVM();
			sq_pushobject(gv, obj);
			sq_getweakrefval(gv, -1);
			type = sq_gettype(gv, -1);
			sq_pop(gv, 2);
		} else {
			type = sq_type(obj);
		}
		return type;
	}
	
	/// @return 配列の長さ/辞書のサイズ/文字列の長さ
	SQInteger len() const;

	/**
	 * 配列の内容を全部PUSH
	 * @param v squirrelVM
	 * @return push した数
	 */
	SQInteger pushArray(HSQUIRRELVM v) const;

	// ---------------------------------------------------
	// 関数処理用
	// ---------------------------------------------------

	/// @return 配列なら true
	bool isClosure() const { SQObjectType t = type(); return t == OT_CLOSURE || t == OT_NATIVECLOSURE; }

	// 呼び出し処理
	SQRESULT call(ObjectInfo *self=NULL);

	template<typename T1> SQRESULT call(T1 p1, ObjectInfo *self=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		SQRESULT ret = sq_call(gv, 2, SQFalse, SQTrue);
		sq_pop(gv, 1);
		return ret;
	}

	template<typename T1, typename T2> SQRESULT call(T1 p1, T2 p2, ObjectInfo *self=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		pushValue(gv, p2);
		SQRESULT ret = sq_call(gv, 3, SQFalse, SQTrue);
		sq_pop(gv, 1);
		return ret;
	}
	
	template<typename R> SQRESULT callResult(R* r, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		if (SQ_SUCCEEDED(ret = sq_call(gv, 1, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}
	template<typename R, typename T1> SQRESULT callResult(R* r, T1 p1, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		if (SQ_SUCCEEDED(ret = sq_call(gv, 2, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}
	template<typename R, typename T1, typename T2> SQRESULT callResult(R* r, T1 p1, T2 p2, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		pushValue(gv, p2);
		if (SQ_SUCCEEDED(ret = sq_call(gv, 3, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}

	/**
	 * 自己オブジェクトメソッド呼び出し（引数無し)
	 * @param methodName メソッド名
	 */
	SQRESULT callMethod(const SQChar *methodName) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(this);
			}
		}
		return SQ_ERROR;
	}

	/**
	 * 自己オブジェクトメソッド呼び出し（引数1つ)
	 * @param methodName メソッド名
	 * @param p1 引数
	 */
	template<typename T1> SQRESULT callMethod(const SQChar *methodName, T1 p1) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(p1, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * 自己オブジェクトメソッド呼び出し（引数2つ)
	 * @param methodName メソッド名
	 * @param p1 引数
	 * @param p2 引数2
	 */
	template<typename T1, typename T2> SQRESULT callMethod(const SQChar *methodName, T1 p1, T2 p2) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(p1, p2, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * 返値有り自己オブジェクトメソッド呼び出し（引数無し)
	 * @param r 帰り値ポインタ
	 * @param methodName メソッド名
	 */
	template<typename R> SQRESULT callMethodResult(R* r, const SQChar *methodName) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, this);
			}
		}
		return SQ_ERROR;
	}

	/**
	 * 返値あり自己オブジェクトメソッド呼び出し（引数1つ)
	 * @param r 帰り値ポインタ
	 * @param methodName メソッド名
	 * @param p1 引数
	 */
	template<typename R, typename T1> SQRESULT callMethodResult(R* r, const SQChar *methodName, T1 p1) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, p1, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * 返値有り自己オブジェクトメソッド呼び出し（引数2つ)
	 * @param r 帰り値ポインタ
	 * @param methodName メソッド名
	 * @param p1 引数
	 * @param p2 引数2
	 */
	template<typename R, typename T1, typename T2> SQRESULT callMethodResult(R* r, const SQChar *methodName, T1 p1, T2 p2) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, p1, p2, this);
			}
		}
		return SQ_ERROR;
	}

	// ---------------------------------------------------
	// クラス処理用
	// ---------------------------------------------------

	// クラスオブジェクトか
	bool isClass() const;


	// ---------------------------------------------------
	// イテレータ処理
	// ---------------------------------------------------

	/**
	 * イテレータ呼び出し関数
	 * @param key キー
	 * @param value 値
	 * @param userData ユーザデータ
	 */
	typedef void (*foreachFunc)(const ObjectInfo &key, const ObjectInfo &value, void *userData);

	/**
	 * 関数版イテレータ処理処理
	 * @param func 呼び出し関数
	 * @param userData ユーザデータ
	 */
	void foreach(foreachFunc func, void *userData=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_pushnull(gv);
		while (SQ_SUCCEEDED(sq_next(gv,-2))) {
			func(ObjectInfo(gv,-2), ObjectInfo(gv,-1), userData);
			sq_pop(gv,2);
		}
		sq_pop(gv,1); // null
	}

	/**
	 * 関数オブジェクト版イテレータ処理。以下を実装した関数オブジェクトを渡す
	 * anytype は ObjectInfo からキャスト可能な型ならなんでもOK
	 * struct Func {
	 *  void operator()(anytype key, anytype value) {
	 * }
	 */
	template<class F>
	void foreach(F &func) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_pushnull(gv);
		while (SQ_SUCCEEDED(sq_next(gv,-2))) {
			func(ObjectInfo(gv,-2), ObjectInfo(gv, -1));
			sq_pop(gv,2);
		}
		sq_pop(gv,1); // null
	}

	// ---------------------------------------------------
	// 文字列処理他
	// ---------------------------------------------------
	
	/**
	 * 文字列表記を返す
	 * tostring() 相当の処理。
	 */
	sqstring toString() const;

private:
	HSQOBJECT obj; // オブジェクト参照情報
};

// --------------------------------------------------------------------------------------
// printf処理
// --------------------------------------------------------------------------------------

/**
 * printf相当の処理
 * @param format 書式文字列
 * @return 表示文字数
 */
inline SQInteger printf(const SQChar *format)
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	return sqstd_printf(gv, 0);
}

template<typename T1>
inline SQInteger printf(const SQChar *format, T1 p1) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	return sqstd_printf(gv, 1);
}

template<typename T1, typename T2>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	return sqstd_printf(gv, 2);
}

template<typename T1, typename T2, typename T3>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	return sqstd_printf(gv, 3);
}

template<typename T1, typename T2, typename T3, typename T4>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3, T4 p4) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	pushValue(gv, p4);
	return sqstd_printf(gv, 4);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	pushValue(gv, p4);
	pushValue(gv, p5);
	return sqstd_printf(gv, 5);
}

void pushValue(HSQUIRRELVM v, const ObjectInfo::ObjectInfoReference &obj);

};

#endif
