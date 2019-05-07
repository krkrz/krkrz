
#include "tjsCommHead.h"

#include "Matrix44Intf.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <string.h>
#include "GeoUtil.h"

#include "tjsArray.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "RectItf.h"


tTJSNI_Matrix44::tTJSNI_Matrix44() : MatrixArray(nullptr), Mat4(1.0f) {
}
tTJSNI_Matrix44::~tTJSNI_Matrix44() {
}
tjs_error TJS_INTF_METHOD tTJSNI_Matrix44::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_Matrix44::Invalidate() {
	if(MatrixArray) MatrixArray->Release(), MatrixArray = nullptr;
}

void tTJSNI_Matrix44::Set( tjs_real m11, tjs_real m12, tjs_real m13, tjs_real m14,
		tjs_real m21, tjs_real m22, tjs_real m23, tjs_real m24,
		tjs_real m31, tjs_real m32, tjs_real m33, tjs_real m34,
		tjs_real m41, tjs_real m42, tjs_real m43, tjs_real m44 ) {
	Mat4[0][0] = static_cast<float>(m11);
	Mat4[0][1] = static_cast<float>(m12);
	Mat4[0][2] = static_cast<float>(m13);
	Mat4[0][3] = static_cast<float>(m14);
	Mat4[1][0] = static_cast<float>(m21);
	Mat4[1][1] = static_cast<float>(m22);
	Mat4[1][2] = static_cast<float>(m23);
	Mat4[1][3] = static_cast<float>(m24);
	Mat4[2][0] = static_cast<float>(m31);
	Mat4[2][1] = static_cast<float>(m32);
	Mat4[2][2] = static_cast<float>(m33);
	Mat4[2][3] = static_cast<float>(m34);
	Mat4[3][0] = static_cast<float>(m41);
	Mat4[3][1] = static_cast<float>(m42);
	Mat4[3][2] = static_cast<float>(m43);
	Mat4[3][3] = static_cast<float>(m44);
}
void tTJSNI_Matrix44::Set( tTJSNI_Matrix44* matrix ) {
	Mat4 = matrix->Mat4;
}
void tTJSNI_Matrix44::Set( tjs_real a[16] ) {
	float* m = (float *)&Mat4[0];
	for( tjs_int i = 0; i < 16; i++ ) {
		m[i] = static_cast<float>(a[i]);
	}
}
void tTJSNI_Matrix44::Set( tjs_uint y, tjs_uint x, tjs_real a ) {
	if( y >= 4 || x >= 4 ) {
		TVPThrowExceptionMessage(TJSRangeError);
	}
	Mat4[y][x] = static_cast<float>(a);
}
tjs_real tTJSNI_Matrix44::Get( tjs_uint y, tjs_uint x ) {
	if( y >= 4 || x >= 4 ) {
		TVPThrowExceptionMessage(TJSRangeError);
	}
	return Mat4[y][x];
}
iTJSDispatch2* tTJSNI_Matrix44::GetMatrixArrayObjectNoAddRef() {
	if(!MatrixArray) {
		// create an Array object
		iTJSDispatch2 * classobj;
		MatrixArray = TJSCreateArrayObject(&classobj);
		classobj->Release();
	}

	const float* m = (const float *)&(Mat4[0][0]);
	for( tjs_int i = 0; i < 16; i++ ) {
		tTJSVariant val((tjs_real)m[i]);
		MatrixArray->PropSetByNum(TJS_MEMBERENSURE, i, &val, MatrixArray);
	}

	return MatrixArray;
}

void tTJSNI_Matrix44::SetRotateZ( tjs_real deg ) {
	float radian = TVPDegToRad( (float)deg );
	float c = std::cos( radian );
	float s = std::sin( radian );
	Mat4[0][0] = c;
	Mat4[1][0] = s;
	Mat4[0][1] = -s;
	Mat4[1][1] = c;
}
void tTJSNI_Matrix44::SetRotate( tjs_real degree, tjs_real x, tjs_real y, tjs_real z ) {
	Mat4 = glm::rotate( TVPDegToRad( (float)degree ), glm::vec3( (float)x, (float)y, (float)z ) );
}
void tTJSNI_Matrix44::Add( const tTJSNI_Matrix44* rhs ) { Mat4 += rhs->Mat4; }
void tTJSNI_Matrix44::Sub( const tTJSNI_Matrix44* rhs ) { Mat4 -= rhs->Mat4; }
void tTJSNI_Matrix44::Mul( const tTJSNI_Matrix44* rhs ) { Mat4 *= rhs->Mat4; }
void tTJSNI_Matrix44::Div( const tTJSNI_Matrix44* rhs ) { Mat4 /= rhs->Mat4; }
void tTJSNI_Matrix44::Inc() { Mat4++; }
void tTJSNI_Matrix44::Dec() { Mat4--; }

