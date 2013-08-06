

#ifndef RectIntfH
#define RectIntfH

#include "tjsNative.h"
#include "ComplexRect.h"

class tTJSNI_Rect : public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

protected:
	tTVPRect Rect;

public:
	tTJSNI_Rect();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	tTVPRect& Get() { return Rect; }
	const tTVPRect& Get() const { return Rect; }

	void SetWidth(tjs_int width) {
		Rect.set_width( width );
	}
	tjs_int GetWidth() const {
		return Rect.get_width();
	}
	void SetHeight(tjs_int height) {
		Rect.set_height( height );
	}
	tjs_int GetHeight() const {
		return Rect.get_height();
	}

	void SetOffset( tjs_int x, tjs_int y ) {
		Rect.set_offsets( x, y );
	}
	void SetSize(tjs_int width, tjs_int height) {
		Rect.set_size( width, height );
	}

	bool IsEmpty() const {
		return Rect.is_empty();
	}
	bool Clip( const tTJSNI_Rect& r ) {
		return Rect.clip( r.Rect );
	}
	bool Union( const tTJSNI_Rect& r ) {
		return TVPUnionRect( &Rect, Rect, r.Rect );
	}
	bool Intersects( const tTJSNI_Rect& r ) const {
		return Rect.intersects_with( r.Rect );
	}
	bool Included( const tTJSNI_Rect& r ) const {
		return Rect.included_in( r.Rect );
	}
	bool Equal( const tTJSNI_Rect& r ) {
		return Rect == r.Rect;
	}
};


class tTJSNC_Rect : public tTJSNativeClass
{
	typedef tTJSNativeClass inherited;

public:
	tTJSNC_Rect();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance();
};

extern iTJSDispatch2 * TVPCreateRectObject( tjs_int left, tjs_int top, tjs_int right, tjs_int bottom );
extern tTJSNativeClass * TVPCreateNativeClass_Rect();
#endif // RectIntfH
