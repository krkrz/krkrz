#include "ncbind/ncbind.hpp"
#include "IrrlichtDrawDevice.h"
#include "IrrlichtSimpleDevice.h"
#include "IrrlichtWindow.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace gui;

/**
 * ログ出力用
 */
void
message_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf_s(msg, 1024, _TRUNCATE, format, args);
	TVPAddLog(ttstr(msg));
	va_end(args);
}

/**
 * エラーログ出力用
 */
void
error_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf_s(msg, 1024, _TRUNCATE, format, args);
	TVPAddImportantLog(ttstr(msg));
	va_end(args);
}

// ----------------------------------------------------------------
// 実体型の登録
// 数値パラメータ系は配列か辞書を使えるような特殊コンバータを構築
// ----------------------------------------------------------------

// 両方自前コンバータ
#define NCB_SET_CONVERTOR_BOTH(type, convertor)\
NCB_TYPECONV_SRCMAP_SET(type, convertor<type>, true);\
NCB_TYPECONV_DSTMAP_SET(type, convertor<type>, true)

// SRCだけ自前コンバータ
#define NCB_SET_CONVERTOR_SRC(type, convertor)\
NCB_TYPECONV_SRCMAP_SET(type, convertor<type>, true);\
NCB_TYPECONV_DSTMAP_SET(type, ncbNativeObjectBoxing::Unboxing, true)

// DSTだけ自前コンバータ
#define NCB_SET_CONVERTOR_DST(type, convertor)\
NCB_TYPECONV_SRCMAP_SET(type, ncbNativeObjectBoxing::Boxing,   true); \
NCB_TYPECONV_DSTMAP_SET(type, convertor<type>, true)

/**
 * 配列かどうかの判定
 * @param var VARIANT
 * @return 配列なら true
 */
static bool IsArray(const tTJSVariant &var)
{
	if (var.Type() == tvtObject) {
		iTJSDispatch2 *obj = var.AsObjectNoAddRef();
		return obj->IsInstanceOf(0, NULL, NULL, L"Array", obj) == TJS_S_TRUE;
	}
	return false;
}

template <class T>
struct ColorConvertor { // 色用コンバータ
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) { // 配列から変換
					dst.setRed((u32)info.getIntValue(0, 0));
					dst.setGreen((u32)info.getIntValue(1, 0));
					dst.setBlue((u32)info.getIntValue(2, 0));
					dst.setAlpha((u32)info.getIntValue(3, 255));
				} else { // 辞書から変換
					dst.setRed((u32)info.getIntValue(L"r", 0));
					dst.setGreen((u32)info.getIntValue(L"g", 0));
					dst.setBlue((u32)info.getIntValue(L"b", 0));
					dst.setAlpha((u32)info.getIntValue(L"a", 255));
				}
			}
		} else {
			dst = T((u32)(tjs_int)src);
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
// XXX 自前登録サンプル
//	template <typename ANYT>
//	void operator ()(const tTJSVariant &dst, const ANYT &asrc) {
//		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
//		if (dict != NULL) {
//			T const* src = ncbTypeConvertor::ToPointer<const ANYT&>::Get(asrc);
//			tTJSVariant r(src->getRed());
//			dict->PropSet(TJS_MEMBERENSURE, L"r", NULL, &r, dict);
//			dst = tTJSVariant(dict, dict);
//			dict->Release();
//		}
//	}
private:
	T dst;
};
#define NCB_MEMBER_PROPERTY(name, type, membername) \
	struct AutoProp_ ## name { \
		static void ProxySet(Class *inst, type value) { inst->membername = value; } \
		static type ProxyGet(Class *inst) {      return inst->membername; } }; \
	NCB_PROPERTY_PROXY(name,AutoProp_ ## name::ProxyGet, AutoProp_ ## name::ProxySet)

#define NCB_PROPERTY2(name, getter, setter) NCB_PROPERTY(name,getter,setter);NCB_METHOD(getter);NCB_METHOD(setter)
#define NCB_PROPERTY_RO2(name, getter) NCB_PROPERTY_RO(name,getter);NCB_METHOD(getter)

NCB_SET_CONVERTOR_DST(SColor, ColorConvertor);
NCB_REGISTER_SUBCLASS_DELAY(SColor) {
	NCB_CONSTRUCTOR((u32));
	NCB_PROPERTY2(red, getRed, setRed);
	NCB_PROPERTY2(blue, getBlue, setBlue);
	NCB_PROPERTY2(green, getGreen, setGreen);
	NCB_PROPERTY2(alpha, getAlpha, setAlpha);
	NCB_PROPERTY_RO2(average, getAverage);
	NCB_METHOD(getInterpolated);
	NCB_METHOD(getInterpolated_quadratic);
	NCB_PROPERTY_RO2(luminance, getLuminance);
	NCB_METHOD(toA1R5G5B5);
	NCB_MEMBER_PROPERTY(color, u32, color);
};

template <class T>
struct ColorfConvertor { // 色用コンバータ
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) { // 配列から変換
					dst.r = (f32)info.getRealValue(0, 0);
					dst.g = (f32)info.getRealValue(1, 0);
					dst.b = (f32)info.getRealValue(2, 0);
					dst.a = (f32)info.getRealValue(3, 1.0);
				} else { // 辞書から変換
					dst.r = (f32)info.getRealValue(L"r", 0);
					dst.g = (f32)info.getRealValue(L"g", 0);
					dst.b = (f32)info.getRealValue(L"b", 0);
					dst.a = (f32)info.getRealValue(L"a", 1.0);
				}
			}
		} else {
			dst = T(SColor((u32)(tjs_int)src));
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};

NCB_SET_CONVERTOR_DST(SColorf, ColorfConvertor);
NCB_REGISTER_SUBCLASS_DELAY(SColorf) {
	NCB_CONSTRUCTOR((SColor));
	NCB_MEMBER_PROPERTY(red, f32, r);
	NCB_MEMBER_PROPERTY(blue, f32, b);
	NCB_MEMBER_PROPERTY(green, f32, g);
	NCB_MEMBER_PROPERTY(alpha, f32, a);
	NCB_METHOD(getInterpolated);
	NCB_METHOD(getInterpolated_quadratic);
	NCB_METHOD(setColorComponentValue);
	NCB_METHOD(toSColor);
};

template <class T>
struct DimensionConvertor { // コンバータ
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) { // 配列から変換
					dst.Width  = (s32)info.getIntValue(0);
					dst.Height = (s32)info.getIntValue(1);
				} else { // 辞書から変換
					dst.Width  = (s32)info.getIntValue(L"width");
					dst.Height = (s32)info.getIntValue(L"height");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};

template <class T>
static
dimension2d<T> *dimension2dFactory(iTJSDispatch2 *objthis, const T width, const T height)
{
	return new dimension2d<T>(width, height);
}

NCB_SET_CONVERTOR_DST(dimension2di, DimensionConvertor);
NCB_REGISTER_SUBCLASS_DELAY(dimension2di) {
	Factory(&dimension2dFactory<s32>);
	NCB_PROPERTY_RO2(area, getArea);
	NCB_MEMBER_PROPERTY(height, s32, Height);
	NCB_MEMBER_PROPERTY(width, s32, Width);
}

template <class T>
struct DimensionConvertor2 { // コンバータ
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) { // 配列から変換
					dst.Width  = (u32)info.getIntValue(0);
					dst.Height = (u32)info.getIntValue(1);
				} else { // 辞書から変換
					dst.Width  = (u32)info.getIntValue(L"width");
					dst.Height = (u32)info.getIntValue(L"height");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};
NCB_SET_CONVERTOR_DST(dimension2d<u32>, DimensionConvertor2);
NCB_REGISTER_SUBCLASS_DELAY(dimension2d<u32>) {
	NCB_CONSTRUCTOR(()); // XXX
	NCB_PROPERTY_RO2(area, getArea);
	NCB_MEMBER_PROPERTY(height, u32, Height);
	NCB_MEMBER_PROPERTY(width, u32, Width);
}

template <class T>
struct DimensionfConvertor { // コンバータ
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) { // 配列から変換
					dst.Width  = (f32)info.getRealValue(0);
					dst.Height = (f32)info.getRealValue(1);
				} else { // 辞書から変換
					dst.Width  = (f32)info.getRealValue(L"width");
					dst.Height = (f32)info.getRealValue(L"height");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};
NCB_SET_CONVERTOR_DST(dimension2df, DimensionfConvertor);
NCB_REGISTER_SUBCLASS_DELAY(dimension2df) {
	NCB_CONSTRUCTOR(()); // XXX
	NCB_PROPERTY_RO2(area, getArea);
	NCB_MEMBER_PROPERTY(height, f32, Height);
	NCB_MEMBER_PROPERTY(width, f32, Width);
}

template <class T>
struct PointConvertor {
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) {
					dst.X = (s32)info.getIntValue(0);
					dst.Y = (s32)info.getIntValue(1);
				} else {
					dst.X = (s32)info.getIntValue(L"x");
					dst.Y = (s32)info.getIntValue(L"y");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};
