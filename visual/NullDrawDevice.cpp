
#include "tjsCommHead.h"
#include "DrawDevice.h"
#include "NullDrawDevice.h"


//---------------------------------------------------------------------------
// tTJSNI_NullDrawDevice : NullDrawDevice TJS native class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_NullDrawDevice::ClassID = (tjs_uint32)-1;
tTJSNC_NullDrawDevice::tTJSNC_NullDrawDevice() : tTJSNativeClass(TJS_W("NullDrawDevice"))
{
	// register native methods/properties

	TJS_BEGIN_NATIVE_MEMBERS(NullDrawDevice)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
// constructor/methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_NullDrawDevice,
	/*TJS class name*/NullDrawDevice)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/NullDrawDevice)
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(interface)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_NullDrawDevice);
		*result = reinterpret_cast<tjs_int64>(_this->GetDevice());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(interface)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
iTJSNativeInstance *tTJSNC_NullDrawDevice::CreateNativeInstance()
{
	return new tTJSNI_NullDrawDevice();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
tTJSNI_NullDrawDevice::tTJSNI_NullDrawDevice()
{
	Device = new tTVPNullDrawDevice();
}
//---------------------------------------------------------------------------
tTJSNI_NullDrawDevice::~tTJSNI_NullDrawDevice()
{
	if(Device) Device->Destruct(), Device = nullptr;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_NullDrawDevice::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_NullDrawDevice::Invalidate()
{
	if(Device) Device->Destruct(), Device = nullptr;
}
//---------------------------------------------------------------------------


