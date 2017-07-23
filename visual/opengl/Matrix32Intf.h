/**
 * Matrix32 クラス
 */

#ifndef Matrix32IntfH
#define Matrix32IntfH

#include "tjsNative.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include "GeoUtil.h"

class tTJSNI_Matrix32 : public tTJSNativeInstance
{
	/*
	 0  1
	 2  3
	 4  5
	*/
	union {
		float a[6];
		float m[3][2];
	} Matrix;

	bool IsDirty;
	float WorkMatrix[16];


	iTJSDispatch2 *MatrixArray; // Array object which holds matrix array ...
	bool MatrixArrayValid;

private:
	static inline void SetIdentity( float* t ) {
		t[0] = 1.0f; t[1] = 0.0f;
		t[2] = 0.0f; t[3] = 1.0f;
		t[4] = 0.0f; t[5] = 0.0f;
	}
	static inline void SetTranslate( float *t, float tx, float ty ) {
		t[0] = 1.0f; t[1] = 0.0f;
		t[2] = 0.0f; t[3] = 1.0f;
		t[4] = tx;   t[5] = ty;
	}
	static inline void SetScale( float* t, float sx, float sy ) {
		t[0] = sx;   t[1] = 0.0f;
		t[2] = 0.0f; t[3] = sy;
		t[4] = 0.0f; t[5] = 0.0f;
	}
	static inline void SetRotate( float* t, float deg ) {
		float radian = TVPDegToRad( deg );
		float c = std::cos( radian );
		float s = std::sin( radian );
		t[0] = c;    t[1] = s;
		t[2] = -s;   t[3] = c;
		t[4] = 0.0f; t[5] = 0.0f;
	}
	static inline void SetSkewX( float* t, float a ) {
		t[0] = 1.0f; t[1] = 0.0f;
		t[2] = std::tan(a); t[3] = 1.0f;
		t[4] = 0.0f; t[5] = 0.0f;
	}
	static inline void SetSkewY( float* t, float a ) {
		t[0] = 1.0f; t[1] = std::tan(a);
		t[2] = 0.0f; t[3] = 1.0f;
		t[4] = 0.0f; t[5] = 0.0f;
	}
	static void Multiply( float* t, const float* s ) {
		float t0 = t[0] * s[0] + t[1] * s[2];
		float t2 = t[2] * s[0] + t[3] * s[2];
		float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
		t[1] = t[0] * s[1] + t[1] * s[3];
		t[3] = t[2] * s[1] + t[3] * s[3];
		t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
		t[0] = t0;
		t[2] = t2;
		t[4] = t4;
	}
	static void Premultiply(float* t, const float* s) {
		float s2[6];
		memcpy(s2, s, sizeof(float)*6);
		Multiply(s2, t);
		memcpy(t, s2, sizeof(float)*6);
	}
	static void TransformPoint(float& dx, float& dy, const float* t, float sx, float sy) {
		dx = sx*t[0] + sy*t[2] + t[4];
		dy = sx*t[1] + sy*t[3] + t[5];
	}

public:
	tTJSNI_Matrix32();
	~tTJSNI_Matrix32() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	void Set( tjs_real m11, tjs_real m12, tjs_real m21, tjs_real m22, tjs_real m31, tjs_real m32 );
	void Set( tTJSNI_Matrix32* matrix );
	void Set( tjs_real a[6] );
	void Set( float a[6] );
	void Set( tjs_uint i, tjs_uint j, tjs_real a );
	tjs_real Get( tjs_uint i, tjs_uint j );
	void SetIdentity() {
		SetIdentity( Matrix.a );
		IsDirty = true;
	}
	iTJSDispatch2 * GetMatrixArrayObjectNoAddRef();

	void SetTranslate( tjs_real tx, tjs_real ty ) {
		SetTranslate( Matrix.a, (float)tx, (float)ty );
		IsDirty = true;
	}
	void SetScale( tjs_real sx, tjs_real sy ) {
		SetScale( Matrix.a, (float)sx, (float)sy );
		IsDirty = true;
	}
	void SetRotate( tjs_real deg ) {
		SetRotate( Matrix.a, (float)deg );
		IsDirty = true;
	}
	void SetSkewX( tjs_real deg ) {
		SetSkewX( Matrix.a, TVPDegToRad( (float)deg ) );
		IsDirty = true;
	}
	void SetSkewY( tjs_real deg ) {
		SetSkewY( Matrix.a, TVPDegToRad( (float)deg ) );
		IsDirty = true;
	}
	void Translate( tjs_real tx, tjs_real ty ) {
		float t[6];
		SetTranslate( t, (float)tx, (float)ty );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void Rotate( tjs_real deg ) {
		float t[6];
		SetRotate( t, (float)deg );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void Rotate( tjs_real deg, tjs_real px, tjs_real py ) {
		float t[6];
		SetTranslate( t, (float)px, (float)py );
		float r[6];
		SetRotate( r, (float)deg );
		Multiply( r, t );
		SetTranslate( t, -(float)px, -(float)py );
		Multiply( t, r );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void SkewX( tjs_real deg ) {
		float t[6];
		SetSkewX( t, TVPDegToRad((float)deg) );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void SkewY( tjs_real deg ) {
		float t[6];
		SetSkewX( t, TVPDegToRad( (float)deg ) );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void Scale( tjs_real sx, tjs_real sy ) {
		float t[6];
		SetScale( t, (float)sx, (float)sy );
		Premultiply( Matrix.a, t );
		IsDirty = true;
	}
	void PreScale( tjs_real sx, tjs_real sy ) {
		float t[6];
		SetScale( t, (float)sx, (float)sy );
		Multiply( Matrix.a, t );
		IsDirty = true;
	}
	void TransformPoint( tjs_real& x, tjs_real& y ) const {
		float sx = (float)x;
		float sy = (float)y;
		float dx = (float)x;
		float dy = (float)y;
		TransformPoint( dx, dy, Matrix.a, sx, sy );
		x = dx;
		y = dy;
	}
	void TransformPoint( float& x, float& y ) const {
		float sx = x;
		float sy = y;
		float dx = x;
		float dy = y;
		TransformPoint( dx, dy, Matrix.a, sx, sy );
		x = dx;
		y = dy;
	}
	void Multiply( const tTJSNI_Matrix32* mat ) {
		Multiply( Matrix.a, mat->Matrix.a );
		IsDirty = true;
	}
	void Premultiply( const tTJSNI_Matrix32* mat ) {
		Premultiply( Matrix.a, mat->Matrix.a );
		IsDirty = true;
	}

	/**
	 * 3x2行列の配列(6要素)として取得する。
	 */
	const float* GetMatrixArray() const;

	/**
	 * 4x4行列の配列(16要素)として取得する。内部的にキャッシュして持っている。
	 * 取得時変更があった場合に再設定。
	 * 読み取りのみで外部からの変更は想定されていないので注意。
	 */
	const float* GetMatrixArray16() const;
};


//---------------------------------------------------------------------------
// tTJSNC_Matrix32 : TJS Matrix32 class
//---------------------------------------------------------------------------
class tTJSNC_Matrix32 : public tTJSNativeClass
{
public:
	tTJSNC_Matrix32();
	static tjs_uint32 ClassID;

protected:
	// iTJSDispatch2 *CreateBaseTJSObject() override;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Matrix32(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Matrix32();
#endif