NCB_SET_CONVERTOR_DST(position2di, PointConvertor);
NCB_REGISTER_SUBCLASS_DELAY(position2di) {
	NCB_CONSTRUCTOR((s32, s32));
	NCB_MEMBER_PROPERTY(x, s32, X);
	NCB_MEMBER_PROPERTY(y, s32, Y);
};

template <class T>
struct PointfConvertor {
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) {
					dst.X = (f32)info.getRealValue(0);
					dst.Y = (f32)info.getRealValue(1);
				} else {
					dst.X = (f32)info.getRealValue(L"x");
					dst.Y = (f32)info.getRealValue(L"y");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};
NCB_SET_CONVERTOR_DST(position2df, PointfConvertor);
NCB_REGISTER_SUBCLASS_DELAY(position2df) {
	NCB_CONSTRUCTOR((f32, f32));
	NCB_MEMBER_PROPERTY(x, f32, X);
	NCB_MEMBER_PROPERTY(y, f32, Y);
};
NCB_SET_CONVERTOR_DST(vector2df, PointfConvertor);
NCB_REGISTER_SUBCLASS_DELAY(vector2df) {
	NCB_CONSTRUCTOR((f32, f32));
	NCB_MEMBER_PROPERTY(x, f32, X);
	NCB_MEMBER_PROPERTY(y, f32, Y);
	NCB_METHOD(equals);
	NCB_PROPERTY_RO2(angle, getAngle);
	NCB_PROPERTY_RO2(angleTrig, getAngleTrig);
	NCB_METHOD(getAngleWith);
	NCB_METHOD(getDistanceFrom);
	NCB_METHOD(getDistanceFromSQ);
	NCB_METHOD(getInterpolated);
	NCB_METHOD(getInterpolated_quadratic);
	NCB_PROPERTY_RO2(length, getLength);
	NCB_PROPERTY_RO2(lengthSQ, getLengthSQ);
	NCB_METHOD(interpolate);
	NCB_METHOD(isBetweenPoints);
};

template <class T>
struct Point3DConvertor {
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) {
					dst.X = (f32)info.getRealValue(0);
					dst.Y = (f32)info.getRealValue(1);
					dst.Z = (f32)info.getRealValue(2);
				} else {
					dst.X = (f32)info.getRealValue(L"x");
					dst.Y = (f32)info.getRealValue(L"y");
					dst.Z = (f32)info.getRealValue(L"z");
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};

NCB_SET_CONVERTOR_DST(vector3df, Point3DConvertor);
NCB_REGISTER_SUBCLASS_DELAY(vector3df) {
	NCB_CONSTRUCTOR((f32,f32,f32));
	NCB_MEMBER_PROPERTY(x, f32, X);
	NCB_MEMBER_PROPERTY(y, f32, Y);
	NCB_MEMBER_PROPERTY(z, f32, Z);
	NCB_METHOD(crossProduct);
	NCB_METHOD(dotProduct);
	NCB_METHOD(equals);
	NCB_METHOD(getDistanceFrom);
	NCB_METHOD(getDistanceFromSQ);
	NCB_PROPERTY_RO2(horizontalAngle, getHorizontalAngle);
	NCB_METHOD(getInterpolated);
	NCB_METHOD(getInterpolated_quadratic);
	NCB_PROPERTY2(length, getLength, setLength);
	NCB_PROPERTY_RO2(lengthSQ, getLengthSQ);
	NCB_METHOD(invert);
	NCB_METHOD(isBetweenPoints);
	NCB_METHOD(normalize);
	NCB_METHOD(rotateXYBy);
	NCB_METHOD(rotateXZBy);
	NCB_METHOD(rotateYZBy);
};

template <class T>
struct RectConvertor {
	typedef ncbInstanceAdaptor<T> AdaptorT;
	template <typename ANYT>
	void operator ()(ANYT &adst, const tTJSVariant &src) {
		if (src.Type() == tvtObject) {
			T *obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef());
			if (obj) {
				dst = *obj;
			} else {
				ncbPropAccessor info(src);
				if (IsArray(src)) {
					dst.UpperLeftCorner.X  = info.getIntValue(0);
					dst.UpperLeftCorner.Y  = info.getIntValue(1);
					dst.LowerRightCorner.X = info.getIntValue(2);
					dst.LowerRightCorner.Y = info.getIntValue(3);
				} else {
					int x = info.getIntValue(L"x");
					int y = info.getIntValue(L"y");
					int x2, y2;
					if (info.HasValue(L"width")) {
						x2 = x + info.getIntValue(L"width") - 1;
						y2 = x + info.getIntValue(L"height") - 1;
					} else {
						x2 = info.getIntValue(L"x2");
						y2 = info.getIntValue(L"y2");
					}
					dst.UpperLeftCorner.X  = x;
					dst.UpperLeftCorner.Y  = y;
					dst.LowerRightCorner.X = x2;
					dst.LowerRightCorner.Y = y2;
				}
			}
		} else {
			dst = T();
		}
		adst = ncbTypeConvertor::ToTarget<ANYT>::Get(&dst);
	}
private:
	T dst;
};
NCB_SET_CONVERTOR_DST(rect<s32>, RectConvertor);
NCB_REGISTER_SUBCLASS_DELAY(rect<s32>) {
	NCB_CONSTRUCTOR((s32, s32, s32, s32));
//	NCB_METHOD(addInternalPoint);
	NCB_METHOD(clipAgainst);
	NCB_METHOD(constrainTo);
	NCB_PROPERTY_RO2(area, getArea);
	NCB_PROPERTY_RO2(center, getCenter);
	NCB_PROPERTY_RO2(size, getSize);
	NCB_METHOD(isPointInside);
	NCB_METHOD(isRectCollided);
	NCB_METHOD(isValid);
	NCB_METHOD(repair);
	NCB_PROPERTY2(width, getWidth, setWidth);
	NCB_PROPERTY2(height, getHeight, setHeight);
	NCB_PROPERTY2(left, getLeft, setLeft);
	NCB_PROPERTY2(top, getTop, setTop);
};

NCB_REGISTER_SUBCLASS(matrix4) {
	NCB_CONSTRUCTOR(());
	NCB_METHOD(buildCameraLookAtMatrixLH);
	NCB_METHOD(buildCameraLookAtMatrixRH);
	NCB_METHOD(buildNDCToDCMatrix);
	NCB_METHOD(buildProjectionMatrixOrthoLH);
	NCB_METHOD(buildProjectionMatrixOrthoRH);
	NCB_METHOD(buildProjectionMatrixPerspectiveFovLH);
	NCB_METHOD(buildProjectionMatrixPerspectiveFovRH);
	NCB_METHOD(buildProjectionMatrixPerspectiveLH);
	NCB_METHOD(buildProjectionMatrixPerspectiveRH);
	NCB_METHOD(buildShadowMatrix);
	NCB_METHOD(buildTextureTransform);
	NCB_METHOD(getDefinitelyIdentityMatrix);
	NCB_METHOD(getInverse);
	NCB_METHOD(getInversePrimitive);
	NCB_METHOD(getRotationDegrees);
	NCB_METHOD(getScale);
	NCB_METHOD(getTranslation);
	//getTransposed (CMatrix4< T > &dest) const  
	//getTransposed () const  
	NCB_METHOD(interpolate);
	NCB_METHOD(inverseRotateVect);
	NCB_METHOD(inverseTranslateVect);
	NCB_METHOD(isIdentity);
	NCB_METHOD(isIdentity_integer_base);
	NCB_METHOD(makeIdentity);
	NCB_METHOD(makeInverse);
//	NCB_METHOD(multiplyWith1x4Matrix);
	//rotateVect (T *out, const core::vector3df &in) const  
	//rotateVect (core::vector3df &out, const core::vector3df &in) const  
	//rotateVect (vector3df &vect) const  
	NCB_METHOD(setbyproduct);
	NCB_METHOD(setbyproduct_nocheck);
	NCB_METHOD(setDefinitelyIdentityMatrix);
	NCB_METHOD(setInverseRotationDegrees);
	NCB_METHOD(setInverseRotationRadians);
	NCB_METHOD(setInverseTranslation);
	//setM (const T *data) 
	NCB_METHOD(setRotationDegrees);
	NCB_METHOD(setRotationRadians);
	//setScale (const T scale) 
	//setScale (const vector3d< T > &scale) 
	NCB_METHOD(setTextureRotationCenter);
	NCB_METHOD(setTextureScale);
	NCB_METHOD(setTextureScaleCenter);
	NCB_METHOD(setTextureTranslate);
	NCB_METHOD(setTranslation);
	NCB_METHOD(transformBox);
	NCB_METHOD(transformBoxEx);
	//transformPlane (const core::plane3d< f32 > &in, core::plane3d< f32 > &out) const  
	//transformPlane (core::plane3d< f32 > &plane) const  
	NCB_METHOD(transformPlane_new);
	//transformVect (T *out, const core::vector3df &in) const  
	//transformVect (vector3df &out, const vector3df &in) const  
	//transformVect (vector3df &vect) const  
	NCB_METHOD(translateVect);
};

