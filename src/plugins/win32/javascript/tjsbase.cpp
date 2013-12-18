#include "tjsbase.h"

extern Persistent<Context> mainContext;

void
TJSBase::wrap(Handle<Object> obj)
{
	bool ret = obj->SetHiddenValue(String::New(TJSINSTANCENAME), External::Wrap(this));
}

// ƒpƒ‰ƒ[ƒ^æ“¾
bool
TJSBase::getVariant(tTJSVariant &result, Handle<Object> obj)
{
	Local<Value> v = obj->GetHiddenValue(String::New(TJSINSTANCENAME));
	bool empty = v.IsEmpty();
	if (!v.IsEmpty()) {
		TJSBase *base = (TJSBase*)External::Unwrap(v);
		result = base->variant;
		return true;
	}
	return false;
}
