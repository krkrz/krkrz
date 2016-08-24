/**
 * 呼び出されるハンドラがシングルスレッドで動作するイベントキュー
 * メインスレッドで処理したいものがある時に使用する
 **/

#ifndef __NATIVE_EVENT_QUEUE_H__
#define __NATIVE_EVENT_QUEUE_H__


class NativeEvent {
public:
	int Result;
	tjs_uint Message;
	tjs_uint WParam;
	tjs_uint64 LParam;
	NativeEvent( int mes ) : Result(0), Message(mes), WParam(0), LParam(0) {}
	NativeEvent(){}
};

class NativeEventQueueIntarface {
public:
	// デフォルトハンドラ
	virtual void HandlerDefault( class NativeEvent& event ) = 0;

	// Queue の生成
	virtual void Allocate() = 0;

	// Queue の削除
	virtual void Deallocate() = 0;

	virtual void Dispatch( class NativeEvent& event ) = 0;

	virtual void PostEvent( const NativeEvent& event ) = 0;
};

class NativeEventQueueImplement : public NativeEventQueueIntarface {
public:
	NativeEventQueueImplement() {}

	// デフォルトハンドラ
	void HandlerDefault( NativeEvent& event );

	// Queue の生成(もしくは接続)
	void Allocate();

	// Queue の削除(もしくは接続解除)
	void Deallocate();

	// メインスレッドでDispatchを実行させるために非メインスレッド等から呼び出される
	void PostEvent( const NativeEvent& event );
};

// コンストラクタで渡されたクラスの関数ポインタをメインスレッドで実行させるためのクラス
template<typename T>
class NativeEventQueue : public NativeEventQueueImplement {
	void (T::*handler_)(NativeEvent&);
	T* owner_;

public:
	NativeEventQueue( T* owner, void (T::*Handler)(NativeEvent&) ) : owner_(owner), handler_(Handler) {}

	void Dispatch( NativeEvent &ev ) {
		(owner_->*handler_)(ev);
	}
};

#endif // __NATIVE_EVENT_QUEUE_H__
