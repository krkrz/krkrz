#include "magickpp.hpp"
#include "arconv.hpp"

NCB_TYPECONV_ARRAY(double*);

using namespace Magick;
struct DrawableFactory {
	static Drawable* Point(double x_, double y_)				{ return new Drawable(DrawablePoint(x_, y_)); }
	static Drawable* PointSize(double pointSize_)				{ return new Drawable(DrawablePointSize(pointSize_)); }
//	static Drawable* Polygon(const CoordinateList &coordinates_);
//	static Drawable* Polyline(const CoordinateList &coordinates_);
	static Drawable* PopGraphicContext()						{ return new Drawable(DrawablePopGraphicContext()); }
	static Drawable* PushGraphicContext()						{ return new Drawable(DrawablePushGraphicContext()); }
	static Drawable* PopPattern()								{ return new Drawable(DrawablePopPattern()); }
	static Drawable* PushPattern(const StringT &id_, long x_, long y_, long width_, long height_)
		/**/													{ return new Drawable(DrawablePushPattern(id_, x_, y_, width_, height_)); }
	static Drawable* Rectangle(double upperLeftX_, double upperLeftY_, double lowerRightX_, double lowerRightY_)
		/**/													{ return new Drawable(DrawableRectangle(upperLeftX_, upperLeftY_, lowerRightX_, lowerRightY_)); }
	static Drawable* Rotation(double angle_)					{ return new Drawable(DrawableRotation(angle_)); }
	static Drawable* RoundRectangle(double centerX_, double centerY_, double width_, double hight_, double cornerWidth_, double cornerHeight_)
		/**/													{ return new Drawable(DrawableRoundRectangle(centerX_, centerY_, width_, hight_, cornerWidth_, cornerHeight_)); }
	static Drawable* Scaling(double x_, double y_)				{ return new Drawable(DrawableScaling(x_, y_)); }
	static Drawable* SkewX(double angle_)						{ return new Drawable(DrawableSkewX(angle_)); }
	static Drawable* SkewY(double angle_)						{ return new Drawable(DrawableSkewY(angle_)); }
	static Drawable* DashArray(/*const*/ double* dasharray_)	{ return new Drawable(DrawableDashArray(dasharray_)); }
	static Drawable* DashOffset(double offset_)					{ return new Drawable(DrawableDashOffset(offset_)); }
	static Drawable* StrokeLineCap(LineCap linecap_)			{ return new Drawable(DrawableStrokeLineCap(linecap_)); }
	static Drawable* StrokeLineJoin(LineJoin linejoin_)			{ return new Drawable(DrawableStrokeLineJoin(linejoin_)); }
	static Drawable* MiterLimit(unsigned int miterlimit_)		{ return new Drawable(DrawableMiterLimit(miterlimit_)); }
	static Drawable* StrokeAntialias(bool flag_)				{ return new Drawable(DrawableStrokeAntialias(flag_)); }
	static Drawable* StrokeColor(const Color &color_)			{ return new Drawable(DrawableStrokeColor(color_)); }
	static Drawable* StrokeOpacity(double opacity_)				{ return new Drawable(DrawableStrokeOpacity(opacity_)); }
	static Drawable* StrokeWidth(double width_)					{ return new Drawable(DrawableStrokeWidth(width_)); }
//	static Drawable* Text(double x_, double y_, const StringT &text_);
//	static Drawable* Text(double x_, double y_, const StringT &text_, const StringT &encoding_);
	static Drawable* TextAntialias(bool flag_)					{ return new Drawable(DrawableTextAntialias(flag_)); }
	static Drawable* TextDecoration(DecorationType decoration_)	{ return new Drawable(DrawableTextDecoration(decoration_)); }
	static Drawable* TextUnderColor(const Color &color_)		{ return new Drawable(DrawableTextUnderColor(color_)); }
	static Drawable* Translation(double x_, double y_)			{ return new Drawable(DrawableTranslation(x_, y_)); }
	static Drawable* Viewbox(unsigned long x1_, unsigned long y1_, unsigned long x2_, unsigned long y2_)
		/**/													{ return new Drawable(DrawableViewbox(x1_, y1_, x2_, y2_)); }
};

// Drawable
MAGICK_SUBCLASS(Drawable) {
#define FACTORY_METHOD(m) Method(TJS_W(# m), &DrawableFactory::m)
	FACTORY_METHOD(Point);
	FACTORY_METHOD(PointSize);
//	FACTORY_METHOD(Polygon);
//	FACTORY_METHOD(Polyline);
	FACTORY_METHOD(PopGraphicContext);
	FACTORY_METHOD(PushGraphicContext);
	FACTORY_METHOD(PopPattern);
	FACTORY_METHOD(PushPattern);
	FACTORY_METHOD(Rectangle);
	FACTORY_METHOD(Rotation);
	FACTORY_METHOD(RoundRectangle);
	FACTORY_METHOD(Scaling);
	FACTORY_METHOD(SkewX);
	FACTORY_METHOD(SkewY);
	FACTORY_METHOD(DashArray);
	FACTORY_METHOD(DashOffset);
	FACTORY_METHOD(StrokeLineCap);
	FACTORY_METHOD(StrokeLineJoin);
	FACTORY_METHOD(MiterLimit);
	FACTORY_METHOD(StrokeAntialias);
	FACTORY_METHOD(StrokeColor);
	FACTORY_METHOD(StrokeOpacity);
	FACTORY_METHOD(StrokeWidth);
//	FACTORY_METHOD(Text);
	FACTORY_METHOD(TextAntialias);
	FACTORY_METHOD(TextDecoration);
	FACTORY_METHOD(TextUnderColor);
	FACTORY_METHOD(Translation);
	FACTORY_METHOD(Viewbox);
}