NCB_REGISTER_SUBCLASS(triangle3df) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(plane3df) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(aabbox3df) {
	NCB_CONSTRUCTOR(());
//	NCB_METHOD(addInternalPoint);
	NCB_METHOD(addInternalBox);
	//NCB_METHOD(classifyPlaneRelation);
	NCB_METHOD(getCenter);
	//NCB_METHOD(getEdges);
	NCB_METHOD(getExtent);
	NCB_METHOD(getInterpolated);
	NCB_METHOD(intersectsWithBox);
//	NCB_METHOD(intersectsWithLine);
	NCB_METHOD(isEmpty);
	NCB_METHOD(isFullInside);
	NCB_METHOD(isPointInside);
	NCB_METHOD(isPointTotalInside);
	NCB_METHOD(repair);
	//NCB_METHOD(reset);
	NCB_MEMBER_PROPERTY(maxEdge, vector3df, MaxEdge);
	NCB_MEMBER_PROPERTY(minEdge, vector3df, MinEdge);
}

NCB_REGISTER_SUBCLASS(SLight) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(SMaterial) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(quake3::SShader) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(SAttributeReadWriteOptions) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(SEvent) {
	NCB_CONSTRUCTOR(());
}

NCB_REGISTER_SUBCLASS(SViewFrustum) {
	NCB_CONSTRUCTOR(());
}


// --------------------------------------------------------------------
// Irrlicht の参照オブジェクトの型の登録
// Irrlicht のインターフェースをそのまま保持できるように工夫
// --------------------------------------------------------------------

/**
 * Irrlichtの参照オブジェクトのラッピング用テンプレートクラス
 */
template <class T>
class IrrWrapper {
	typedef T IrrClassT;
	typedef IrrWrapper<IrrClassT> WrapperT;
protected:
	IrrClassT *obj;
public:
	IrrWrapper() : obj(NULL) {}
	IrrWrapper(IrrClassT *obj) : obj(obj) {
		if (obj) {
			obj->grab();
		}
	}
	IrrWrapper(const IrrWrapper &orig) : obj(orig.obj) {
		if (obj) {
			obj->grab();
		}
	}
	~IrrWrapper() {
		if (obj) {
			obj->drop();
			obj=NULL;
		}
	}
	IrrClassT *getIrrObject() { return obj; }

	struct BridgeFunctor {
		IrrClassT* operator()(WrapperT *p) const {
			return p->getIrrObject();
		}
	};

	template <class CastT>
	struct CastBridgeFunctor {
		CastT* operator()(WrapperT *p) const {
			return (CastT*)p->getIrrObject();
		}
	};

};

/**
 * Irrlichtの参照オブジェクトをラッピングしたクラス用のコンバータ（汎用）
 */
template <class T>
struct IrrTypeConvertor {
	typedef typename ncbTypeConvertor::Stripper<T>::Type IrrClassT;
	typedef T *IrrClassP;
	typedef IrrWrapper<IrrClassT> WrapperT;
	typedef ncbInstanceAdaptor<WrapperT> AdaptorT;

	static IrrClassT *getIrrObject(const tTJSVariant &src) {
		WrapperT *obj;
		if (src.Type() == tvtObject && (obj = AdaptorT::GetNativeInstance(src.AsObjectNoAddRef()))) {
			return obj->getIrrObject();
		} else {
			return NULL;
		}
	}
	void operator ()(IrrClassP &dst, const tTJSVariant &src) {
		dst = getIrrObject(src);
	}
	void operator ()(tTJSVariant &dst, const IrrClassP &src) {
		if (src != NULL) {
			iTJSDispatch2 *adpobj = AdaptorT::CreateAdaptor(new WrapperT(src));
			if (adpobj) {
				dst = tTJSVariant(adpobj, adpobj);
				adpobj->Release();			
			} else {
				dst = NULL;
			}
		} else {
			dst.Clear();
		}
	}
};

// Irrlicht参照オブジェクト用のコンバータの登録マクロ
#define NCB_IRR_CONVERTOR(type) \
NCB_SET_CONVERTOR(type*, IrrTypeConvertor<type>);\
NCB_SET_CONVERTOR(const type*, IrrTypeConvertor<const type>)

// ラッピング処理用
#define NCB_REGISTER_IRR_SUBCLASS(Class) NCB_IRR_CONVERTOR(Class);NCB_REGISTER_SUBCLASS(IrrWrapper<Class>) { typedef Class IrrClass;
#define NCB_IRR_METHOD(name)  Method(TJS_W(# name), &IrrClass::name, Bridge<IrrWrapper<IrrClass>::BridgeFunctor>())
#define NCB_IRR_MCAST(ret, method, args) static_cast<ret (IrrClass::*) args>(&IrrClass::method)
#define NCB_IRR_METHOD2(name, ret, method, args) Method(TJS_W(# name), NCB_IRR_MCAST(ret, method, args), Bridge<IrrWrapper<IrrClass>::BridgeFunctor>())
#define NCB_IRR_PROPERTY(name,get,set)  Property(TJS_W(# name), &IrrClass::get, &IrrClass::set, Bridge<IrrWrapper<IrrClass>::BridgeFunctor>())
#define NCB_IRR_MEMBER_PROPERTY(name, type, membername) \
	struct AutoProp_ ## name { \
		static void ProxySet(IrrClass *inst, type value) { inst->membername = value; } \
		static type ProxyGet(IrrClass *inst) {      return inst->membername; } }; \
	Property(TJS_W(#name), AutoProp_ ## name::ProxyGet, AutoProp_ ## name::ProxySet, Bridge<IrrWrapper<IrrClass>::BridgeFunctor>())

#define NCB_IRR_PROPERTY2(name,get,set)  NCB_IRR_PROPERTY(name,get,set);NCB_IRR_METHOD(get);NCB_IRR_METHOD(set)

NCB_REGISTER_IRR_SUBCLASS(IAttributes)
	NCB_CONSTRUCTOR(());
};

// Create系の、参照が１つ余分にはいった帰り値の処理
// tTJSVariant に置換後 drop() することで参照の値を調整する
// @param name 吉里吉里に登録する名前
// @param type 帰り値の型
// @param func 呼び出すメソッド名
// @param arg 登録する引数リスト。型も必要。第一引数に必ず IrrWrapper<IrrClass> *obj が必要
// @param argto 処理呼び出し用の引数リスト。名前リストのみ
//
#define NCB_IRR_CREATE_NAME(name, type, func, arg, argto)\
struct AutoCreate_ ## name {\
	static tTJSVariant Func arg {\
		type *ret = obj->getIrrObject()->func argto;\
		tTJSVariant var;\
		IrrTypeConvertor<type> conv;\
		conv(var, ret);\
		ret->drop();\
		return var;\
	}\
};\
NCB_METHOD_PROXY(name, AutoCreate_ ## name::Func)
#define NCB_IRR_CREATE(name, type, arg, argto) NCB_IRR_CREATE_NAME(name, type, name, arg, argto)


NCB_REGISTER_IRR_SUBCLASS(ISceneNodeAnimator)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(animateNode);
NCB_IRR_METHOD(getType);
NCB_IRR_CREATE(createClone, IrrClass, (IrrWrapper<IrrClass> *obj, ISceneNode *node, ISceneManager *newManager), (node, newManager));
};

NCB_REGISTER_IRR_SUBCLASS(ISceneNodeAnimatorCollisionResponse)
NCB_CONSTRUCTOR(());
NCB_IRR_PROPERTY2(ellipsoidRadius, getEllipsoidRadius, setEllipsoidRadius);
NCB_IRR_PROPERTY2(ellipsoidTranslation, getEllipsoidTranslation, setEllipsoidTranslation);
NCB_IRR_PROPERTY2(gravity, getGravity, setGravity);
NCB_IRR_PROPERTY2(world, getWorld, setWorld);
NCB_IRR_METHOD(isFalling);
};

/**
 * IMesh 専用コンバータ
 */
template <class T>
struct IMeshTypeConvertor {
	typedef typename ncbTypeConvertor::Stripper<T>::Type IrrClassT;
	typedef T *IrrClassP;
	typedef IrrWrapper<IrrClassT> WrapperT;
	typedef ncbInstanceAdaptor<WrapperT> AdaptorT;
	
	void operator ()(IrrClassP &dst, const tTJSVariant &src) {
#define GET_IRR_NATIVEINSTANCE(src, type) (IrrClassP)IrrTypeConvertor<type>::getIrrObject(src)
		IrrClassP node;
		if ((node = GET_IRR_NATIVEINSTANCE(src, IAnimatedMesh)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IMesh))
			) {
			dst = node;
		} else {
			dst = NULL;
		}
	}
	void operator ()(tTJSVariant &dst, const IrrClassP &src) {
		if (src != NULL) {
#define GET_IRR_ADAPTOR(src, type) ncbInstanceAdaptor<IrrWrapper<type>>::CreateAdaptor(new IrrWrapper<type>((type*)src))
			iTJSDispatch2 *adpobj;
			adpobj = AdaptorT::CreateAdaptor(new WrapperT(src));
			if (adpobj) {
				dst = tTJSVariant(adpobj, adpobj);
				adpobj->Release();			
			} else {
				dst = NULL;
			}
		} else {
			dst.Clear();
		}
	}
};

