
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__


class Exception /*: public std::exception*/ {
	tjs_string message_;
public:
	Exception( const tjs_string& mes ) : message_(mes) {
	}
	virtual const tjs_char* what() const {
		return message_.c_str();
	}
};

class EAbort : public Exception {
public:
	EAbort( const tjs_char* mes ) : Exception(tjs_string(mes)) {
	}
};

#endif // __EXCEPTION_H__
