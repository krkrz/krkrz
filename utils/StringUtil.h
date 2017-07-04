
#ifndef __STRING_UTILITY_H__
#define __STRING_UTILITY_H__

#include <string>
#include <iostream>
#include <algorithm>
#include <locale>

struct equal_char_ignorecase {
	inline bool operator()(char x, char y) const {
		std::locale loc;
		return std::tolower(x,loc) == std::tolower(y,loc);
	}
};

inline bool icomp( const std::string& x, const std::string& y ) {
	return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin(), equal_char_ignorecase());
}

struct equal_wchar_ignorecase {
	inline bool operator()(tjs_char x, tjs_char y) const {
		std::locale loc;
		return std::tolower(x,loc) == std::tolower(y,loc);
	}
};
inline bool icomp( const tjs_string& x, const tjs_string& y ) {
	return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin(), equal_wchar_ignorecase());
}

inline std::string Trim( const std::string& val ) {
	static const char* TRIM_STR=" \01\02\03\04\05\06\a\b\t\n\v\f\r\x0E\x0F\x7F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";
	std::string::size_type pos = val.find_first_not_of( TRIM_STR );
	std::string::size_type lastpos = val.find_last_not_of( TRIM_STR );
	if( pos == lastpos ) {
		if( pos == std::string::npos ) {
			return val;
		} else {
			return val.substr(pos,1);
		}
	} else {
		std::string::size_type len = lastpos - pos + 1;
		return val.substr(pos,len);
	}
}
inline tjs_string Trim( const tjs_string& val ) {
	static const tjs_char* TRIM_STR=TJS_W(" \01\02\03\04\05\06\a\b\t\n\v\f\r\x0E\x0F\x7F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");
	tjs_string::size_type pos = val.find_first_not_of( TRIM_STR );
	tjs_string::size_type lastpos = val.find_last_not_of( TRIM_STR );
	if( pos == lastpos ) {
		if( pos == tjs_string::npos ) {
			return val;
		} else {
			return val.substr(pos,1);
		}
	} else {
		tjs_string::size_type len = lastpos - pos + 1;
		return val.substr(pos,len);
	}
}
template <typename TContainer>
void split( const tjs_string& val, const tjs_string& delim, TContainer& result ) {
	result.clear();
	tjs_string::size_type pos = 0;
	while( pos != tjs_string::npos ) {
		tjs_string::size_type p = val.find( delim, pos );
		if( p == tjs_string::npos ){
			result.push_back( val.substr(pos) );
			break;
		} else {
			result.push_back( val.substr(pos, p - pos) );
		}
		pos = p + delim.size();
	}
}

#endif // __STRING_UTILITY_H__
