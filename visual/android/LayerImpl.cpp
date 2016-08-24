//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Layer Management
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "LayerImpl.h"
#include "MsgIntf.h"

#include "TVPColor.h"

//---------------------------------------------------------------------------
//  convert color identifier or TVP system color to/from actual color
//---------------------------------------------------------------------------
tjs_uint32 TVPToActualColor(tjs_uint32 color)
{
	if(color & 0xff000000)
	{
		color = ColorToRGB( color ); // system color to RGB
		// convert byte order to 0xRRGGBB since ColorToRGB's return value is in
		// a format of 0xBBGGRR.
		return ((color&0xff)<<16) + (color&0xff00) + ((color&0xff0000)>>16);
	}
	else
	{
		return color;
	}
}
//---------------------------------------------------------------------------
tjs_uint32 TVPFromActualColor(tjs_uint32 color)
{
	color &= 0xffffff;
	return color;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTJSNI_Layer
//---------------------------------------------------------------------------
tTJSNI_Layer::tTJSNI_Layer(void)
{
}
//---------------------------------------------------------------------------
tTJSNI_Layer::~tTJSNI_Layer()
{
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_Layer::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	return tTJSNI_BaseLayer::Construct(numparams, param, tjs_obj);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD
tTJSNI_Layer::Invalidate()
{
	tTJSNI_BaseLayer::Invalidate();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNC_Layer::CreateNativeInstance : returns proper instance object
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_Layer::CreateNativeInstance()
{
	return new tTJSNI_Layer();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCreateNativeClass_Layer
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Layer()
{
	return new tTJSNC_Layer();
}
//---------------------------------------------------------------------------

