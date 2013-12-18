#include <list>
#include <string>

#include "magickpp.hpp"

struct MagickPP {
	template <typename T> struct Container { typedef std::list<T> Type; };

	typedef char const* NameT;

	typedef Magick::CoderInfo CoderInfoT;
	typedef Magick::Image     ImageT;

	typedef Container<CoderInfoT>::Type CoderListT;
	typedef Container<ImageT>::Type     ImageListT;

	typedef ncbInstanceAdaptor<CoderInfoT>       CoderAdaptorT;
	typedef ncbInstanceAdaptor<ImageT>           ImageAdaptorT;


	// イメージ(群)読み込み
	static iTJSDispatch2* readImages(StringT const &spec) {

		ImageListT lst;
		Magick::readImages(&lst, spec);

		// Array オブジェクトを作成
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_uint32 hint = 0;

		for (ImageListT::iterator it = lst.begin(); it != lst.end(); it++) {
			ImageT *img = new ImageT(*it);
			iTJSDispatch2 *adp = ImageAdaptorT::CreateAdaptor(img, false);
			tTJSVariant var(adp), *param = &var;
			adp->Release();
			array->FuncCall(0, TJS_W("add"), &hint, 0, 1, &param, array);
		}
		return array;
	}

	// Corder一覧取得
	static iTJSDispatch2* supports() {

		CoderListT cl; 
		Magick::coderInfoList(&cl,                   // Reference to output list 
							  CoderInfoT::AnyMatch,  // readable formats
							  CoderInfoT::AnyMatch,  // writable formats
							  CoderInfoT::AnyMatch); // multi-frame support

		// Array オブジェクトを作成
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_uint32 hint = 0;

		ttstr exp(TJS_W("new MagickPP.CoderInfo('"));
		for (CoderListT::const_iterator it = cl.begin(); it != cl.end(); it++) {
			tTJSVariant var, *param = &var;
			TVPExecuteExpression(exp + ttstr(it->name().c_str()) + TJS_W("')"), &var);
			array->FuncCall(0, TJS_W("add"), &hint, 0, 1, &param, array);
		}

		return array;
	}

	// ImageMagick バージョン取得
	static NameT version() { return MagickVersion; }


};


NCB_REGISTER_CLASS(MagickPP)
{
	// magick++ classes
	SUBCLASS(Blob);
	SUBCLASS(CoderInfo);
	SUBCLASS(Color);
	SUBCLASS(Drawable);
	SUBCLASS(Geometry);
	SUBCLASS(Image);
	SUBCLASS(TypeMetric);

	// enums
	SUBCLASS(ClassType);
	SUBCLASS(ChannelType);
	SUBCLASS(ColorspaceType);
	SUBCLASS(CompositeOperator);
	SUBCLASS(CompressionType);
	SUBCLASS(DisposeType);
	SUBCLASS(EndianType);
	SUBCLASS(MagickEvaluateOperator);
	SUBCLASS(FillRule);
	SUBCLASS(FilterTypes);
	SUBCLASS(GravityType);
	SUBCLASS(ImageType);
	SUBCLASS(InterlaceType);
	SUBCLASS(LineCap);
	SUBCLASS(LineJoin);
	SUBCLASS(NoiseType);
	SUBCLASS(OrientationType);
	SUBCLASS(PaintMethod);
	SUBCLASS(QuantumType);
	SUBCLASS(RenderingIntent);
	SUBCLASS(ResolutionType);
	SUBCLASS(StorageType);
	SUBCLASS(StretchType);
	SUBCLASS(StyleType);
	SUBCLASS(DecorationType);

	// special
	PROP_RO(version);
	PROP_RO(supports);
	NCB_METHOD(readImages);
}