NCB_SET_CONVERTOR(IMesh*, IMeshTypeConvertor<IMesh>);
NCB_SET_CONVERTOR(const IMesh*, IMeshTypeConvertor<const IMesh>);
NCB_REGISTER_SUBCLASS(IrrWrapper<IMesh>) {
NCB_CONSTRUCTOR(());
	typedef IMesh IrrClass;
#define IMESH_METHOD \
NCB_IRR_PROPERTY2(boundingBox, getBoundingBox, setBoundingBox);\
NCB_IRR_METHOD2(getMeshBuffer, IMeshBuffer*, getMeshBuffer, (u32) const);\
NCB_IRR_METHOD2(getMeshBuffer2, IMeshBuffer*, getMeshBuffer, (const SMaterial &) const);\
NCB_IRR_METHOD(getMeshBufferCount);\
NCB_IRR_METHOD(setMaterialFlag)
IMESH_METHOD;
};

NCB_REGISTER_IRR_SUBCLASS(IMeshBuffer)
NCB_CONSTRUCTOR(());
NCB_IRR_PROPERTY2(boundingBox, getBoundingBox, setBoundingBox);
NCB_IRR_METHOD2(append, void, append, (const IMeshBuffer * const));
//NCB_IRR_METHOD2(append2)
NCB_IRR_METHOD(getIndexCount);
//NCB_IRR_METHOD(getIndices) XXX 配列
NCB_IRR_METHOD2(getMaterial, SMaterial&, getMaterial, (void));
NCB_IRR_METHOD(getVertexCount);
NCB_IRR_METHOD(getVertexType);
//NCB_IRR_METHOD(getvertices) XXX 配列
};

NCB_REGISTER_IRR_SUBCLASS(IWriteFile)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(getFileName);
NCB_IRR_METHOD(getPos);
NCB_IRR_METHOD(seek);
//write (const void *buffer, u32 sizeToWrite)=0 
};

NCB_REGISTER_IRR_SUBCLASS(IReadFile)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(getFileName);
NCB_IRR_METHOD(getPos);
NCB_IRR_METHOD(getSize);
//read (void *buffer, u32 sizeToRead)=0 
NCB_IRR_METHOD(seek);
};

NCB_REGISTER_IRR_SUBCLASS(IFileList)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(IMeshCache)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(IMeshManipulator)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(IMeshWriter)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(getType);
NCB_IRR_METHOD(writeMesh);
};

NCB_REGISTER_IRR_SUBCLASS(IAnimatedMesh)
NCB_CONSTRUCTOR(());
IMESH_METHOD;
NCB_IRR_METHOD(getFrameCount);
NCB_IRR_METHOD(getMesh);
NCB_IRR_METHOD(getMeshType);
};

NCB_REGISTER_IRR_SUBCLASS(IGUIFont)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ITriangleSelector)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ISceneNodeAnimatorFactory)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ISceneNodeFactory)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ISceneCollisionManager)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(IAnimationEndCallBack)
	NCB_CONSTRUCTOR(());
};

NCB_TYPECONV_CAST_INTEGER(E_CULLING_TYPE);
NCB_TYPECONV_CAST_INTEGER(E_MATERIAL_FLAG);
NCB_TYPECONV_CAST_INTEGER(E_MATERIAL_TYPE);
NCB_TYPECONV_CAST_INTEGER(ESCENE_NODE_TYPE);
NCB_TYPECONV_CAST_INTEGER(E_SCENE_NODE_RENDER_PASS);
NCB_TYPECONV_CAST_INTEGER(EMESH_WRITER_TYPE);
NCB_TYPECONV_CAST_INTEGER(E_TRANSFORMATION_STATE);
NCB_TYPECONV_CAST_INTEGER(E_TEXTURE_CREATION_FLAG);
NCB_TYPECONV_CAST_INTEGER(E_VIDEO_DRIVER_FEATURE);
NCB_TYPECONV_CAST_INTEGER(ECOLOR_FORMAT);
NCB_TYPECONV_CAST_INTEGER(E_TERRAIN_PATCH_SIZE);
NCB_TYPECONV_CAST_INTEGER(E_JOINT_UPDATE_ON_RENDER);
NCB_TYPECONV_CAST_INTEGER(EMD2_ANIMATION_TYPE);
NCB_TYPECONV_CAST_INTEGER(E_BONE_ANIMATION_MODE);
NCB_TYPECONV_CAST_INTEGER(E_BONE_SKINNING_SPACE);
NCB_TYPECONV_CAST_INTEGER(ESCENE_NODE_ANIMATOR_TYPE);
NCB_TYPECONV_CAST_INTEGER(E_VERTEX_TYPE);
NCB_TYPECONV_CAST_INTEGER(ELOG_LEVEL);
NCB_TYPECONV_CAST_INTEGER(E_ANIMATED_MESH_TYPE);

/**
 * ISceneNode 専用コンバータ
 */
template <class T>
struct ISceneNodeTypeConvertor {
	typedef typename ncbTypeConvertor::Stripper<T>::Type IrrClassT;
	typedef T *IrrClassP;
	typedef IrrWrapper<IrrClassT> WrapperT;
	typedef ncbInstanceAdaptor<WrapperT> AdaptorT;
	
	void operator ()(IrrClassP &dst, const tTJSVariant &src) {
#define GET_IRR_NATIVEINSTANCE(src, type) (IrrClassP)IrrTypeConvertor<type>::getIrrObject(src)
		IrrClassP node;
		if ((node = GET_IRR_NATIVEINSTANCE(src, IAnimatedMeshSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IBillboardSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IBoneSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, ICameraSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IDummyTransformationSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, ILightSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IMeshSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IParticleSystemSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, ITerrainSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, ITextSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, IShadowVolumeSceneNode)) ||
			(node = GET_IRR_NATIVEINSTANCE(src, ISceneNode))
			) {
			dst = node;
		} else {
			dst = NULL;
		}
	}
	void operator ()(tTJSVariant &dst, const IrrClassP &src) {
		if (src != NULL) {
#define GET_IRR_ADAPTOR(src, type) ncbInstanceAdaptor<IrrWrapper<type>>::CreateAdaptor(new IrrWrapper<type>((type*)src))
			iTJSDispatch2 *adpobj;
			switch (src->getType()) {
			case ESNT_TEXT:	    adpobj = GET_IRR_ADAPTOR(src, ITextSceneNode); break;
			case ESNT_TERRAIN:	adpobj = GET_IRR_ADAPTOR(src, ITerrainSceneNode); break;
			case ESNT_MESH:		adpobj = GET_IRR_ADAPTOR(src, IMeshSceneNode); break;
			case ESNT_LIGHT:    adpobj = GET_IRR_ADAPTOR(src, ILightSceneNode);	break;
			case ESNT_DUMMY_TRANSFORMATION:	adpobj = GET_IRR_ADAPTOR(src, IDummyTransformationSceneNode);break;
			case ESNT_CAMERA:
			case ESNT_CAMERA_MAYA:
			case ESNT_CAMERA_FPS: adpobj = GET_IRR_ADAPTOR(src, ICameraSceneNode); break;
			case ESNT_BILLBOARD:  adpobj = GET_IRR_ADAPTOR(src, IBillboardSceneNode); break;
			case ESNT_ANIMATED_MESH: adpobj = GET_IRR_ADAPTOR(src, IAnimatedMeshSceneNode); break;
			case ESNT_PARTICLE_SYSTEM: adpobj = GET_IRR_ADAPTOR(src, IParticleSystemSceneNode); break;
			default:
				adpobj = AdaptorT::CreateAdaptor(new WrapperT(src));
				break;
			}
			if (adpobj) {
				dst = tTJSVariant(adpobj, adpobj);
				adpobj->Release();			
			} else {
				dst = NULL;
			}
		} else {
			dst.Clear();
		}
	}
};
NCB_SET_CONVERTOR(ISceneNode*, ISceneNodeTypeConvertor<ISceneNode>);
NCB_SET_CONVERTOR(const ISceneNode*, ISceneNodeTypeConvertor<const ISceneNode>);
NCB_REGISTER_SUBCLASS(IrrWrapper<ISceneNode>) {
	NCB_CONSTRUCTOR(());
	typedef ISceneNode IrrClass;
#define ISCENENODE_METHOD \
	NCB_IRR_PROPERTY2(automaticCulling, getAutomaticCulling, setAutomaticCulling);\
	NCB_IRR_PROPERTY2(id, getID, setID);\
	NCB_IRR_PROPERTY2(name, getName, setName);\
	NCB_IRR_PROPERTY2(parent, getParent, setParent);\
	NCB_IRR_PROPERTY2(position, getPosition, setPosition);\
	NCB_IRR_PROPERTY2(rotation, getRotation, setRotation);\
	NCB_IRR_PROPERTY2(scale, getScale, setScale);\
	NCB_IRR_PROPERTY2(triangleSelector, getTriangleSelector, setTriangleSelector);\
	NCB_IRR_PROPERTY2(debugDataVisible, isDebugDataVisible, setDebugDataVisible);\
	NCB_IRR_PROPERTY2(visible, isVisible, setVisible);\
	NCB_IRR_METHOD(addAnimator);\
	NCB_IRR_METHOD(addChild);\
	NCB_IRR_METHOD(clone);\
	NCB_IRR_METHOD(deserializeAttributes);\
	NCB_IRR_METHOD(getAbsolutePosition);\
	NCB_IRR_METHOD(getAbsoluteTransformation);\
	/**NCB_IRR_METHOD(getAnimators); XXX リストを返してる*/\
	NCB_IRR_METHOD(getBoundingBox);\
	/**NCB_IRR_METHOD(getChildren); XXX リストを返してる*/\
	NCB_IRR_METHOD(getMaterial);\
	NCB_IRR_METHOD(getMaterialCount);\
	NCB_IRR_METHOD(getRelativeTransformation);\
	NCB_IRR_METHOD(getTransformedBoundingBox);\
	NCB_IRR_METHOD(getType);\
	NCB_IRR_METHOD(isDebugObject);\
	NCB_IRR_METHOD(remove);\
	NCB_IRR_METHOD(removeAll);\
	NCB_IRR_METHOD(removeAnimator);\
	NCB_IRR_METHOD(removeAnimators);\
	NCB_IRR_METHOD(removeChild);\
	NCB_IRR_METHOD(render);\
	NCB_IRR_METHOD(serializeAttributes);\
	NCB_IRR_METHOD(setMaterialFlag);\
	NCB_IRR_METHOD(setMaterialTexture);\
	NCB_IRR_METHOD(setMaterialType);\
	NCB_IRR_METHOD(updateAbsolutePosition)
	ISCENENODE_METHOD;
};

