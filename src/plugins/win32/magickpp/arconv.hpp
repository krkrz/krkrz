#ifndef _arrconv_hpp_
#define _arrconv_hpp_

#include "ncbind.hpp"


template <typename T> struct ToValue;
template <typename T> struct ToValue<T*> { typedef T Type; };

template <typename T>
struct ToArrayedPointer {
	typedef typename ToValue<T>::Type TargetT;
	typedef ncbPropAccessor<TargetT> PropAccT;
	typedef typename PropAccT::SizeT SizeT;

	ToArrayedPointer() : _array(0) {}
	~ToArrayedPointer() {
		if (_array) delete[] _array;
		_array = 0;
	}
	template <typename DST>
	inline void operator()(DST &dst, tTJSVariant const &src) {
		if (ncbTypedefs::GetVariantType(src) == tvtObject &&
			src.IsInstanceOf(TJS_W("Array"))) {
			PropAccT acc(src.AsObjectNoAddRef());
			SizeT sz = acc.GetCount();
			_array = new TargetT[sz + 1];
			for (SizeT i = 0; i < sz; i++) _array[i] = acc.GetValue(i);
			_array[sz] = 0; // terminator
		} else {
			TVPThrowExceptionMessage(TJS_W("Non Array Parameter"));
		}
		dst = static_cast<DST>(_array);
	}
private:
	TargetT *_array;
};


#define NCB_TYPECONV_ARRAY(type) \
	NCB_TYPECONV_DSTMAP_SET(type, ToArrayedPointer<type>, false)

#endif
