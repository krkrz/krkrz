/**
 * Matrix44 クラス
 */

#ifndef Matrix44IntfH
#define Matrix44IntfH

#include "tjsNative.h"

class tTJSNI_Matrix44 : public tTJSNativeInstance
{
public:
	tTJSNI_Matrix44();
	~tTJSNI_Matrix44() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	void Set( tjs_real m11, tjs_real m12, tjs_real m13, tjs_real m14,
			tjs_real m21, tjs_real m22, tjs_real m23, tjs_real m24,
			tjs_real m31, tjs_real m32, tjs_real m33, tjs_real m34,
			tjs_real m41, tjs_real m42, tjs_real m43, tjs_real m44 );
	void Set( tTJSNI_Matrix44* matrix );
	void Set( tjs_real a[16] );
	void Set( tjs_int i, tjs_int j, tjs_real a );
	tjs_real Get( tjs_int i, tjs_int j );
	void DoIdentity();
	iTJSDispatch2 * GetMatrixArrayObjectNoAddRef();
};


//---------------------------------------------------------------------------
// tTJSNC_Matrix44 : TJS Matrix44 class
//---------------------------------------------------------------------------
class tTJSNC_Matrix44 : public tTJSNativeClass
{
public:
	tTJSNC_Matrix44();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Matrix44(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Matrix44();
#endif
