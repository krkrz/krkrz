//---------------------------------------------------------------------------
// "Screen" TJS Class implementation
//---------------------------------------------------------------------------
#ifndef ScreenIntfH
#define ScreenIntfH

#include "tjsNative.h"
#include "drawable.h"
#include "ComplexRect.h"
#include "tvpinputdefs.h"
#include "EventIntf.h"
#include "ObjectList.h"
#include "LayerTreeOwner.h"



//---------------------------------------------------------------------------
// Screen Management
//---------------------------------------------------------------------------
//extern void TVPClearAllScreenInputEvents();
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// tTJSNI_Screen
//---------------------------------------------------------------------------
class tTJSNI_Screen : public tTJSNativeInstance, public iTVPLayerTreeOwner
{
	typedef tTJSNativeInstance inherited;

private:
	std::vector<tTJSVariantClosure> ObjectVector;
	bool ObjectVectorLocked;

protected:
	iTJSDispatch2 *Owner;
	tTVPScreen* screen_;

public:
	iTJSDispatch2 * TJS_INTF_METHOD GetOwnerNoAddRef() const { return Owner; }

public:
	tTJSNI_Screen();
	~tTJSNI_Screen();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

	void FireOnActivate(bool activate_or_deactivate);

	// call from application
	void SetScreen( tTVPScreen* screen ) { screen_ = screen; }

	void ClearInputEvents();

public:
	void OnActivate();
	void OnDeactivate();

	void OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	void OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);

	void OnKeyDown(tjs_uint key, tjs_uint32 shift);
	void OnKeyUp(tjs_uint key, tjs_uint32 shift);
	void OnKeyPress(tjs_char key); // IME 形式が違うのでどうするか

	void OnTouchDown( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void OnTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void OnTouchMove( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	
	void OnTouchScaling( tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag );
	void OnTouchRotate( tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag );
	void OnMultiTouch();

	void OnDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution );

	void Add(tTJSVariantClosure clo);
	void Remove(tTJSVariantClosure clo);
};
//---------------------------------------------------------------------------
// tTJSNC_Screen : TJS Screen class
//---------------------------------------------------------------------------
class tTJSNC_Screen : public tTJSNativeClass
{
public:
	tTJSNC_Screen();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Screen();
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Input Events
//---------------------------------------------------------------------------
class tTVPOnMouseDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tTVPMouseButton Buttons;
	tjs_uint32 Flags;
public:
	tTVPOnMouseDownInputEvent(tTJSNI_Screen *screen, tjs_int x, tjs_int y,
		tTVPMouseButton buttons, tjs_uint32 flags) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), Buttons(buttons), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnMouseDown(X, Y, Buttons, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tTVPMouseButton Buttons;
	tjs_uint32 Flags;
public:
	tTVPOnMouseUpInputEvent(tTJSNI_Screen *screen, tjs_int x, tjs_int y,
		tTVPMouseButton buttons, tjs_uint32 flags) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), Buttons(buttons), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnMouseUp(X, Y, Buttons, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseMoveInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tjs_uint32 Flags;
public:
	tTVPOnMouseMoveInputEvent(tTJSNI_Screen *screen, tjs_int x, tjs_int y,
		tjs_uint32 flags) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnMouseMove(X, Y, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint Key;
	tjs_uint32 Shift;
public:
	tTVPOnKeyDownInputEvent(tTJSNI_Screen *screen, tjs_uint key, tjs_uint32 shift) :
		tTVPBaseInputEvent(screen, Tag), Key(key), Shift(shift) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnKeyDown(Key, Shift); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint Key;
	tjs_uint32 Shift;
public:
	tTVPOnKeyUpInputEvent(tTJSNI_Screen *screen, tjs_uint key, tjs_uint32 shift) :
		tTVPBaseInputEvent(screen, Tag), Key(key), Shift(shift) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnKeyUp(Key, Shift); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyPressInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_char Key;
public:
	tTVPOnKeyPressInputEvent(tTJSNI_Screen *screen, tjs_char key) :
		tTVPBaseInputEvent(screen, Tag), Key(key) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnKeyPress(Key); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseWheelInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint32 Shift;
	tjs_int WheelDelta;
	tjs_int X;
	tjs_int Y;
public:
	tTVPOnMouseWheelInputEvent(tTJSNI_Screen *screen, tjs_uint32 shift,
		tjs_int wheeldelta, tjs_int x, tjs_int y) :
		tTVPBaseInputEvent(screen, Tag), Shift(shift), WheelDelta(wheeldelta),
		X(x), Y(y) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnMouseWheel(Shift, WheelDelta, X, Y); }
};
//---------------------------------------------------------------------------
class tTVPOnActivateEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	bool ActivateOrDeactivate;
public:
	tTVPOnActivateEvent(tTJSNI_Screen *screen, bool activate_or_deactivate) :
		tTVPBaseInputEvent(screen, Tag), ActivateOrDeactivate(activate_or_deactivate) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnActivate(ActivateOrDeactivate); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchDownInputEvent(tTJSNI_Screen *screen, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnTouchDown(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchUpInputEvent(tTJSNI_Screen *screen, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnTouchUp(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchMoveInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchMoveInputEvent(tTJSNI_Screen *screen, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(screen, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnTouchMove(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchScalingInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real StartDistance;
	tjs_real CurrentDistance;
	tjs_real CX;
	tjs_real CY;
	tjs_int Flag;
public:
	tTVPOnTouchScalingInputEvent(tTJSNI_Screen *screen, tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag ) :
		tTVPBaseInputEvent(screen, Tag), StartDistance(startdist), CurrentDistance(curdist), CX(cx), CY(cy), Flag(flag) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnTouchScaling( StartDistance, CurrentDistance, CX, CY, Flag ); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchRotateInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real StartAngle;
	tjs_real CurrentAngle;
	tjs_real Distance;
	tjs_real CX;
	tjs_real CY;
	tjs_int Flag;
public:
	tTVPOnTouchRotateInputEvent(tTJSNI_Screen *screen, tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag ) :
		tTVPBaseInputEvent(screen, Tag), StartAngle(startangle), CurrentAngle(curangle), Distance(dist), CX(cx), CY(cy), Flag(flag) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnTouchRotate( StartAngle, CurrentAngle, Distance, CX, CY, Flag ); }
};
//---------------------------------------------------------------------------
class tTVPOnMultiTouchInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;

public:
	tTVPOnMultiTouchInputEvent(tTJSNI_Screen *screen ) :
		tTVPBaseInputEvent(screen, Tag) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnMultiTouch(); }
};
//---------------------------------------------------------------------------
class tTVPOnDisplayRotateInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int Orientation;
	tjs_int Rotate;
	tjs_int BPP;
	tjs_int HorizontalResolution;
	tjs_int VerticalResolution;
public:
	tTVPOnDisplayRotateInputEvent(tTJSNI_Screen *screen, tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution ) :
		tTVPBaseInputEvent(screen, Tag), Orientation(orientation), Rotate(rotate), BPP(bpp),
		HorizontalResolution(hresolution), VerticalResolution(vresolution) {};
	void Deliver() const
	{ ((tTJSNI_Screen*)GetSource())->OnDisplayRotate( Orientation, Rotate, BPP, HorizontalResolution, VerticalResolution ); }
};
//---------------------------------------------------------------------------
#endif
