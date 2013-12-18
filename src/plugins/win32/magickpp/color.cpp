#include "magickpp.hpp"

struct ColorProxy {
	typedef Magick::ColorRGB  ColorRGB;
	typedef Magick::ColorHSL  ColorHSL;
	typedef Magick::ColorYUV  ColorYUV;
	typedef Magick::ColorGray ColorGray;
	typedef Magick::ColorMono ColorMono;
	typedef Magick::Quantum   Quantum;

#define PROP_PROXY(type, base, tag, method) \
	static type Get ## tag(Color const *c)   { return base(*c).method(); } \
	static void Set ## tag(Color *c, type v) { base tmp(*c); tmp.method(v); *c = tmp; }


	PROP_PROXY(double, ColorRGB, R, red);
	PROP_PROXY(double, ColorRGB, G, green);
	PROP_PROXY(double, ColorRGB, B, blue);

	PROP_PROXY(double, ColorHSL, H, hue);
	PROP_PROXY(double, ColorHSL, S, saturation);
	PROP_PROXY(double, ColorHSL, L, luminosity);

	PROP_PROXY(double, ColorYUV, Y, y);
	PROP_PROXY(double, ColorYUV, U, u);
	PROP_PROXY(double, ColorYUV, V, v);

	PROP_PROXY(double, ColorGray, Gray, shade);
	PROP_PROXY(bool,   ColorMono, Mono, mono);

	static Color _QRGBA(Quantum r, Quantum g, Quantum b, Quantum a) { return Color(r, g, b, a); }
	static Color _QRGB( Quantum r, Quantum g, Quantum b)            { return Color(r, g, b); }
	static Color _RGBA(double r, double g, double b, double a) { ColorRGB c(r, g, b); c.alpha(a); return c; }

	static Color _RGB( double r, double g, double b) { return ColorRGB(r, g, b); }
	static Color _HSL( double h, double s, double l) { return ColorHSL(h, s, l); }
	static Color _YUV( double y, double u, double v) { return ColorYUV(y, u, v); }
	static Color _Gray(double s)                     { return ColorGray(s);      }
	static Color _Mono(bool   m)                     { return ColorMono(m);      }
};


// Color
MAGICK_SUBCLASS(Color) {
	NCB_CONSTRUCTOR(());

	// quick
	Method(TJS_W("QRGBA"), ColorProxy::_QRGBA);
	Method(TJS_W("QRGB" ), ColorProxy::_QRGB);
	Method(TJS_W("RGBA"),  ColorProxy::_RGBA);
	Method(TJS_W("RGB"),   ColorProxy::_RGB);
	Method(TJS_W("HSL"),   ColorProxy::_HSL);
	Method(TJS_W("YUV"),   ColorProxy::_YUV);
	Method(TJS_W("GRAY"),  ColorProxy::_Gray);
	Method(TJS_W("MONO"),  ColorProxy::_Mono);

	typedef Magick::Quantum Quantum;
	PROP_RW(Quantum, redQuantum);
	PROP_RW(Quantum, greenQuantum);
	PROP_RW(Quantum, blueQuantum);
	PROP_RW(Quantum, alphaQuantum);
	PROP_RW(double,  alpha);
	PROP_RW(bool, isValid);
//	PROP_RO(intensity);

	NCB_PROPERTY_DETAIL(string,
						Const, StringT, Class::operator StringT, (),
						Class, Color const&, Class::operator =, (StringT const&)
						);
	NCB_METHOD(scaleDoubleToQuantum);
	NCB_METHOD_DETAIL(scaleQuantumToDouble, Static, double, Class::scaleQuantumToDouble, (double));

	NCB_PROPERTY_PROXY(red,        ColorProxy::GetR, ColorProxy::SetR);
	NCB_PROPERTY_PROXY(green,      ColorProxy::GetG, ColorProxy::SetG);
	NCB_PROPERTY_PROXY(blue,       ColorProxy::GetB, ColorProxy::SetB);

	NCB_PROPERTY_PROXY(hue,        ColorProxy::GetH, ColorProxy::SetH);
	NCB_PROPERTY_PROXY(saturation, ColorProxy::GetS, ColorProxy::SetS);
	NCB_PROPERTY_PROXY(luminosity, ColorProxy::GetL, ColorProxy::SetL);

	NCB_PROPERTY_PROXY(y,          ColorProxy::GetY, ColorProxy::SetY);
	NCB_PROPERTY_PROXY(u,          ColorProxy::GetU, ColorProxy::SetU);
	NCB_PROPERTY_PROXY(v,          ColorProxy::GetV, ColorProxy::SetV);

	NCB_PROPERTY_PROXY(shade,      ColorProxy::GetGray, ColorProxy::SetGray);
	NCB_PROPERTY_PROXY(mono,       ColorProxy::GetMono, ColorProxy::SetMono);
}
