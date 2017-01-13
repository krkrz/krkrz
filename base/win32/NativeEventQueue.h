
#ifndef __NATIVE_EVENT_QUEUE_H__
#define __NATIVE_EVENT_QUEUE_H__

// 呼び出されるハンドラがシングルスレッドで動作するイベントキュー

class NativeEvent {
public:
	LRESULT Result;
	HWND HWnd;
	UINT Message;
	WPARAM WParam;
	LPARAM LParam;

	NativeEvent(){}
	NativeEvent( int mes ) : Result(0), HWnd(NULL), Message(mes), WParam(0), LParam(0) {}
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
	HWND window_handle_;
	WNDCLASSEX	wc_;

	int CreateUtilWindow();
	static LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

public:
	NativeEventQueueImplement() : window_handle_(NULL) {}

	// デフォルトハンドラ
	void HandlerDefault( NativeEvent& event );

	// Queue の生成
	void Allocate();

	// Queue の削除
	void Deallocate();

	void PostEvent( const NativeEvent& event );

	HWND GetOwner() { return window_handle_; }
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
