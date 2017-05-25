/**
 * Mesh2D クラス
 */

#ifndef Mesh2DIntfH
#define Mesh2DIntfH

#include "tjsNative.h"

class tTJSNI_Mesh2D : public tTJSNativeInstance
{
public:
	tTJSNI_Mesh2D();
	~tTJSNI_Mesh2D() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;
	void TJS_INTF_METHOD Destruct() override;
};


//---------------------------------------------------------------------------
// tTJSNC_Mesh2D : TJS Mesh2D class
//---------------------------------------------------------------------------
class tTJSNC_Mesh2D : public tTJSNativeClass
{
public:
	tTJSNC_Mesh2D();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Mesh2D(); }
};
#endif
