#ifndef __TJSBASE_H__
#define __TJSBASE_H__

#include <windows.h>
#include "tp_stub.h"
#include <v8.h>
using namespace v8;

#define TJSINSTANCENAME L"__tjsinstance__"

class TJSBase {
public:
	TJSBase(const tTJSVariant &variant) : variant(variant) {}
	virtual ~TJSBase() {};
	void wrap(Handle<Object> obj);
	static bool getVariant(tTJSVariant &result, Handle<Object> obj);
protected:
	tTJSVariant variant;
};

#endif