NCB_REGISTER_IRR_SUBCLASS(IShadowVolumeSceneNode)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(setMeshToRenderFrom);
};

NCB_REGISTER_IRR_SUBCLASS(IAnimatedMeshSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_PROPERTY2(mesh, getMesh, setMesh);
NCB_IRR_PROPERTY2(readOnlymaterials, isReadOnlyMaterials, setReadOnlyMaterials);
NCB_IRR_METHOD(addShadowVolumeSceneNode);
NCB_IRR_METHOD(animateJoints);
NCB_IRR_METHOD(getEndFrame);
NCB_IRR_METHOD(getFrameNr);
NCB_IRR_METHOD2(getJointNode,  IBoneSceneNode*, getJointNode, (const c8*));
NCB_IRR_METHOD2(getJointNode2, IBoneSceneNode*, getJointNode, (u32));
//NCB_IRR_METHOD(getMD3TagTransformation)
NCB_IRR_METHOD(getMS3DJointNode);
NCB_IRR_METHOD(getStartFrame);
NCB_IRR_METHOD(getXJointNode);
NCB_IRR_METHOD(setAnimationEndCallback);
NCB_IRR_METHOD(setAnimationSpeed);
NCB_IRR_METHOD(setCurrentFrame);
NCB_IRR_METHOD(setFrameLoop);
NCB_IRR_METHOD(setJointMode);
NCB_IRR_METHOD(setLoopMode);
NCB_IRR_METHOD2(setMD2Animation, bool, setMD2Animation, (EMD2_ANIMATION_TYPE));
NCB_IRR_METHOD2(setMD2Animation, bool, setMD2Animation, (const c8 *));
NCB_IRR_METHOD(setRenderFromIdentity);
NCB_IRR_METHOD(setTransitionTime);
};

NCB_REGISTER_IRR_SUBCLASS(IBillboardSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_PROPERTY2(size, getSize, setSize);
NCB_IRR_METHOD(getColor);
NCB_IRR_METHOD2(setColor,  void, setColor, (const SColor&, const SColor &));
NCB_IRR_METHOD2(setColor2, void, setColor, (const SColor&));
};

NCB_REGISTER_IRR_SUBCLASS(IBoneSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_MEMBER_PROPERTY(positionHint, s32, positionHint);
NCB_IRR_MEMBER_PROPERTY(rotationHint, s32, rotationHint);
NCB_IRR_MEMBER_PROPERTY(scaleHint, s32, scaleHint);
NCB_IRR_PROPERTY2(animationMode, getAnimationMode, setAnimationMode);
NCB_IRR_PROPERTY2(skinningSpace, getSkinningSpace, setSkinningSpace);
NCB_IRR_METHOD(getBoneIndex);
NCB_IRR_METHOD(getBoneName);
//NCB_IRR_METHOD(getBoundingBox);
//NCB_IRR_METHOD(render);
NCB_IRR_METHOD(updateAbsolutePositionOfAllChildren);
};

NCB_REGISTER_IRR_SUBCLASS(ICameraSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_PROPERTY2(aspectRatio, getAspectRatio, setAspectRatio);
NCB_IRR_PROPERTY2(farValue, getFarValue, setFarValue);
NCB_IRR_PROPERTY2(fov, getFOV, setFOV);
NCB_IRR_PROPERTY2(nearValue, getNearValue, setNearValue);
NCB_IRR_PROPERTY2(projectionMatrix, getProjectionMatrix, setProjectionMatrix);
NCB_IRR_PROPERTY2(target, getTarget, setTarget);
NCB_IRR_PROPERTY2(upVector, getUpVector, setUpVector);
NCB_IRR_PROPERTY2(inputReceiverEnabled, isInputReceiverEnabled, setInputReceiverEnabled);
NCB_IRR_PROPERTY2(orthogonal, isOrthogonal, setIsOrthogonal);
NCB_IRR_METHOD(getViewFrustum);
NCB_IRR_METHOD(getViewMatrix);
};

NCB_REGISTER_IRR_SUBCLASS(IDummyTransformationSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_METHOD(getRelativeTransformationMatrix);
};

NCB_REGISTER_IRR_SUBCLASS(ILightSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
//NCB_IRR_PROPERTY2(lightData, NCB_IRR_MCAST(SLight&,getLightData,(void)), setLightData);
NCB_IRR_METHOD(setLightData);
NCB_IRR_METHOD2(getLightData, SLight&, getLightData, (void));
};

NCB_REGISTER_IRR_SUBCLASS(IMeshSceneNode)
NCB_CONSTRUCTOR(());
ISCENENODE_METHOD;
NCB_IRR_PROPERTY2(mesh, getMesh, setMesh);
NCB_IRR_PROPERTY2(readOnlymaterials, isReadOnlyMaterials, setReadOnlyMaterials);
};

NCB_REGISTER_IRR_SUBCLASS(IParticleSystemSceneNode)
	NCB_CONSTRUCTOR(());
	ISCENENODE_METHOD;
};

NCB_REGISTER_IRR_SUBCLASS(ITerrainSceneNode)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ITextSceneNode)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(setText);
NCB_IRR_METHOD(setTextColor);
};

NCB_REGISTER_IRR_SUBCLASS(IImage)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(ITexture)
	NCB_CONSTRUCTOR(());
};

NCB_REGISTER_IRR_SUBCLASS(IVideoDriver)
NCB_CONSTRUCTOR(());
NCB_IRR_PROPERTY2(viewport, getViewPort, setViewPort);
NCB_IRR_METHOD(addDynamicLight);
NCB_IRR_METHOD2(addTexture, ITexture*, addTexture, (const dimension2d<s32>&, const c8*, ECOLOR_FORMAT));
NCB_IRR_METHOD2(addTexture2, ITexture*, addTexture, (const c8*, IImage *));
//NCB_IRR_METHOD(beginScene);
NCB_IRR_METHOD(clearZBuffer);
NCB_IRR_CREATE(createAttributesFromMaterial, IAttributes, (IrrWrapper<IrrClass> *obj, const video::SMaterial &material), (material));
NCB_IRR_CREATE_NAME(createImage,  IImage, createImage, (IrrWrapper<IrrClass> *obj, ECOLOR_FORMAT format, const core::dimension2d<s32>&size), (format, size));
NCB_IRR_CREATE_NAME(createImage2, IImage, createImage, (IrrWrapper<IrrClass> *obj, ECOLOR_FORMAT format, IImage *imageToCopy), (format, imageToCopy));
NCB_IRR_CREATE_NAME(createImage3, IImage, createImage, (IrrWrapper<IrrClass> *obj, IImage *imageToCopy, const core::position2d< s32 > &pos, const core::dimension2d< s32 > &size),(imageToCopy, pos, size));
NCB_IRR_CREATE(createImageFromFile, IImage, (IrrWrapper<IrrClass> *obj, const c8 *filename), (filename));
NCB_IRR_CREATE(createRenderTargetTexture, ITexture, (IrrWrapper<IrrClass> *obj, const core::dimension2d< s32 > &size, const c8 *name), (size, name));
NCB_IRR_CREATE(createScreenShot, IImage, (IrrWrapper<IrrClass> *obj), ());
NCB_IRR_METHOD(deleteAllDynamicLights);
NCB_IRR_METHOD2(draw2DImage,  void, draw2DImage, (const ITexture*texture, const position2d<s32>&destPos));
NCB_IRR_METHOD2(draw2DImage2,  void, draw2DImage, (const ITexture*texture,
												  const position2d<s32>&destPos,
												  const core::rect<s32>&sourceRect,
												  const core::rect<s32>*clipRect,
												  SColor color,
												  bool useAlphaChannelOfTexture));
