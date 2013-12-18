#ifndef _magickpp_hpp_
#define _magickpp_hpp_

#include <Magick++.h>
#include <magick/version.h>

#include "ncbind.hpp"

typedef std::string StringT;

// exception フック
template <>
struct ncbNativeClassMethodBase::invokeHookAll<true> {
	template <typename T>
	static inline typename T::ResultT Do(T &t) {
		try {
			return t();
		} catch (Magick::Exception &err) {
			ttstr what(TJS_W("Magick++ Exception: ")); what += ttstr(err.what());
			TVPThrowExceptionMessage(what.c_str());
			throw;
		} catch (std::exception &err) {
			ttstr what(TJS_W("STL Exception: ")); what += ttstr(err.what());
			TVPThrowExceptionMessage(what.c_str());
			throw;
		}
	}
};

// 引き数と返り値に std::string をサポートさせる
NCB_TYPECONV_STL_STRING(StringT);

// 引き数と返り値に enum をサポート（面倒なので数値変換）
#define MAGICK_INTEGER(e) using Magick::e; NCB_TYPECONV_CAST_INTEGER(e)
MAGICK_INTEGER(ChannelType); //##
MAGICK_INTEGER(ClassType); //##
MAGICK_INTEGER(ColorspaceType); //##
MAGICK_INTEGER(CompositeOperator); //##
MAGICK_INTEGER(CompressionType); //##
MAGICK_INTEGER(EndianType); //##
MAGICK_INTEGER(FillRule); //## 保留
MAGICK_INTEGER(FilterTypes); //##
MAGICK_INTEGER(ImageType); //##
MAGICK_INTEGER(InterlaceType); //##
MAGICK_INTEGER(LineCap); //##
MAGICK_INTEGER(LineJoin); //##
MAGICK_INTEGER(OrientationType); //##
MAGICK_INTEGER(PaintMethod); //##
MAGICK_INTEGER(RenderingIntent); //##
MAGICK_INTEGER(ResolutionType); //##

MAGICK_INTEGER(DecorationType);
MAGICK_INTEGER(DisposeType);
MAGICK_INTEGER(GravityType);
MAGICK_INTEGER(NoiseType);
MAGICK_INTEGER(StorageType);
MAGICK_INTEGER(StretchType);
MAGICK_INTEGER(StyleType);

MAGICK_INTEGER(QuantumType); // magick/quantum.h
MAGICK_INTEGER(MagickEvaluateOperator);
//MAGICK_INTEGER(ChannelType);  // magick/magick-type.h

#define MAGICK_OBJECT(obj) using Magick::obj; NCB_TYPECONV_BOXING(obj)
MAGICK_OBJECT(Blob);
MAGICK_OBJECT(CoderInfo);
MAGICK_OBJECT(Color);
MAGICK_OBJECT(Drawable);
MAGICK_OBJECT(Geometry);
MAGICK_OBJECT(Image);
MAGICK_OBJECT(TypeMetric);

#define MAGICK_SUBCLASS(cls) \
	NCB_REGISTER_SUBCLASS_DELAY(cls)

#define MAGICK_ENUM(e) \
	NCB_REGISTER_SUBCLASS_DELAY(e)

#define ENUM(n) struct prop ## n { static Class Get() { return n; } }; Property(TJS_W(# n), & prop ## n::Get, 0)


// 読み取り専用プロパティ短縮用
#define PROP_RO(prop) NCB_PROPERTY_RO(prop, prop)
#define PROP_WO(prop) NCB_PROPERTY_WO(prop, prop)
#define PROP_RW(type, prop) PROP_RW_TAG(prop, type, Const, type, Class)
#define PROP_rw(type, prop) PROP_RW_TAG(prop, type, Class, type, Class)

#define PROP_RW_TAG(prop, typeR, tagR, typeW, tagW) \
	NCB_PROPERTY_DETAIL(prop, \
						tagR, typeR, ClassT::prop, (), \
						tagW, void,  ClassT::prop, (typeW))
#define PROP_RW_TYPE(type, prop) \
	PROP_RW_TAG(prop, type, Const, type const&, Class)

#define PROP_BLOB(prop)     PROP_RW_TYPE(Blob,     prop)
#define PROP_COLOR(prop)    PROP_RW_TYPE(Color,    prop)
#define PROP_GEOMETRY(prop) PROP_RW_TYPE(Geometry, prop)
#define PROP_IMAGE(prop)    PROP_RW_TYPE(Image,    prop)
#define PROP_STRING(prop)   PROP_RW_TYPE(StringT,  prop)

#define SUBCLASS(cls) \
	NCB_SUBCLASS(cls, cls);

#endif
