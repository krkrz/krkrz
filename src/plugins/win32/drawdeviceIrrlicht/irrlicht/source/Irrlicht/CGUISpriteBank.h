// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_SPRITE_BANK_H_INCLUDED__
#define __C_GUI_SPRITE_BANK_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISpriteBank.h"

namespace irr
{

namespace video
{
	class IVideoDriver;
	class ITexture;
}

namespace gui
{

	class IGUIEnvironment;

//! Sprite bank interface.
class CGUISpriteBank : public IGUISpriteBank
{
public:

	CGUISpriteBank(IGUIEnvironment* env);
	virtual ~CGUISpriteBank();

	virtual core::array< core::rect<s32> >& getPositions();
	virtual core::array< SGUISprite >& getSprites();

	virtual u32 getTextureCount() const;
	virtual video::ITexture* getTexture(u32 index) const;
	virtual void addTexture(video::ITexture* texture);
	virtual void setTexture(u32 index, video::ITexture* texture);

	//! draws a sprite in 2d with position and color
	virtual void draw2DSprite(u32 index, const core::position2di& pos, const core::rect<s32>* clip=0,
				const video::SColor& color= video::SColor(255,255,255,255),
				u32 starttime=0, u32 currenttime=0, bool loop=true, bool center=false);

protected:

	core::array<SGUISprite>  Sprites;
	core::array< core::rect<s32> > Rectangles;
	core::array<video::ITexture*> Textures;
	IGUIEnvironment* Environment;
	video::IVideoDriver* Driver;

};

} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

#endif // __C_GUI_SPRITE_BANK_H_INCLUDED__


