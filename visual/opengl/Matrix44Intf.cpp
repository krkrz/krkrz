
#include "tjsCommHead.h"

#include "Matrix44Intf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "tjsArray.h"

tTJSNI_Matrix44::tTJSNI_Matrix44() {
}
tTJSNI_Matrix44::~tTJSNI_Matrix44() {
}
tjs_error TJS_INTF_METHOD tTJSNI_Matrix44::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Matrix44::Invalidate() {
}

void tTJSNI_Matrix44::Set( tjs_real m11, tjs_real m12, tjs_real m13, tjs_real m14,
		tjs_real m21, tjs_real m22, tjs_real m23, tjs_real m24,
		tjs_real m31, tjs_real m32, tjs_real m33, tjs_real m34,
		tjs_real m41, tjs_real m42, tjs_real m43, tjs_real m44 ) {}
void tTJSNI_Matrix44::Set( tTJSNI_Matrix44* matrix ) {}
void tTJSNI_Matrix44::Set( tjs_real a[16] ) {}
void tTJSNI_Matrix44::Set( tjs_int i, tjs_int j, tjs_real a ) {}
tjs_real tTJSNI_Matrix44::Get( tjs_int i, tjs_int j ) {
	return 0.0;
}
void tTJSNI_Matrix44::DoIdentity() {
}
iTJSDispatch2* tTJSNI_Matrix44::GetMatrixArrayObjectNoAddRef() {
	return nullptr;
}

//---------------------------------------------------------------------------
// tTJSNC_Matrix44 : TJS Matrix44 class
//---------------------------------------------------------------------------
#define TJS_DEFINE_MATRIX_PROP( MI, MJ ) \
TJS_BEGIN_NATIVE_PROP_DECL( m##MI##MJ ) \
{ \
	TJS_BEGIN_NATIVE_PROP_GETTER \
	{ \
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44); \
		*result = _this->Get( MI-1, MJ-1 ); \
		return TJS_S_OK; \
	} \
	TJS_END_NATIVE_PROP_GETTER \
 \
	TJS_BEGIN_NATIVE_PROP_SETTER \
	{ \
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44); \
		_this->Set( MI-1, MJ-1, (tjs_real)*param ); \
		return TJS_S_OK; \
	} \
	TJS_END_NATIVE_PROP_SETTER \
} \
TJS_END_NATIVE_PROP_DECL( m##MI##MJ )
//----------------------------------------------------------------------
tjs_uint32 tTJSNC_Matrix44::ClassID = -1;
tTJSNC_Matrix44::tTJSNC_Matrix44() : tTJSNativeClass(TJS_W("Matrix44"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Matrix44) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Matrix44, /*TJS class name*/Matrix44)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Matrix44)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/set)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44);

	if( numparams == 16 ) {
		_this->Set( *param[ 0], *param[ 1], *param[ 2], *param[ 3],
					*param[ 4], *param[ 5], *param[ 6], *param[ 7],
					*param[ 8], *param[ 9], *param[ 9], *param[10],
					*param[11], *param[12], *param[13], *param[14] );
	} else if( numparams == 1 ) {
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			tTJSNI_Matrix44* matrix = nullptr;
			if( TJS_SUCCEEDED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE, tTJSNC_Matrix44::ClassID, (iTJSNativeInstance**)&matrix)) ) {
				if( !matrix ) TVPThrowExceptionMessage(TJS_W("Parameter require Matrix44 class instance."));
				_this->Set( matrix );
				return TJS_S_OK;
			} else {
				tjs_int count = TJSGetArrayElementCount(clo.Object);
				tjs_real a[16];
				if( count >= 16 ) {
					tTJSVariant tmp;
					for( tjs_int i = 0; i < 16; i++ ) {
						if(TJS_FAILED(clo.Object->PropGetByNum(TJS_MEMBERMUSTEXIST, i, &tmp, clo.ObjThis)))
							TVPThrowExceptionMessage( TJS_W("Insufficient number of arrays.") );
						a[i] = (tjs_real)tmp;
					}
					_this->Set( a );
					return TJS_S_OK;
				}
			}
		}
		return TJS_E_INVALIDPARAM;
	} else {
		return TJS_E_BADPARAMCOUNT;
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/set)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/reset)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44);
	_this->DoIdentity();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/reset)
//----------------------------------------------------------------------


//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(array)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44);
		iTJSDispatch2 *dsp = _this->GetMatrixArrayObjectNoAddRef();
		*result = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(array)
//----------------------------------------------------------------------
TJS_DEFINE_MATRIX_PROP( 1, 1 )
TJS_DEFINE_MATRIX_PROP( 1, 2 )
TJS_DEFINE_MATRIX_PROP( 1, 3 )
TJS_DEFINE_MATRIX_PROP( 1, 4 )
TJS_DEFINE_MATRIX_PROP( 2, 1 )
TJS_DEFINE_MATRIX_PROP( 2, 2 )
TJS_DEFINE_MATRIX_PROP( 2, 3 )
TJS_DEFINE_MATRIX_PROP( 2, 4 )
TJS_DEFINE_MATRIX_PROP( 3, 1 )
TJS_DEFINE_MATRIX_PROP( 3, 2 )
TJS_DEFINE_MATRIX_PROP( 3, 3 )
TJS_DEFINE_MATRIX_PROP( 3, 4 )
TJS_DEFINE_MATRIX_PROP( 4, 1 )
TJS_DEFINE_MATRIX_PROP( 4, 2 )
TJS_DEFINE_MATRIX_PROP( 4, 3 )
TJS_DEFINE_MATRIX_PROP( 4, 4 )
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
#undef TJS_DEFINE_MATRIX_PROP

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Matrix44()
{
	tTJSNativeClass *cls = new tTJSNC_Matrix44();
	return cls;
}
//---------------------------------------------------------------------------

