
#include "tjsCommHead.h"

#include "Mesh2DIntf.h"


tTJSNI_Mesh2D::tTJSNI_Mesh2D() {
}
tTJSNI_Mesh2D::~tTJSNI_Mesh2D() {
}
tjs_error TJS_INTF_METHOD tTJSNI_Mesh2D::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Mesh2D::Invalidate() {
}
void TJS_INTF_METHOD tTJSNI_Mesh2D::Destruct() {
}

//---------------------------------------------------------------------------
// tTJSNC_Mesh2D : TJS Mesh2D class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Mesh2D::ClassID = -1;
tTJSNC_Mesh2D::tTJSNC_Mesh2D() : tTJSNativeClass(TJS_W("Mesh2D"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Mesh2D) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Mesh2D, /*TJS class name*/Mesh2D)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Mesh2D)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
	
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
