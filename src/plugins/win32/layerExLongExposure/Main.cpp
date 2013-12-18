#include <windows.h>
#include "ncbind.hpp"

struct   LongExposure {
	/**/~LongExposure() { term(); }
	/**/ LongExposure(iTJSDispatch2 *obj) : self(obj), buffer(0), width(0), height(0) {}

	void init() {
		term();
		if (!GetLayerSize(self, width, height))
			TVPThrowExceptionMessage(TJS_W("LongExposure.init: invalid layer image"));
		size_t len = width * height * 4;
		buffer = new DWORD[len];
		ZeroMemory(buffer, sizeof(DWORD)*len);
	}

	void snap() {
		if (!buffer)
			TVPThrowExceptionMessage(TJS_W("LongExposure.snap: not initialized"));
		size_t curw = 0, curh = 0;
		if (!GetLayerSize(self, curw, curh))
			TVPThrowExceptionMessage(TJS_W("LongExposure.snap: invalid layer image"));
		if (curw != width || curh != height)
			TVPThrowExceptionMessage(TJS_W("LongExposure.snap: invalid layer size"));

		const BYTE *ptr = 0;
		long pitch = 0;
		GetLayerImage(self, ptr, pitch);

		for (size_t y = 0; y < height; y++) {
			DWORD* w = buffer + width*4 * y;
			const BYTE* r = ptr + pitch * y;
			for (size_t x = 0; x < width; x++) {
				*w++ += (DWORD)(*r++);
				*w++ += (DWORD)(*r++);
				*w++ += (DWORD)(*r++);
				*w++ += (DWORD)(*r++);
			}
		}
	}

	struct MinMaxRGBA {
		struct UnitRGBA {
			UnitRGBA(DWORD n) : r(n), g(n), b(n), a(n) {}
			DWORD r, g, b, a;
		} _min, _max;
		MinMaxRGBA() : _min(0xFFFFFFFF), _max(0) {}
		inline void setMinMaxR(DWORD n) { setMinMax(n, _min.r, _max.r); }
		inline void setMinMaxG(DWORD n) { setMinMax(n, _min.g, _max.g); }
		inline void setMinMaxB(DWORD n) { setMinMax(n, _min.b, _max.b); }
		inline void setMinMaxA(DWORD n) { setMinMax(n, _min.a, _max.a); }
		inline BYTE getNormalizeR(DWORD n) { return getNormalize(n, _min.r, _max.r); }
		inline BYTE getNormalizeG(DWORD n) { return getNormalize(n, _min.g, _max.g); }
		inline BYTE getNormalizeB(DWORD n) { return getNormalize(n, _min.b, _max.b); }
		inline BYTE getNormalizeA(DWORD n) { return getNormalize(n, _min.a, _max.a); }
	private:
		inline void setMinMax(DWORD n, DWORD &min, DWORD &max) {
			if (n < min) min = n;
			if (n > max) max = n;
		}
		inline BYTE getNormalize(DWORD n, DWORD min, DWORD max) {
			if (min >= max) return 0xFF;
			if (n < min) n = min;
			if (n > max) n = max;
			return (BYTE)((n - min) * 255 / (max - min));
		}
	};
	bool _stat(MinMaxRGBA &m) {
		for (size_t y = 0; y < height; y++) {
			const DWORD* r = buffer + width*4 * y;
			for (size_t x = 0; x < width; x++) {
				m.setMinMaxB(*r++);
				m.setMinMaxG(*r++);
				m.setMinMaxR(*r++);
				m.setMinMaxA(*r++);
			}
		}
		return true;
	}
	tTJSVariant stat() {
		if (!buffer)
			TVPThrowExceptionMessage(TJS_W("LongExposure.stat: not initialized"));

		MinMaxRGBA m;
		if (!_stat(m))
			TVPThrowExceptionMessage(TJS_W("LongExposure.stat: invalid layer image"));

		ncbDictionaryAccessor rdic;
		tTJSVariant r;
		if (rdic.IsValid()) {
			rdic.SetValue(TJS_W("min_r"), (tTVInteger)m._min.r);
			rdic.SetValue(TJS_W("min_g"), (tTVInteger)m._min.g);
			rdic.SetValue(TJS_W("min_b"), (tTVInteger)m._min.b);
			rdic.SetValue(TJS_W("min_a"), (tTVInteger)m._min.a);
			rdic.SetValue(TJS_W("max_r"), (tTVInteger)m._max.r);
			rdic.SetValue(TJS_W("max_g"), (tTVInteger)m._max.g);
			rdic.SetValue(TJS_W("max_b"), (tTVInteger)m._max.b);
			rdic.SetValue(TJS_W("max_a"), (tTVInteger)m._max.a);
			r = tTJSVariant(rdic, rdic);
		}
		return r;
	}

