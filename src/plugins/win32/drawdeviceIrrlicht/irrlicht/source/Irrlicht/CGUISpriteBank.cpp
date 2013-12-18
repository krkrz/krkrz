// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUISpriteBank.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "ITexture.h"

namespace irr
{
namespace gui
{

CGUISpriteBank::CGUISpriteBank(IGUIEnvironment* env) :
	Environment(env), Driver(0)
{
	if (Environment)
	{
		Driver = Environment->getVideoDriver();
		if (Driver)
			Driver->grab();
	}
}


CGUISpriteBank::~CGUISpriteBank()
{
	// drop textures
	for (u32 i=0; i<Textures.size(); ++i)
		if (Textures[i])
			Textures[i]->drop();

	// drop video driver
	if (Driver)
		Driver->drop();
}


core::array< core::rect<s32> >& CGUISpriteBank::getPositions()
{
	return Rectangles;
}


core::array< SGUISprite >& CGUISpriteBank::getSprites()
{
	return Sprites;
}


u32 CGUISpriteBank::getTextureCount() const
{
	return Textures.size();
}


video::ITexture* CGUISpriteBank::getTexture(u32 index) const
{
	if (index < Textures.size())
		return Textures[index];
	else
		return 0;
}


void CGUISpriteBank::addTexture(video::ITexture* texture)
{
	if (texture)
		texture->grab();

	Textures.push_back(texture);
}


void CGUISpriteBank::setTexture(u32 index, video::ITexture* texture)
{
	while (index > Textures.size())
		Textures.push_back(0);

	if (Textures[index])
		Textures[index]->drop();

	if (texture)
		texture->grab();

	Textures[index] = texture;
}


//! draws a sprite in 2d with scale and color
void CGUISpriteBank::draw2DSprite(u32 index, const core::position2di& pos,
		const core::rect<s32>* clip, const video::SColor& color,
		u32 starttime, u32 currenttime, bool loop, bool center)
{
	if (Sprites[index].Frames.empty() || index >= Sprites.size())
		return;

	// work out frame number
	u32 frame = 0;
	if (Sprites[index].frameTime)
	{
		u32 f = ((currenttime - starttime) / Sprites[index].frameTime);
		if (loop)
			frame = f % Sprites[index].Frames.size();
		else
			frame = (f >= Sprites[index].Frames.size()) ? Sprites[index].Frames.size()-1 : f;
	}

	const video::ITexture* tex = Textures[Sprites[index].Frames[frame].textureNumber];
	if (!tex)
		return;

	const u32 rn = Sprites[index].Frames[frame].rectNumber;
	if (rn >= Rectangles.size())
		return;

	const core::rect<s32>& r = Rectangles[rn];

	if (center)
	{
		core::position2di p = pos;
		p -= r.getSize() / 2;
		Driver->draw2DImage(tex, p, r, clip, color, true);
	}
	else
	{
		Driver->draw2DImage(tex, pos, r, clip, color, true);
	}
}


} // namespace gui
} // namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

