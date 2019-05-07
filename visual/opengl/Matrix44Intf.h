/**
 * Matrix44 クラス
 */

#ifndef Matrix44IntfH
#define Matrix44IntfH

#include "tjsNative.h"

#include <glm/mat4x4.hpp>

class tTJSNI_Matrix44 : public tTJSNativeInstance
{
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

	static void Multiply( glm::mat4& m1, const glm::mat4& m2 ) {
		m1 = glm::mat4(
			m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2] + m1[3][0] * m2[0][3],
			m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2] + m1[3][1] * m2[0][3],
			m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2] + m1[3][2] * m2[0][3],
			m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2] + m1[3][3] * m2[0][3],
			m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2] + m1[3][0] * m2[1][3],
			m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2] + m1[3][1] * m2[1][3],
			m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2] + m1[3][2] * m2[1][3],
			m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2] + m1[3][3] * m2[1][3],
			m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2] + m1[3][0] * m2[2][3],
			m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2] + m1[3][1] * m2[2][3],
			m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2] + m1[3][2] * m2[2][3],
			m1[0][3] * m2[2][0] + m1[1][3] * m2[2][1] + m1[2][3] * m2[2][2] + m1[3][3] * m2[2][3],
			m1[0][0] * m2[3][0] + m1[1][0] * m2[3][1] + m1[2][0] * m2[3][2] + m1[3][0] * m2[3][3],
			m1[0][1] * m2[3][0] + m1[1][1] * m2[3][1] + m1[2][1] * m2[3][2] + m1[3][1] * m2[3][3],
			m1[0][2] * m2[3][0] + m1[1][2] * m2[3][1] + m1[2][2] * m2[3][2] + m1[3][2] * m2[3][3],
			m1[0][3] * m2[3][0] + m1[1][3] * m2[3][1] + m1[2][3] * m2[3][2] + m1[3][3] * m2[3][3]);
	}
	static void Premultiply( glm::mat4& m2, const glm::mat4& m1 ) {
		m2 = glm::mat4(
			m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2] + m1[3][0] * m2[0][3],
			m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2] + m1[3][1] * m2[0][3],
			m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2] + m1[3][2] * m2[0][3],
			m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2] + m1[3][3] * m2[0][3],
			m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2] + m1[3][0] * m2[1][3],
			m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2] + m1[3][1] * m2[1][3],
			m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2] + m1[3][2] * m2[1][3],
			m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2] + m1[3][3] * m2[1][3],
			m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2] + m1[3][0] * m2[2][3],
			m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2] + m1[3][1] * m2[2][3],
			m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2] + m1[3][2] * m2[2][3],
			m1[0][3] * m2[2][0] + m1[1][3] * m2[2][1] + m1[2][3] * m2[2][2] + m1[3][3] * m2[2][3],
			m1[0][0] * m2[3][0] + m1[1][0] * m2[3][1] + m1[2][0] * m2[3][2] + m1[3][0] * m2[3][3],
			m1[0][1] * m2[3][0] + m1[1][1] * m2[3][1] + m1[2][1] * m2[3][2] + m1[3][1] * m2[3][3],
			m1[0][2] * m2[3][0] + m1[1][2] * m2[3][1] + m1[2][2] * m2[3][2] + m1[3][2] * m2[3][3],
			m1[0][3] * m2[3][0] + m1[1][3] * m2[3][1] + m1[2][3] * m2[3][2] + m1[3][3] * m2[3][3] );
	}

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
		Mat4[0][0] = Mat4[1][1] = Mat4[2][2] = Mat4[3][3] = 1.0f;
		Mat4[0][1] = Mat4[0][2] = Mat4[0][3] = Mat4[1][0] =
		Mat4[1][2] = Mat4[1][3] = Mat4[2][0] = Mat4[2][1] =
		Mat4[2][3] = Mat4[3][0] = Mat4[3][1] = Mat4[3][2] = 0.0f;
	}
	iTJSDispatch2 * GetMatrixArrayObjectNoAddRef();

	void SetTranslate( tjs_real x, tjs_real y, tjs_real z = 0.0 ) {
		Mat4[3][0] = (float)x;
		Mat4[3][1] = (float)y;
		Mat4[3][2] = (float)z;
	}
	void SetScale( tjs_real x, tjs_real y, tjs_real z = 1.0 ) {
		Mat4[0][0] = (float)x;
		Mat4[1][1] = (float)y;
		Mat4[2][2] = (float)z;
	}
	void SetRotateZ( tjs_real deg );
	void SetRotate( tjs_real degree, tjs_real x, tjs_real y, tjs_real z );
	void RotateX( tjs_real degree, tjs_real px, tjs_real py, tjs_real pz );

	void Add( const tTJSNI_Matrix44* rhs );
	void Sub( const tTJSNI_Matrix44* rhs );
	void Mul( const tTJSNI_Matrix44* rhs );
	void Div( const tTJSNI_Matrix44* rhs );
	void Inc();
	void Dec();

	void MulVector( tjs_real& x, tjs_real& y, tjs_real& z, tjs_real& w );

	void Translate( tjs_real x, tjs_real y, tjs_real z = 0.0 );
	void Rotate( tjs_real degree, tjs_real x, tjs_real y, tjs_real z );
	void Scale( tjs_real x, tjs_real y, tjs_real z = 1.0 );
	void Ortho( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top, tjs_real znear, tjs_real zfar );
	void Ortho( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top );
	void Frustum( tjs_real left, tjs_real right, tjs_real bottom, tjs_real top, tjs_real znear, tjs_real zfar );
	void Perspective( tjs_real fovy, tjs_real aspect, tjs_real znear, tjs_real zfar );
	void PerspectiveFov( tjs_real fovy, tjs_real width, tjs_real height, tjs_real znear, tjs_real zfar );
	void LookAt( tjs_real eyeX, tjs_real eyeY, tjs_real eyeZ, tjs_real centerX, tjs_real centerY, tjs_real centerZ, tjs_real upX, tjs_real upY, tjs_real upZ );
	static void Project( const tTJSNI_Matrix44* model, const tTJSNI_Matrix44* proj, const class tTJSNI_Rect* viewport, tjs_real& x, tjs_real& y, tjs_real& z );


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