	void copy(tTJSVariant v) {
		if (!buffer)
			TVPThrowExceptionMessage(TJS_W("LongExposure.copy: not initialized"));
		size_t curw = 0, curh = 0;
		if (!GetLayerSize(self, curw, curh))
			TVPThrowExceptionMessage(TJS_W("LongExposure.copy: invalid layer image"));
		if (curw != width || curh != height)
			TVPThrowExceptionMessage(TJS_W("LongExposure.copy: invalid layer size"));

		BYTE *ptr = 0;
		long pitch = 0;
		if (!GetLayerImageForWrite(self, ptr, pitch))
			TVPThrowExceptionMessage(TJS_W("LongExposure.copy: invalid layer image"));

		MinMaxRGBA m;
		switch (v.Type()) {
		case tvtVoid:
			if (!_stat(m))
				TVPThrowExceptionMessage(TJS_W("LongExposure.copy: stat failed"));
			break;
		case tvtObject:
			{
				ncbPropAccessor rdic(v);
				if (rdic.IsValid()) {
					m._min.r = rdic.getIntValue(TJS_W("min_r"));
					m._min.g = rdic.getIntValue(TJS_W("min_g"));
					m._min.b = rdic.getIntValue(TJS_W("min_b"));
					m._min.a = rdic.getIntValue(TJS_W("min_a"));
					m._max.r = rdic.getIntValue(TJS_W("max_r"));
					m._max.g = rdic.getIntValue(TJS_W("max_g"));
					m._max.b = rdic.getIntValue(TJS_W("max_b"));
					m._max.a = rdic.getIntValue(TJS_W("max_a"));
				}
			}
			break;
		}
		for (size_t y = 0; y < height; y++) {
			const DWORD* r = buffer + width*4 * y;
			BYTE* w = ptr + pitch * y;
			for (size_t x = 0; x < width; x++) {
				*w++ = m.getNormalizeB(*r++);
				*w++ = m.getNormalizeG(*r++);
				*w++ = m.getNormalizeR(*r++);
				*w++ = m.getNormalizeA(*r++);
			}
		}
	}

	void term() {
		if (buffer) delete[] buffer;
		buffer = 0;
		width = height = 0;
	}

private:
	iTJSDispatch2 *self;
	DWORD *buffer;
	size_t width, height;

	static iTJSDispatch2 *LayerClass;
	static bool GetLayerSize(iTJSDispatch2 *lay, size_t &w, size_t &h) {
		static ttstr hasImage   (TJS_W("hasImage"));
		static ttstr imageWidth (TJS_W("imageWidth"));
		static ttstr imageHeight(TJS_W("imageHeight"));

		tTVInteger lw, lh;
		if (!LayerPropGet(lay, hasImage) ||
			(lw = LayerPropGet(lay, imageWidth )) <= 0 ||
			(lh = LayerPropGet(lay, imageHeight)) <= 0) return false;
		w = (size_t)lw;
		h = (size_t)lh;
		return true;
	}
	static bool GetLayerImage(iTJSDispatch2 *lay, const BYTE* &ptr, long &pitch) {
		static ttstr mainImageBufferPitch(TJS_W("mainImageBufferPitch"));
		static ttstr mainImageBuffer(TJS_W("mainImageBuffer"));

		tTVInteger lpitch, lptr;
		if ((lpitch = LayerPropGet(lay, mainImageBufferPitch)) == 0 ||
			(lptr   = LayerPropGet(lay, mainImageBuffer)) == 0) return false;
		pitch = (long)lpitch;
		ptr = reinterpret_cast<const BYTE*>(lptr);
		return true;
	}
	static bool GetLayerImageForWrite(iTJSDispatch2 *lay, BYTE* &ptr, long &pitch) {
		static ttstr mainImageBufferPitch(TJS_W("mainImageBufferPitch"));
		static ttstr mainImageBufferForWrite(TJS_W("mainImageBufferForWrite"));

		tTVInteger lpitch, lptr;
		if ((lpitch = LayerPropGet(lay, mainImageBufferPitch)) == 0 ||
			(lptr   = LayerPropGet(lay, mainImageBufferForWrite)) == 0) return false;
		pitch = (long)lpitch;
		ptr = reinterpret_cast<BYTE*>(lptr);
		return true;
	}
	static tTVInteger LayerPropGet(iTJSDispatch2 *lay, ttstr &prop, tTVInteger defval = 0) {
		if (!LayerClass) {
			tTJSVariant var;
			TVPExecuteExpression(TJS_W("Layer"), &var);
			LayerClass = var.AsObjectNoAddRef();
		}
		tTJSVariant val;
		return (TJS_FAILED(LayerClass->PropGet(0, prop.c_str(), prop.GetHint(), &val, lay))) ? defval : val.AsInteger();
	}
};
iTJSDispatch2* LongExposure::LayerClass = 0;


NCB_GET_INSTANCE_HOOK(LongExposure)
{
	/**/  NCB_GET_INSTANCE_HOOK_CLASS () {}
	/**/ ~NCB_GET_INSTANCE_HOOK_CLASS () {}
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) SetNativeInstance(objthis, (obj = new ClassT(objthis)));
		return obj;
	}
};
NCB_ATTACH_CLASS_WITH_HOOK(LongExposure, Layer)
{
	Method(TJS_W("initExposure"), &Class::init);
	Method(TJS_W("snapExposure"), &Class::snap);
	Method(TJS_W("statExposure"), &Class::stat);
	Method(TJS_W("copyExposure"), &Class::copy);
	Method(TJS_W("termExposure"), &Class::term);
}
