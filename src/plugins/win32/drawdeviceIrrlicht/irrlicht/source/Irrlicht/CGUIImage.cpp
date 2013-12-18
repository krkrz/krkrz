// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIImage.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"

namespace irr
{
namespace gui
{



//! constructor
CGUIImage::CGUIImage(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: IGUIImage(environment, parent, id, rectangle), Color(255,255,255,255),
	Texture(0), UseAlphaChannel(false), ScaleImage(false)
{
	#ifdef _DEBUG
	setDebugName("CGUIImage");
	#endif
}



//! destructor
CGUIImage::~CGUIImage()
{
	if (Texture)
		Texture->drop();
}



//! sets an image
void CGUIImage::setImage(video::ITexture* image)
{
	if (image == Texture)
		return;

	if (Texture)
		Texture->drop();

	Texture = image;

	if (Texture)
		Texture->grab();
}


//! sets the color of the image
void CGUIImage::setColor(video::SColor color)
{
	Color = color;
}


//! draws the element and its children
void CGUIImage::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> rect = AbsoluteRect;

	if (Texture)
	{
		if (ScaleImage)
		{
			video::SColor Colors[4];
			Colors[0] = Color;
			Colors[1] = Color;
			Colors[2] = Color;
			Colors[3] = Color;

			driver->draw2DImage(Texture, AbsoluteRect, 
				core::rect<s32>(core::position2d<s32>(0,0), Texture->getOriginalSize()),
				&AbsoluteClippingRect, Colors, UseAlphaChannel);
		}
		else
		{
			driver->draw2DImage(Texture, AbsoluteRect.UpperLeftCorner, 
				core::rect<s32>(core::position2d<s32>(0,0), Texture->getOriginalSize()),
				&AbsoluteClippingRect, Color, UseAlphaChannel);
		}
	}
	else
	{
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_DARK_SHADOW), AbsoluteRect, &AbsoluteClippingRect);
	}

	IGUIElement::draw();
}


//! sets if the image should use its alpha channel to draw itself
void CGUIImage::setUseAlphaChannel(bool use)
{
	UseAlphaChannel = use;
}

//! sets if the image should use its alpha channel to draw itself
void CGUIImage::setScaleImage(bool scale)
{
	ScaleImage = scale;
}

//! Returns true if the image is scaled to fit, false if not
bool CGUIImage::isImageScaled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return ScaleImage;
}

//! Returns true if the image is using the alpha channel, false if not
bool CGUIImage::isAlphaChannelUsed() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return UseAlphaChannel;
}


//! Writes attributes of the element.
void CGUIImage::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIImage::serializeAttributes(out,options);

	out->addTexture	("Texture", Texture);
	out->addBool	("UseAlphaChannel", UseAlphaChannel);
	out->addColor	("Color", Color);
	out->addBool	("ScaleImage", ScaleImage);

}

//! Reads attributes of the element
void CGUIImage::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIImage::deserializeAttributes(in,options);

	setImage(in->getAttributeAsTexture("Texture"));
	setUseAlphaChannel(in->getAttributeAsBool("UseAlphaChannel"));
	setColor(in->getAttributeAsColor("Color"));
	setScaleImage(in->getAttributeAsBool("ScaleImage"));
}




} // end namespace gui
} // end namespace irr


#endif // _IRR_COMPILE_WITH_GUI_