void tTJSNI_Matrix44::MulVector( tjs_real& x, tjs_real& y, tjs_real& z, tjs_real& w ) {
	glm::vec4 v( (float)x, (float)y, (float)z, (float)w );
	glm::vec4 ret = Mat4 * v;
	x = ret.x; y = ret.y; z = ret.z; w = ret.w;
}
void tTJSNI_Matrix44::Translate( tjs_real x, tjs_real y, tjs_real z ) {
	Mat4 = glm::translate( Mat4, glm::vec3( (float)x, (float)y, (float)z ) );
}
void tTJSNI_Matrix44::Rotate( tjs_real degree, tjs_real x, tjs_real y, tjs_real z ) {
	Mat4 = glm::rotate( Mat4, TVPDegToRad( (float)degree), glm::vec3( (float)x, (float)y, (float)z ) );
}
void tTJSNI_Matrix44::RotateX( tjs_real degree, tjs_real px, tjs_real py, tjs_real pz ) {
	glm::mat4 t;
	t[3][0] = (float)px; t[3][1] = (float)py; t[3][2] = (float)pz;
	float radian = TVPDegToRad( (float)degree );
	float c = std::cos( radian );
	float s = std::sin( radian );
	glm::mat4 r;
	r[1][1] = c; r[1][2] = s;
	r[2][1] = -s; r[2][2] = c;
	Multiply( r, t );
	t[3][0] = (float)-px; t[3][1] = (float)-py; t[3][2] = (float)-pz;
	Multiply( t, r );
	Premultiply( Mat4, t );
}
void tTJSNI_Matrix44::Scale( tjs_real x, tjs_real y, tjs_real z ) {
	Mat4 = glm::scale( Mat4, glm::vec3( (float)x, (float)y, (float)z ) );
}
void tTJSNI_Matrix44::Ortho( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top, tjs_real znear, tjs_real zfar ) {
	Mat4 = glm::ortho( (float)left, (float)right, (float)bottom, (float)top, (float)znear, (float)zfar );
}
void tTJSNI_Matrix44::Ortho( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top ) {
	Mat4 = glm::ortho( (float)left, (float)right, (float)bottom, (float)top );
}
void tTJSNI_Matrix44::Frustum( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top, tjs_real znear, tjs_real zfar ) {
	Mat4 = glm::frustum( (float)left, (float)right, (float)bottom, (float)top, (float)znear, (float)zfar );
}
void tTJSNI_Matrix44::Perspective( tjs_real fovy, tjs_real aspect, tjs_real znear, tjs_real zfar ) {
	Mat4 = glm::perspective( (float)fovy, (float)aspect, (float)znear, (float)zfar );
}
void tTJSNI_Matrix44::PerspectiveFov( tjs_real fovy, tjs_real width, tjs_real height, tjs_real znear, tjs_real zfar ) {
	Mat4 = glm::perspectiveFov( (float)fovy, (float)width, (float)height, (float)znear, (float)zfar );
}
void tTJSNI_Matrix44::LookAt( tjs_real eyeX, tjs_real eyeY, tjs_real eyeZ, tjs_real centerX, tjs_real centerY, tjs_real centerZ, tjs_real upX, tjs_real upY, tjs_real upZ ) {
	Mat4 = glm::lookAt( glm::vec3( eyeX, eyeY, eyeZ ), glm::vec3( centerX, centerY, centerZ ), glm::vec3( upX, upY, upZ ) );
}
void tTJSNI_Matrix44::Project( const tTJSNI_Matrix44* model, const tTJSNI_Matrix44* proj, const class tTJSNI_Rect* viewport, tjs_real& x, tjs_real& y, tjs_real& z ) {
	glm::vec3 win( (float)x, (float)y, (float)z );
	const tTVPRect& v = viewport->Get();
	glm::vec4 viewp( (float)v.left, (float)v.top, (float)v.get_width(), (float)v.get_height() );
	glm::vec3 ret = glm::project( win, model->Mat4, proj->Mat4, viewp );
	x = ret.x;
	y = ret.y;
	z = ret.z;
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
					*param[ 8], *param[ 9], *param[10], *param[11],
					*param[12], *param[13], *param[14], *param[15] );
	} else if( numparams == 1 ) {
		tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
		if( clo.Object ) {
			tTJSNI_Matrix44* matrix = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
			if( matrix != nullptr ) {
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
	_this->SetIdentity();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/reset)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setTranslate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	if( numparams == 2 ) _this->SetTranslate( *param[0], *param[1] );
	else _this->SetTranslate( *param[0], *param[1], *param[2] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setTranslate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setScale )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	if( numparams == 2 ) _this->SetScale( *param[0], *param[1] );
	else _this->SetScale( *param[0], *param[1], *param[2] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setScale )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setRotate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	_this->SetRotate( *param[0], *param[1], *param[2], *param[3] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setRotate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setRotateZ )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	_this->SetRotateZ( *param[0] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/setRotateZ )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/add )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix44* matrix = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
	if( !matrix ) return TJS_E_INVALIDPARAM;

	_this->Add( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/add )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/sub )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix44* matrix = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
	if( !matrix ) return TJS_E_INVALIDPARAM;

	_this->Sub( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/sub )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/mul )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix44* matrix = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
	if( !matrix ) return TJS_E_INVALIDPARAM;

	_this->Mul( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/mul )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/div )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
	tTJSNI_Matrix44* matrix = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
	if( !matrix ) return TJS_E_INVALIDPARAM;

	_this->Div( matrix );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/div )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/translate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	if( numparams == 2 ) _this->Translate( (tjs_real)*param[0], (tjs_real)*param[1] );
	else _this->Translate( (tjs_real)*param[0], (tjs_real)*param[1], (tjs_real)*param[2] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/translate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/rotate )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	_this->Rotate( *param[0], *param[1], *param[2], *param[3] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/rotate )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/rotateX ) {
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	_this->RotateX( *param[0], *param[1], *param[2], *param[3] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/rotateX )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/mulVector ) {
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	tjs_real x = *param[0];
	tjs_real y = *param[1];
	tjs_real z = *param[2];
	tjs_real w = *param[3];
	_this->MulVector( x, y, z, w );
	*param[0] = x;
	*param[1] = y;
	*param[2] = z;
	*param[3] = w;
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/mulVector )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/scale )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 2 ) return TJS_E_BADPARAMCOUNT;
	if( numparams == 2 ) _this->Scale( *param[0], *param[1] );
	else _this->Scale( *param[0], *param[1], *param[2] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/scale )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ortho )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	if( numparams < 6 ) {
		_this->Ortho( *param[0], *param[1], *param[2], *param[3] );
	} else {
		_this->Ortho( *param[0], *param[1], *param[2], *param[3], *param[4], *param[5] );
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/ortho )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/frustum )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 6 ) return TJS_E_BADPARAMCOUNT;
	_this->Frustum( *param[0], *param[1], *param[2], *param[3], *param[4], *param[5] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/frustum )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/perspective )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 4 ) return TJS_E_BADPARAMCOUNT;
	_this->Perspective( *param[0], *param[1], *param[2], *param[3] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/perspective )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/perspectiveFov )
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 5 ) return TJS_E_BADPARAMCOUNT;
	_this->PerspectiveFov( *param[0], *param[1], *param[2], *param[3], *param[4] );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/perspectiveFov )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/project )
{
	if( numparams < 6 ) return TJS_E_BADPARAMCOUNT;
	tjs_real x = *param[3];
	tjs_real y = *param[4];
	tjs_real z = *param[5];
	tTJSNI_Matrix44* model = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[0] );
	if( !model ) return TJS_E_INVALIDPARAM;
	tTJSNI_Matrix44* proj = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param[1] );
	if( !proj ) return TJS_E_INVALIDPARAM;
	tTJSNI_Rect* viewport = (tTJSNI_Rect*)TJSGetNativeInstance( tTJSNC_Rect::ClassID, param[2] );
	if( !viewport ) return TJS_E_INVALIDPARAM;
	tTJSNI_Matrix44::Project( model, proj, viewport, x, y, z );
	*param[3] = x;
	*param[4] = y;
	*param[5] = z;
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/project )
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/lookAt ) {
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Matrix44 );
	if( numparams < 9 ) return TJS_E_BADPARAMCOUNT;
	tjs_real ex = *param[0];
	tjs_real ey = *param[1];
	tjs_real ez = *param[2];
	tjs_real cx = *param[3];
	tjs_real cy = *param[4];
	tjs_real cz = *param[5];
	tjs_real ux = *param[6];
	tjs_real uy = *param[7];
	tjs_real uz = *param[8];
	_this->LookAt( ex, ey, ez, cx, cy, cz, ux, uy, uz );
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/lookAt )
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

