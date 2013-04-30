
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>

class Exception : public std::exception {
	std::string message_;
public:
	Exception( const std::string& mes ) : message_(mes) {
	}
	virtual const char* what() const {
		return message_.c_str();
	}
};

class EAbort : public Exception {
public:
	EAbort( const char* mes ) : Exception(std::string(mes)) {
	}
};

#endif // __EXCEPTION_H__
