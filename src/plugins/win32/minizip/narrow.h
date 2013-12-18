#ifndef __NARROW_STRING__
#define __NARROW_STRING__

#include <windows.h>

/**
 * C•¶Žš—ñˆ——p
 */
class NarrowString {
private:
	tjs_nchar *_data;
public:
	NarrowString(const ttstr &str, bool utf8=false) : _data(NULL) {
		if (utf8) {
			const tjs_char *n = str.c_str();
			int	len = ::WideCharToMultiByte(CP_UTF8, 0, n, -1, NULL, 0, NULL, NULL);
			if (len > 0) {
				_data = new tjs_nchar[len + 1];
				::WideCharToMultiByte(CP_UTF8, 0, n, -1, _data, len, NULL, NULL);
				_data[len] = '\0';
			}
		} else {
			tjs_int len = str.GetNarrowStrLen();
			if (len > 0) {
				_data = new tjs_nchar[len+1];
				str.ToNarrowStr(_data, len+1);
			}
		}
	}
	~NarrowString() {
		delete[] _data;
	}

	const tjs_nchar *data() {
		return _data;
	}

	operator const char *() const
	{
		return (const char *)_data;
	}
};

#endif