//NCB_IRR_METHOD2(draw2DImage,  void, draw2DImage, (const ITexture* texture, const core::position2d<s32>&,
//												  const core::array<core::rect<s32> >&, const core::array<s32>&, s32,
//												  const core::rect<s32>*, SColor,  bool)); XXX 配列
//NCB_IRR_METHOD2(draw2DImage,  void, draw2DImage, (const ITexture*, const rect<s32>& destRect,
//												  const rect<s32>& sourceRect,
//												  const rect<s32>* clipRect,
//												  SColor *color, bool);  XXX 配列
NCB_IRR_METHOD(draw2DLine);
NCB_IRR_METHOD(draw2DPolygon);
//NCB_IRR_METHOD2(draw2DRectangle,  method_cast<void, Class, >(&IrrClass::draw2DRectangle)); XXX 配列
//NCB_IRR_METHOD2(draw2DRectangle2, method_cast<void, Class, >(&IrrClass::draw2DRectangle)); XXX 配列
NCB_IRR_METHOD(draw3DBox);
NCB_IRR_METHOD(draw3DLine);
NCB_IRR_METHOD(draw3DTriangle);
//NCB_IRR_METHOD(drawIndexedTriangleFan); XXX 配列
//NCB_IRR_METHOD(drawIndexedTriangleFan2); XXX 配列
//NCB_IRR_METHOD(drawIndexedTriangleList); XXX 配列
//NCB_IRR_METHOD(drawIndexedTriangleList2); XXX 配列
//NCB_IRR_METHOD(drawIndexedTriangleList3); XXX 配列
NCB_IRR_METHOD(drawMeshBuffer);
NCB_IRR_METHOD(drawStencilShadow);
NCB_IRR_METHOD(drawStencilShadowVolume);
//NCB_IRR_METHOD(drawVertexPrimitiveList); XXX配列
NCB_IRR_METHOD(enableClipPlane);
//NCB_IRR_METHOD(endScene);
NCB_IRR_METHOD(fillMaterialStructureFromAttributes);
NCB_IRR_METHOD(findTexture);
NCB_IRR_METHOD(getCurrentRenderTargetSize);
NCB_IRR_METHOD(getDriverType);
NCB_IRR_METHOD(getDynamicLight);
NCB_IRR_METHOD(getFPS);
NCB_IRR_METHOD(getMaximalDynamicLightAmount);
NCB_IRR_METHOD(getMaximalPrimitiveCount);
NCB_IRR_METHOD(getMeshManipulator);
NCB_IRR_METHOD(getName);
NCB_IRR_METHOD(getPrimitiveCountDrawn);
NCB_IRR_METHOD(getScreenSize);
NCB_IRR_METHOD2(getTexture, ITexture*, getTexture, (const c8*));
NCB_IRR_METHOD(getTextureByIndex);
NCB_IRR_METHOD(getTextureCount);
NCB_IRR_METHOD(getTransform);
NCB_IRR_METHOD2(makeColorKeyTexture, void, makeColorKeyTexture, (video::ITexture* texture, video::SColor color) const);
NCB_IRR_METHOD2(makeColorKeyTexture2, void, makeColorKeyTexture, (video::ITexture* texture, core::position2d<s32> colorKeyPixelPos) const);
NCB_IRR_METHOD(makeNormalMapTexture);
NCB_IRR_METHOD(queryFeature);
NCB_IRR_METHOD(removeAllTextures);
NCB_IRR_METHOD(removeTexture);
NCB_IRR_METHOD(renameTexture);
NCB_IRR_METHOD(setClipPlane);
NCB_IRR_METHOD(setFog);
NCB_IRR_METHOD(setMaterial);
NCB_IRR_METHOD(setMaterialRendererName);
NCB_IRR_METHOD(setRenderTarget);
NCB_IRR_METHOD(setTextureCreationFlag);
NCB_IRR_METHOD(setTransform);
NCB_IRR_METHOD(writeImageToFile);

//使わない
//NCB_IRR_METHOD2(createImageFromData);
//NCB_IRR_METHOD(addExternalImageLoader);
//NCB_IRR_METHOD(addExternalImageWriter);
//NCB_IRR_METHOD(addMaterialRenderer);
//NCB_IRR_METHOD(getExposedVideoData);
//NCB_IRR_METHOD(getGPUProgrammingServices);
//NCB_IRR_METHOD(getMaterialRenderer);
//NCB_IRR_METHOD(getMaterialRendererCount);
//NCB_IRR_METHOD(getMaterialRendererName);
};

static bool ISceneManagerLoadScene(IrrWrapper<ISceneManager> *obj, const char *filename)
{
	return obj->getIrrObject()->loadScene(filename);
}

static bool ISceneManagerSaveScene(IrrWrapper<ISceneManager> *obj, const char *filename)
{
	return obj->getIrrObject()->saveScene(filename);
}

static bool ISceneManagerLoadSceneFile(IrrWrapper<ISceneManager> *obj, IReadFile *file)
{
	return obj->getIrrObject()->loadScene(file);
}

static bool ISceneManagerSaveSceneFile(IrrWrapper<ISceneManager> *obj, IWriteFile *file)
{
	return obj->getIrrObject()->saveScene(file);
}

NCB_REGISTER_IRR_SUBCLASS(ISceneManager)
NCB_CONSTRUCTOR(());
NCB_IRR_PROPERTY2(activeCamera, getActiveCamera, setActiveCamera);
NCB_IRR_PROPERTY2(ambientLight, getAmbientLight, setAmbientLight);
NCB_IRR_PROPERTY2(shadowColor, getShadowColor, setShadowColor);
NCB_IRR_METHOD(addAnimatedMeshSceneNode);
NCB_IRR_METHOD(addArrowMesh);
NCB_IRR_METHOD(addBillboardSceneNode);
NCB_IRR_METHOD(addBillboardTextSceneNode);
NCB_IRR_METHOD(addCameraSceneNode);
//NCB_IRR_METHOD(addCameraSceneNodeFPS); XXX SKeyMap の配列が渡されてる
NCB_IRR_METHOD(addCameraSceneNodeMaya);
NCB_IRR_METHOD(addCubeSceneNode);
NCB_IRR_METHOD(addDummyTransformationSceneNode);
NCB_IRR_METHOD(addEmptySceneNode);
NCB_IRR_METHOD(addHillPlaneMesh);
NCB_IRR_METHOD(addLightSceneNode);
NCB_IRR_METHOD(addMeshSceneNode);
NCB_IRR_METHOD2(addOctTreeSceneNode,  ISceneNode*, addOctTreeSceneNode, (IAnimatedMesh *, ISceneNode *, s32, s32, bool));
NCB_IRR_METHOD2(addOctTreeSceneNode2, ISceneNode*, addOctTreeSceneNode, (IMesh *, ISceneNode *, s32, s32, bool));
NCB_IRR_METHOD(addParticleSystemSceneNode);
NCB_IRR_METHOD(addQuake3SceneNode);
NCB_IRR_METHOD(addSceneNode);
NCB_IRR_METHOD(addSkyDomeSceneNode);
NCB_IRR_METHOD(addSphereMesh);
NCB_IRR_METHOD(addSphereSceneNode);
NCB_IRR_METHOD(addTerrainMesh);
NCB_IRR_METHOD2(addTerrainSceneNode, ITerrainSceneNode*, addTerrainSceneNode, (const c8*, ISceneNode*, s32,
																			   const vector3df&, const vector3df&, const vector3df&, 
																			   SColor, s32, E_TERRAIN_PATCH_SIZE, s32, bool));
NCB_IRR_METHOD(addTextSceneNode);
NCB_IRR_METHOD(addToDeletionQueue);
NCB_IRR_METHOD(addWaterSurfaceSceneNode);
NCB_IRR_METHOD(clear);
NCB_IRR_CREATE(createCollisionResponseAnimator, ISceneNodeAnimatorCollisionResponse, (IrrWrapper<IrrClass> *obj, ITriangleSelector *world, ISceneNode *sceneNode, const core::vector3df &ellipsoidRadius, const core::vector3df &gravityPerSecond, const core::vector3df &ellipsoidTranslation, f32 slidingValue),
			   (world, sceneNode, ellipsoidRadius, gravityPerSecond, ellipsoidTranslation, slidingValue));
