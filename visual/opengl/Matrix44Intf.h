/**
 * Matrix44 クラス
 */

#ifndef Matrix44IntfH
#define Matrix44IntfH

#include "tjsNative.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

class tTJSNI_Matrix44 : public tTJSNativeInstance
{
	//static const tjs_real PI = 3.141592653589793;
	/*
	 0  1  2  3
	 4  5  6  7
	 8  9 10 11
	12 13 14 15
	*/
	/*
	union {
		float a[16];
		float m[4][4];
	} Matrix;
	*/

	glm::mat4 Mat4;

	iTJSDispatch2 *MatrixArray; // Array object which holds matrix array ...
	bool MatrixArrayValid;

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
	void Set( tjs_uint i, tjs_uint j, tjs_real a );
	tjs_real Get( tjs_uint i, tjs_uint j );
	void SetIdentity() {
		Mat4 = glm::mat4( 1.0f );

		Mat4[0][0] = Mat4[1][1] = Mat4[2][2] = Mat4[3][3] = 1.0f;
		Mat4[0][1] = Mat4[0][2] = Mat4[0][3] = Mat4[1][0] =
		Mat4[1][2] = Mat4[1][3] = Mat4[2][0] = Mat4[2][1] =
		Mat4[2][3] = Mat4[3][0] = Mat4[3][1] = Mat4[3][2] = 0.0f;
	}
	iTJSDispatch2 * GetMatrixArrayObjectNoAddRef();

	void SetTranslate( tjs_real x, tjs_real y, tjs_real z = 0.0 ) {
		//Mat4 = glm::translate( glm::vec3( (float)x, (float)y, (float)z) );
		Mat4[3][0] = (float)x;
		Mat4[3][1] = (float)x;
		Mat4[3][2] = (float)z;
	}
	void SetScale( tjs_real x, tjs_real y, tjs_real z = 1.0 ) {
		//Mat4 = glm::scale( glm::vec3( (float)x, (float)y, 1.0f ) );
		Mat4[0][0] = (float)x;
		Mat4[1][1] = (float)y;
		Mat4[2][2] = (float)z;
	}
	void SetRotateZ( tjs_real deg ) {
		tjs_real radian = deg * M_PI / 180.0;
		float c = (float)std::cos( radian );
		float s = (float)std::sin( radian );
		Mat4[0][0] = c;
		Mat4[1][0] = s;
		Mat4[0][1] = -s;
		Mat4[1][1] = c;
	}
	void Add( const tTJSNI_Matrix44* rhs ) { Mat4 += rhs->Mat4; }
	void Sub( const tTJSNI_Matrix44* rhs ) { Mat4 -= rhs->Mat4; }
	void Mul( const tTJSNI_Matrix44* rhs ) { Mat4 *= rhs->Mat4; }
	void Div( const tTJSNI_Matrix44* rhs ) { Mat4 /= rhs->Mat4; }
	void Inc() { Mat4++; }
	void Dec() { Mat4--; }

	const float *GetMatrixArray() const { return (const float *)&Mat4[0]; }
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
	// iTJSDispatch2 *CreateBaseTJSObject() override;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Matrix44(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Matrix44();
#endif
