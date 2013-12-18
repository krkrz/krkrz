#ifndef __BASE64_H_
#define __BASE64_H_

#include <tchar.h>
#include <string>

using namespace std;
typedef basic_string<TCHAR> tstring;

tstring base64encode(const TCHAR *input, int len);

#endif __BASE64_H_
