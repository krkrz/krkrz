
#include "tjsCommHead.h"

#include "tjsArray.h"
#include "Matrix32Intf.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "tjsArray.h"
#include <string.h>

//---------------------------------------------------------------------------
tTJSNI_Matrix32::tTJSNI_Matrix32() : IsDirty(true), MatrixArray(nullptr) {
	// 単位行列として初期化
	float* m = WorkMatrix;
	 m[0] = 1.0f;  m[1] = 0.0f;  m[2] = 0.0f;  m[3] = 0.0f;
	 m[4] = 0.0f;  m[5] = 1.0f;  m[6] = 0.0f;  m[7] = 0.0f;
	 m[8] = 0.0f;  m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
	m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}
//---------------------------------------------------------------------------
tTJSNI_Matrix32::~tTJSNI_Matrix32() {
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_Matrix32::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	if( numparams == 6 ) {
		float* m = Matrix.a;
		m[0] = static_cast<float>( (tjs_real)*param[0] );
		m[1] = static_cast<float>( (tjs_real)*param[1] );
		m[2] = static_cast<float>( (tjs_real)*param[2] );
		m[3] = static_cast<float>( (tjs_real)*param[3] );
		m[4] = static_cast<float>( (tjs_real)*param[4] );
		m[5] = static_cast<float>( (tjs_real)*param[5] );
	} else {
		SetIdentity();
	}
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Matrix32::Invalidate() {
	if(MatrixArray) MatrixArray->Release(), MatrixArray = nullptr;
}
//---------------------------------------------------------------------------
void tTJSNI_Matrix32::Set( tjs_real m11, tjs_real m12, tjs_real m21, tjs_real m22, tjs_real m31, tjs_real m32 ) {
	Matrix.m[0][0] = static_cast<float>(m11);
	Matrix.m[0][1] = static_cast<float>(m12);
	Matrix.m[1][0] = static_cast<float>(m21);
	Matrix.m[1][1] = static_cast<float>(m22);
	Matrix.m[2][0] = static_cast<float>(m31);
	Matrix.m[2][1] = static_cast<float>(m32);
	IsDirty = true;
}
//---------------------------------------------------------------------------
void tTJSNI_Matrix32::Set( tTJSNI_Matrix32* matrix ) {
	memcpy( Matrix.a, matrix->Matrix.a, sizeof(float)*6 );
	IsDirty = true;
}
//---------------------------------------------------------------------------
void tTJSNI_Matrix32::Set( tjs_real a[6] ) {
	float* m = Matrix.a;
	m[0] = static_cast<float>( a[0] );
	m[1] = static_cast<float>( a[1] );
	m[2] = static_cast<float>( a[2] );
	m[3] = static_cast<float>( a[3] );
	m[4] = static_cast<float>( a[4] );
	m[5] = static_cast<float>( a[5] );
	IsDirty = true;
}
//---------------------------------------------------------------------------
void tTJSNI_Matrix32::Set( float a[6] ) {
	memcpy( Matrix.a, a, sizeof( float ) * 6 );
	IsDirty = true;
}
//---------------------------------------------------------------------------
void tTJSNI_Matrix32::Set( tjs_uint y, tjs_uint x, tjs_real a ) {
	if( y > 2 || x > 1 ) {
		TVPThrowExceptionMessage(TJSRangeError);
	}
	Matrix.m[y][x] = static_cast<float>(a);
	IsDirty = true;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Matrix32::Get( tjs_uint y, tjs_uint x ) {
	if( y > 2 || x > 1 ) {
		TVPThrowExceptionMessage(TJSRangeError);
	}
	return Matrix.m[y][x];
}
//---------------------------------------------------------------------------
iTJSDispatch2* tTJSNI_Matrix32::GetMatrixArrayObjectNoAddRef() {
	if(!MatrixArray) {
		// create an Array object
		iTJSDispatch2 * classobj;
		MatrixArray = TJSCreateArrayObject(&classobj);
		classobj->Release();
	}
	const float* m = Matrix.a;
	for( tjs_int i = 0; i < 6; i++ ) {
		tTJSVariant val((tjs_real)m[i]);
		MatrixArray->PropSetByNum(TJS_MEMBERENSURE, i, &val, MatrixArray);
	}
	return MatrixArray;
}
//---------------------------------------------------------------------------
const float* tTJSNI_Matrix32::GetMatrixArray() const {
	return Matrix.a;
}
//---------------------------------------------------------------------------
const float* tTJSNI_Matrix32::GetMatrixArray16() const {
	if( IsDirty ) {
		const float* t = Matrix.a;
		float* m = const_cast<float*>(WorkMatrix);
		/*
		 m[0] = t[0];  m[1] = t[1];  m[2] = 0.0f;  m[3] = 0.0f;
		 m[4] = t[2];  m[5] = t[3];  m[6] = 0.0f;  m[7] = 0.0f;
		 m[8] = 0.0f;  m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
		m[12] = t[4]; m[13] = t[5]; m[14] = 0.0f; m[15] = 1.0f;
		*/
		// 単位行列としてコンストラクタで初期化済みなので、変更のある部分のみ再設定。
		// 読み取りのみで外部からの変更は想定されていないので注意
		 m[0] = t[0];  m[1] = t[1];
		 m[4] = t[2];  m[5] = t[3];
		m[12] = t[4]; m[13] = t[5];
		const_cast<tTJSNI_Matrix32*>(this)->IsDirty = false;
	}
	return WorkMatrix;
}
//---------------------------------------------------------------------------
// tTJSNC_Matrix32 : TJS Matrix32 class
//---------------------------------------------------------------------------
#define TJS_DEFINE_MATRIX_PROP( MI, MJ ) \
TJS_BEGIN_NATIVE_PROP_DECL( m##MI##MJ ) \
{ \
	TJS_BEGIN_NATIVE_PROP_GETTER \
	{ \
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32); \
		*result = _this->Get( MI-1, MJ-1 ); \
		return TJS_S_OK; \
	} \
	TJS_END_NATIVE_PROP_GETTER \
 \
	TJS_BEGIN_NATIVE_PROP_SETTER \
	{ \
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32); \
		_this->Set( MI-1, MJ-1, (tjs_real)*param ); \
		return TJS_S_OK; \
	} \
	TJS_END_NATIVE_PROP_SETTER \
} \
TJS_END_NATIVE_PROP_DECL( m##MI##MJ )
//----------------------------------------------------------------------
tjs_uint32 tTJSNC_Matrix32::ClassID = -1;
tTJSNC_Matrix32::tTJSNC_Matrix32() : tTJSNativeClass(TJS_W("Matrix32"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Matrix32) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_Matrix32, /*TJS class name*/Matrix32)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Matrix32)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/set)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32);

	if( numparams == 6 ) {
		_this->Set( *param[0], *param[1], *param[2], *param[3], *param[4], *param[5] );
	} else if( numparams == 1 ) {
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			tTJSNI_Matrix32* matrix = (tTJSNI_Matrix32*)TJSGetNativeInstance( tTJSNC_Matrix32::ClassID, param[0] );
			if( matrix != nullptr ) {
				_this->Set( matrix );
				return TJS_S_OK;
			} else {
				tjs_int count = TJSGetArrayElementCount(clo.Object);
				tjs_real a[6];
				if( count >= 6 ) {
					tTJSVariant tmp;
					for( tjs_int i = 0; i < 6; i++ ) {
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
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32);
	_this->SetIdentity();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/reset)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setTranslate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	_this->SetTranslate( (tjs_real)*param[0], (tjs_real)*param[1] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setTranslate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setScale )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	_this->SetScale( (tjs_real)*param[0], (tjs_real)*param[1] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setScale )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setRotate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SetRotate( (tjs_real)*param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setRotate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setSkewX )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SetSkewX( (tjs_real)*param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setSkewX )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setSkewY )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SetSkewY( (tjs_real)*param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setSkewY )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/translate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	_this->Translate( (tjs_real)*param[0], (tjs_real)*param[1] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/translate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/rotate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	if( numparams < 3 ) {
		_this->Rotate( (tjs_real)*param[0] );
	} else {
		_this->Rotate( (tjs_real)*param[0], (tjs_real)*param[1], (tjs_real)*param[2] );
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/rotate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/scale )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	_this->Scale( (tjs_real)*param[0], (tjs_real)*param[1] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/scale )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/preScale )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	_this->PreScale( (tjs_real)*param[0], (tjs_real)*param[1] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/preScale )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/skewX )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SkewX( (tjs_real)*param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/skewX )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/skewY )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SkewY( (tjs_real)*param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/skewY )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/transform )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	tjs_real x = (tjs_real)*param[0];
	tjs_real y = (tjs_real)*param[1];
	_this->TransformPoint( x, y );
	*param[0] = x;
	*param[1] = y;
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/transform )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/multiply )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix32* matrix = (tTJSNI_Matrix32*)TJSGetNativeInstance( tTJSNC_Matrix32::ClassID, param[1] );
	if( !matrix ) return TJS_E_INVALIDPARAM;
	_this->Multiply( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/multiply )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/premultiply )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix32* matrix = (tTJSNI_Matrix32*)TJSGetNativeInstance( tTJSNC_Matrix32::ClassID, param[1] );
	if( !matrix ) return TJS_E_INVALIDPARAM;
	_this->Premultiply( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/premultiply )
//----------------------------------------------------------------------

//----------------------------------------------------------------------

//-- properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(array)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix32);
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
TJS_DEFINE_MATRIX_PROP( 2, 1 )
TJS_DEFINE_MATRIX_PROP( 2, 2 )
TJS_DEFINE_MATRIX_PROP( 3, 1 )
TJS_DEFINE_MATRIX_PROP( 3, 2 )
//----------------------------------------------------------------------

//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
#undef TJS_DEFINE_MATRIX_PROP

//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Matrix32()
{
	tTJSNativeClass *cls = new tTJSNC_Matrix32();
	return cls;
}
//---------------------------------------------------------------------------