NCB_IRR_CREATE(createDeleteAnimator, ISceneNodeAnimator, (IrrWrapper<IrrClass> *obj, u32 timeMs), (timeMs));
NCB_IRR_CREATE(createFlyCircleAnimator, ISceneNodeAnimator, (IrrWrapper<IrrClass> *obj, const core::vector3df &center, f32 radius, f32 speed, const core::vector3df &direction), (center, radius, speed, direction));
NCB_IRR_CREATE(createFlyStraightAnimator, ISceneNodeAnimator,
			   (IrrWrapper<IrrClass> *obj, const core::vector3df &startPoint, const core::vector3df &endPoint, u32 timeForWay, bool loop), (startPoint, endPoint, timeForWay, loop));
//NCB_IRR_CREATE(createFollowSplineAnimator); XXX 配列が渡されてる
NCB_IRR_CREATE(createMeshWriter, IMeshWriter, (IrrWrapper<IrrClass> *obj, EMESH_WRITER_TYPE type), (type));
//virtual IMetaTriangleSelector *  createMetaTriangleSelector ()=0 
NCB_IRR_CREATE(createNewSceneManager, ISceneManager, (IrrWrapper<IrrClass> *obj, bool cloneContent), (cloneContent));
NCB_IRR_CREATE(createOctTreeTriangleSelector, ITriangleSelector, (IrrWrapper<IrrClass> *obj, IMesh *mesh, ISceneNode *node, s32 minimalPolysPerNode=32), (mesh, node, minimalPolysPerNode));
NCB_IRR_CREATE(createRotationAnimator, ISceneNodeAnimator, (IrrWrapper<IrrClass> *obj, const core::vector3df &rotationPerSecond), (rotationPerSecond));
NCB_IRR_CREATE(createTerrainTriangleSelector, ITriangleSelector, (IrrWrapper<IrrClass> *obj, ITerrainSceneNode *node, s32 LOD), (node, LOD));
//NCB_IRR_CREATE(createTextureAnimator); XXX 配列が渡されてる
NCB_IRR_CREATE(createTriangleSelector, ITriangleSelector, (IrrWrapper<IrrClass> *obj, IMesh *mesh, ISceneNode *node), (mesh, node));
NCB_IRR_CREATE(createTriangleSelectorFromBoundingBox, ITriangleSelector, (IrrWrapper<IrrClass> *obj, ISceneNode *node), (node));
NCB_IRR_METHOD(drawAll);
NCB_IRR_METHOD(getDefaultSceneNodeFactory);
NCB_IRR_METHOD(getGUIEnvironment);
NCB_IRR_METHOD2(getMesh, IAnimatedMesh*, getMesh, (const c8 *));
NCB_IRR_METHOD(getMeshCache);
NCB_IRR_METHOD(getMeshManipulator);
NCB_IRR_METHOD(getParameters);
NCB_IRR_METHOD(getRootSceneNode);
NCB_IRR_METHOD(getSceneCollisionManager);
NCB_IRR_METHOD(getSceneNodeAnimatorFactory);
NCB_IRR_METHOD(getSceneNodeFactory);
NCB_IRR_METHOD(getSceneNodeFromId);
NCB_IRR_METHOD(getSceneNodeFromName);
NCB_IRR_METHOD(getSceneNodeFromType);
NCB_IRR_METHOD(getSceneNodeTypeName);
NCB_IRR_METHOD(getVideoDriver);
NCB_IRR_METHOD(postEventFromUser);
NCB_IRR_METHOD(registerNodeForRendering);
NCB_METHOD_PROXY(loadScene, ISceneManagerLoadScene);
NCB_METHOD_PROXY(saveScene, ISceneManagerLoadScene);
NCB_METHOD_PROXY(loadSceneFile, ISceneManagerLoadSceneFile);
NCB_METHOD_PROXY(saveSceneFile, ISceneManagerSaveSceneFile);

//使わない
//NCB_IRR_METHOD(addExternalMeshLoader);
//NCB_METHOD(registerSceneAnimatorFactory);
//NCB_METHOD(registerSceneFactory);
//NCB_IRR_METHOD(getRegisteredSceneNodeAnimatorFactoryCount);
//NCB_IRR_METHOD(getRegisteredSceneNodeFactoryCount);
};

NCB_REGISTER_IRR_SUBCLASS(IGUIEnvironment)
NCB_CONSTRUCTOR(());
};

static tTJSVariant IFileSystemCreateAndWriteFile(IrrWrapper<IFileSystem> *fsystem, const tjs_char *filename, bool append)
{
}

NCB_REGISTER_IRR_SUBCLASS(IFileSystem)
NCB_CONSTRUCTOR(());
NCB_IRR_METHOD(addFolderFileArchive);
NCB_IRR_METHOD(addPakFileArchive);
NCB_IRR_METHOD(addZipFileArchive);
NCB_IRR_METHOD(changeWorkingDirectoryTo);
NCB_IRR_CREATE(createAndOpenFile, IReadFile, (IrrWrapper<IrrClass> *obj, const c8 *filename), (filename));
NCB_IRR_CREATE(createAndWriteFile, IWriteFile, (IrrWrapper<IrrClass> *obj, const c8 *filename, bool append), (filename, append));
NCB_IRR_CREATE(createEmptyAttributes, IAttributes, (IrrWrapper<IrrClass> *obj, video::IVideoDriver *driver), (driver));
NCB_IRR_CREATE(createFileList, IFileList, (IrrWrapper<IrrClass> *obj), ());
//NCB_IRR_CREATE(createMemoryReadFile);
//NCB_IRR_CREATE(createXMLReader);
//NCB_IRR_CREATE(createXMLReader);
//NCB_IRR_CREATE(createXMLReaderUTF8);
//NCB_IRR_CREATE(createXMLReaderUTF8);
//NCB_IRR_CREATE(createXMLWriter);
//NCB_IRR_CREATE(createXMLWriter);
NCB_IRR_METHOD(existFile);
// virtual core::stringc  getAbsolutePath (const core::stringc &filename) const =0 
// virtual core::stringc  getFileDir (const core::stringc &filename) const =0 
NCB_IRR_METHOD(getWorkingDirectory);
};

NCB_REGISTER_IRR_SUBCLASS(ILogger)
NCB_CONSTRUCTOR(());
NCB_IRR_PROPERTY2(logLevel, getLogLevel, setLogLevel);
NCB_IRR_METHOD2(log, void, log, (const wchar_t*,ELOG_LEVEL));
NCB_IRR_METHOD2(logHint, void, log, (const wchar_t*,const wchar_t*hint,ELOG_LEVEL));
};

// --------------------------------------------------------------------
// Irrlicht 操作用の基本オブジェクトの登録
// --------------------------------------------------------------------

#define BASE_METHOD \
	NCB_PROPERTY(eventMask, getEventMask, setEventMask);\
	NCB_PROPERTY_RO2(videoDriver, getVideoDriver);\
	NCB_PROPERTY_RO2(sceneManager, getSceneManager);\
	NCB_PROPERTY_RO2(guiEnvironment, getGUIEnvironment);\
	NCB_PROPERTY_RO2(logger, getLogger);\
	NCB_PROPERTY_RO2(fileSystem, getFileSystem)

NCB_REGISTER_SUBCLASS(IrrlichtDrawDevice) {
	Factory(&Class::Factory);
	BASE_METHOD;
	NCB_PROPERTY_RO(interface, getDevice);
	NCB_PROPERTY2(zoomMode, getZoomMode, setZoomMode);
	NCB_PROPERTY2(width, getWidth, setWidth);
	NCB_PROPERTY2(height, getHeight, setHeight);
	NCB_METHOD(setSize);
	NCB_PROPERTY_RO2(destWidth, getDestWidth);
	NCB_PROPERTY_RO2(destHeight, getDestHeight);
	NCB_PROPERTY2(defaultVisible, getDefaultVisible, setDefaultVisible);
	NCB_METHOD(getVisible);
	NCB_METHOD(setVisible);
};

NCB_REGISTER_SUBCLASS(IrrlichtWindow) {
	Factory(&Class::Factory);
	BASE_METHOD;
	NCB_PROPERTY2(left, getLeft, setLeft);
	NCB_PROPERTY2(top, getTop, setTop);
	NCB_PROPERTY2(width, getWidth, setWidth);
	NCB_PROPERTY2(height, getHeight, setHeight);
	NCB_PROPERTY2(visible, getVisible, setVisible);
	NCB_METHOD(setPos);
	NCB_METHOD(setSize);
	NCB_MEMBER_PROPERTY(transparentEvent, bool, transparentEvent);
};

NCB_REGISTER_SUBCLASS(IrrlichtSimpleDevice) {
	Factory(&Class::Factory);
	BASE_METHOD;
	NCB_PROPERTY2(width, getWidth, setWidth);
	NCB_PROPERTY2(height, getHeight, setHeight);
	NCB_METHOD(setSize);
	RawCallback(L"updateToLayer", &IrrlichtSimpleDevice::updateToLayer, 0);
};


/**
 * 名前空間用ダミー
 */
struct Irrlicht {
};

#define ENUM(n) Variant(#n, (int)n)
#define NCB_SUBCLASS_NAME(name) NCB_SUBCLASS(name, name)
#define NCB_IRR_SUBCLASS(Class) NCB_SUBCLASS(Class, IrrWrapper<Class>)

