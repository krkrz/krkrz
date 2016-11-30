
#ifndef __NATIVE_EVENT_QUEUE_H__
#define __NATIVE_EVENT_QUEUE_H__

// 呼び出されるハンドラがシングルスレッドで動作するイベントキュー

struct NativeEvent {
	tjs_uint Message;
	tjs_uint64 Result;
	union {
		tjs_uint64 WParam;
		struct {
			tjs_uint32 WParam0;
			tjs_uint32 WParam1;
		};
		struct {
			float WParamf0;
			float WParamf1;
		};
	};
	union {
		tjs_uint64 LParam;
		struct {
			tjs_uint32 LParam0;
			tjs_uint32 LParam1;
		};
		struct {
			float LParamf0;
			float LParamf1;
		};
	};

	NativeEvent(){}
	NativeEvent( int mes ) : Message(mes) {}
	NativeEvent( int mes, tjs_int64 wparam, tjs_int64 lparam ) : Message(mes) { WParam = wparam; LParam = lparam; }
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
	NativeEventQueueImplement() /*: window_handle_(NULL)*/ {}

	// デフォルトハンドラ
	void HandlerDefault( NativeEvent& event ) {}

	// Queue の生成
	void Allocate();

	// Queue の削除
	void Deallocate();

	void PostEvent( const NativeEvent& event );
};


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
