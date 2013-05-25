
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

//#include <exception>
#include "tstring.h"

class Exception /*: public std::exception*/ {
	tstring message_;
public:
	Exception( const tstring& mes ) : message_(mes) {
	}
	virtual const TCHAR* what() const {
		return message_.c_str();
	}
};

class EAbort : public Exception {
public:
	EAbort( const TCHAR* mes ) : Exception(tstring(mes)) {
	}
};

#endif // __EXCEPTION_H__