NCB_REGISTER_CLASS(Irrlicht) {

	ENUM(ELL_INFORMATION);
	ENUM(ELL_WARNING);
	ENUM(ELL_ERROR);
	ENUM(ELL_NONE);

	// EMESH_WRITER_TYPE
	ENUM(EMWT_IRR_MESH);
	ENUM(EMWT_COLLADA);
	ENUM(EMWT_STL);
	
	//static

	// Irrlicht データ型クラス
	NCB_SUBCLASS_NAME(SColor);
	NCB_SUBCLASS_NAME(SColorf);
	NCB_SUBCLASS_NAME(position2di);
	NCB_SUBCLASS_NAME(position2df);
	NCB_SUBCLASS_NAME(dimension2di);
	NCB_SUBCLASS_NAME(dimension2df);
	NCB_SUBCLASS_NAME(vector2df);
	NCB_SUBCLASS_NAME(vector3df);
	NCB_SUBCLASS(recti, rect<s32>);
	NCB_SUBCLASS(matrix4, matrix4);
	NCB_SUBCLASS_NAME(triangle3df);
	NCB_SUBCLASS_NAME(plane3df);
	NCB_SUBCLASS_NAME(aabbox3df);
	NCB_SUBCLASS_NAME(SLight);
	NCB_SUBCLASS_NAME(SMaterial);
	NCB_SUBCLASS(SShader, quake3::SShader);
	NCB_SUBCLASS_NAME(SAttributeReadWriteOptions);
	NCB_SUBCLASS_NAME(SEvent);
	NCB_SUBCLASS_NAME(SViewFrustum);

	// Irrlicht 参照用クラス
	NCB_IRR_SUBCLASS(IFileSystem);
	NCB_IRR_SUBCLASS(IReadFile);
	NCB_IRR_SUBCLASS(IWriteFile);
	NCB_IRR_SUBCLASS(IFileList);
	NCB_IRR_SUBCLASS(IAttributes);
	NCB_IRR_SUBCLASS(ISceneNodeAnimator);
	NCB_IRR_SUBCLASS(ISceneNodeAnimatorCollisionResponse);
	NCB_IRR_SUBCLASS(IMesh);
	NCB_IRR_SUBCLASS(IMeshBuffer);
	NCB_IRR_SUBCLASS(IMeshCache);
	NCB_IRR_SUBCLASS(IMeshManipulator);
	NCB_IRR_SUBCLASS(IMeshWriter);
	NCB_IRR_SUBCLASS(IAnimatedMesh);
	NCB_IRR_SUBCLASS(IGUIFont);
	NCB_IRR_SUBCLASS(ITriangleSelector);
	NCB_IRR_SUBCLASS(ISceneNodeAnimatorFactory);
	NCB_IRR_SUBCLASS(ISceneNodeFactory);
	NCB_IRR_SUBCLASS(ISceneCollisionManager);
	NCB_IRR_SUBCLASS(IAnimationEndCallBack);

	NCB_IRR_SUBCLASS(ISceneNode);
	NCB_IRR_SUBCLASS(IAnimatedMeshSceneNode);
	NCB_IRR_SUBCLASS(IBillboardSceneNode);
	NCB_IRR_SUBCLASS(IBoneSceneNode);
	NCB_IRR_SUBCLASS(ICameraSceneNode);
	NCB_IRR_SUBCLASS(IDummyTransformationSceneNode);
	NCB_IRR_SUBCLASS(ILightSceneNode);
	NCB_IRR_SUBCLASS(IMeshSceneNode);
	NCB_IRR_SUBCLASS(IParticleSystemSceneNode);
	NCB_IRR_SUBCLASS(ITerrainSceneNode);
	NCB_IRR_SUBCLASS(ITextSceneNode);
	NCB_IRR_SUBCLASS(IShadowVolumeSceneNode);
	NCB_IRR_SUBCLASS(IImage);
	NCB_IRR_SUBCLASS(ITexture);

	// Irrlicht 操作用クラス
	NCB_IRR_SUBCLASS(IVideoDriver);
	NCB_IRR_SUBCLASS(ISceneManager);
	NCB_IRR_SUBCLASS(IGUIEnvironment);
	NCB_IRR_SUBCLASS(ILogger);

	// Irrlicht ベースクラス
	NCB_SUBCLASS(SimpleDevice, IrrlichtSimpleDevice);
	NCB_SUBCLASS(DrawDevice, IrrlichtDrawDevice);
	NCB_SUBCLASS(Window, IrrlichtWindow);

	// イベントマスク用

#define EMASK(n)	Variant(#n, (int)IrrlichtBase::n)

	EMASK(EMASK_ATTACH);
	EMASK(EMASK_DETACH);
	EMASK(EMASK_EVENT);
	EMASK(EMASK_BEFORE_SCENE);
	EMASK(EMASK_AFTER_SCENE);
	EMASK(EMASK_BEFORE_GUI);
	EMASK(EMASK_AFTER_GUI);
}

/**
 * レイヤに IImage からの吸い出し処理を拡張
 */
static tjs_error TJS_INTF_METHOD
copyIImage(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	typedef IrrWrapper<IImage> WrapperT;
	typedef ncbInstanceAdaptor<WrapperT> AdaptorT;
	WrapperT *obj;
	IImage *image;
	if (param[0]->Type() != tvtObject ||
		(obj = AdaptorT::GetNativeInstance(param[0]->AsObjectNoAddRef())) == NULL ||
		(image = obj->getIrrObject()) == NULL) {
		TVPThrowExceptionMessage(TJS_W("src must be IImage."));
	}
	
	dimension2d<s32> size = image->getDimension();
	int width  = size.Width;
	int height = size.Height;

	// レイヤのサイズを調整
	ncbPropAccessor layer(objthis);
	layer.SetValue(L"width",  width);
	layer.SetValue(L"height", height);
	layer.SetValue(L"imageLeft",  0);
	layer.SetValue(L"imageTop",   0);
	layer.SetValue(L"imageWidth",  width);
	layer.SetValue(L"imageHeight", height);
	
	unsigned char *buffer = (unsigned char*)layer.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
	int pitch = layer.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
	
	// 画像データのコピー
	if (image->getColorFormat() == ECF_A8R8G8B8) {
		unsigned char *src = (unsigned char*)image->lock();
		int slen   = width * 4;
		int spitch = image->getPitch();
		for (int y=0;y<height;y++) {
			memcpy(buffer, src, slen);
			src    += spitch;
			buffer += pitch;
		}
		image->unlock();
	} else {
		for (int y=0;y<height;y++) {
			u32 *line = (u32 *)buffer;
			for (int x=0;x<width;x++) {
				*line++ = image->getPixel(x, y).color;
				buffer += pitch;
			}
		}
	}

	// レイヤを更新
	objthis->FuncCall(0, L"update", NULL, NULL, 0, NULL, objthis);

	return TJS_S_OK;
}

/**
 * レイヤに ITexture からの吸い出し処理を拡張
 * @param ただし ARGB に限る
 */
static tjs_error TJS_INTF_METHOD
copyITexture(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	
	typedef IrrWrapper<ITexture> WrapperT;
	typedef ncbInstanceAdaptor<WrapperT> AdaptorT;
	WrapperT *obj;
	ITexture *texture;
	if (param[0]->Type() != tvtObject ||
		(obj = AdaptorT::GetNativeInstance(param[0]->AsObjectNoAddRef())) == NULL ||
		(texture = obj->getIrrObject()) == NULL) {
		TVPThrowExceptionMessage(TJS_W("src must be ITexture."));
	}
	
	dimension2d<s32> size = texture->getSize();
	int width  = size.Width;
	int height = size.Height;
	
	// レイヤのサイズを調整
	ncbPropAccessor layer(objthis);
	layer.SetValue(L"width",  width);
	layer.SetValue(L"height", height);
	layer.SetValue(L"imageLeft",  0);
	layer.SetValue(L"imageTop",   0);
	layer.SetValue(L"imageWidth",  width);
	layer.SetValue(L"imageHeight", height);
	
	unsigned char *buffer = (unsigned char*)layer.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
	int pitch = layer.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
	
	// 画像データのコピー
	if (texture->getColorFormat() == ECF_A8R8G8B8) {
		unsigned char *src = (unsigned char*)texture->lock();
		int slen   = width * 4;
		int spitch = texture->getPitch();
		for (int y=0;y<height;y++) {
			memcpy(buffer, src, slen);
			src    += spitch;
			buffer += pitch;
		}
		texture->unlock();

		// レイヤを更新
		objthis->FuncCall(0, L"update", NULL, NULL, 0, NULL, objthis);

	} else {
		TVPThrowExceptionMessage(TJS_W("invalid format texture."));
	}
	return TJS_S_OK;
}


NCB_ATTACH_FUNCTION(copyIImage, Layer, copyIImage);
NCB_ATTACH_FUNCTION(copyITexture, Layer, copyIImage);

/**
 * 登録処理前
 */
void PreRegistCallback()
{
	registerWindowClass();
}

/**
 * 開放処理後
 */
void PostUnregistCallback()
{
	unregisterWindowClass();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