#if 0
// 演算子のオーバーロードしたかったが期待したように動作しないので、諦める。
//---------------------------------------------------------------------------
class tTSJNC_Matrix44Object: public tTJSCustomObject {
public:
	tjs_error TJS_INTF_METHOD Operation( tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint, tTJSVariant *result, const tTJSVariant *param, iTJSDispatch2 *objthis ) override {
		tjs_uint32 op = flag & TJS_OP_MASK;
		tTJSNI_Matrix44* ni;
		tjs_error hr = objthis->NativeInstanceSupport( TJS_NIS_GETINSTANCE, tTJSNC_Matrix44::ClassID, (iTJSNativeInstance**)&ni );
		if( TJS_SUCCEEDED( hr ) ) {
			switch( op ) {
			case TJS_OP_INC:
				ni->Inc();
				return TJS_S_OK;
			case TJS_OP_DEC:
				ni->Dec();
				return TJS_S_OK;
			}
			tTJSNI_Matrix44* rhs = (tTJSNI_Matrix44*)TJSGetNativeInstance( tTJSNC_Matrix44::ClassID, param );
			if( rhs ) {
				switch( op ) {
				case TJS_OP_SUB:
					ni->Sub( rhs );
					return TJS_S_OK;
				case TJS_OP_ADD:
					ni->Add( rhs );
					return TJS_S_OK;
				case TJS_OP_DIV:
					ni->Div( rhs );
					return TJS_S_OK;
				case TJS_OP_MUL:
					ni->Mul( rhs );
					return TJS_S_OK;
				}
			}
		}
		return tTJSCustomObject::Operation( flag, membername, hint, result, param, objthis );
		/*
		switch(op) {
		case TJS_OP_BAND:// operator &= (*param);
		case TJS_OP_BOR: // operator |= (*param);
		case TJS_OP_BXOR: //operator ^= (*param);
		case TJS_OP_SUB: // operator -= (*param);
		case TJS_OP_ADD: // operator += (*param);
		case TJS_OP_MOD: // operator %= (*param);
		case TJS_OP_DIV: // operator /= (*param);
		case TJS_OP_IDIV: // idivequal(*param);
		case TJS_OP_MUL: // operator *= (*param);
		case TJS_OP_LOR: // logicalorequal(*param);
		case TJS_OP_LAND: // logicalandequal(*param);
		case TJS_OP_SAR: // operator >>= (*param);
		case TJS_OP_SAL: // operator <<= (*param);
		case TJS_OP_SR: // rbitshiftequal(*param);
		case TJS_OP_INC:
		case TJS_OP_DEC:
		default: // return tTJSCustomObject::Operation( flga, membername, hint, result, param, objthis );
		}
		*/
	}
	tjs_error TJS_INTF_METHOD OperationByNum( tjs_uint32 flag, tjs_int num, tTJSVariant *result, const tTJSVariant *param, iTJSDispatch2 *objthis ) override {
		return tTJSCustomObject::OperationByNum( flag, num, result, param, objthis );
	}
};
//---------------------------------------------------------------------------
iTJSDispatch2 *tTJSNC_Matrix44::CreateBaseTJSObject() {
	return new tTSJNC_Matrix44Object();
}
#endif
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Matrix44()
{
	tTJSNativeClass *cls = new tTJSNC_Matrix44();
	return cls;
}
//---------------------------------------------------------------------------

