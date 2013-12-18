// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIFont.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "os.h"
#include "IGUIEnvironment.h"
#include "IXMLReader.h"
#include "IReadFile.h"
#include "IVideoDriver.h"
#include "IGUISpriteBank.h"
#include "CImage.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIFont::CGUIFont(IGUIEnvironment *env, const c8* filename)
: Driver(0), SpriteBank(0), Environment(env), WrongCharacter(0),
	MaxHeight(0), GlobalKerningWidth(0), GlobalKerningHeight(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIFont");
	#endif

	if (Environment)
	{
		// don't grab environment, to avoid circular references
		Driver = Environment->getVideoDriver();

		SpriteBank = Environment->addEmptySpriteBank(filename);
		if (SpriteBank)
			SpriteBank->grab();
	}

	if (Driver)
		Driver->grab();
}


//! destructor
CGUIFont::~CGUIFont()
{
	if (Driver)
		Driver->drop();

	if (SpriteBank)
		SpriteBank->drop();
}


//! loads a font file from xml
bool CGUIFont::load(io::IXMLReader* xml)
{
	if (!SpriteBank)
		return false;

	while (xml->read())
	{
		if (io::EXN_ELEMENT == xml->getNodeType())
		{
			if (core::stringw(L"Texture") == xml->getNodeName())
			{
				// add a texture
				core::stringc fn = xml->getAttributeValue(L"filename");
				u32 i = (u32)xml->getAttributeValueAsInt(L"index");
				core::stringw alpha = xml->getAttributeValue(L"hasAlpha");

				while (i+1 > SpriteBank->getTextureCount())
					SpriteBank->addTexture(0);

				// disable mipmaps+filtering
				bool mipmap = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
				Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

				// load texture
				SpriteBank->setTexture(i, Driver->getTexture(fn.c_str()));

				// set previous mip-map+filter state
				Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, mipmap);

				// couldn't load texture, abort.
				if (!SpriteBank->getTexture(i))
				{
					os::Printer::log("Unable to load all textures in the font, aborting", ELL_ERROR);
					_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
					return false;
				}
				else
				{
					// colorkey texture rather than alpha channel?
					if (alpha == core::stringw("false"))
						Driver->makeColorKeyTexture(SpriteBank->getTexture(i), core::position2di(0,0));
				}
			} 
			else if (core::stringw(L"c") == xml->getNodeName())
			{
				// adding a character to this font
				SFontArea a;
				SGUISpriteFrame f;
				SGUISprite s;
				core::rect<s32> rectangle;

				a.underhang		= xml->getAttributeValueAsInt(L"u");
				a.overhang		= xml->getAttributeValueAsInt(L"o");
				a.spriteno		= SpriteBank->getSprites().size();
				s32 texno		= xml->getAttributeValueAsInt(L"i");

				// parse rectangle
				core::stringc rectstr	= xml->getAttributeValue(L"r");
				wchar_t ch				= xml->getAttributeValue(L"c")[0];

				const c8 *c = rectstr.c_str();
				s32 val;
				val = 0;
				while (*c >= '0' && *c <= '9') 
				{ 
					val *= 10; 
					val += *c - '0'; 
					c++;
				}
				rectangle.UpperLeftCorner.X = val;
				while (*c == L' ' || *c == L',') c++;

				val = 0;
				while (*c >= '0' && *c <= '9') 
				{ 
					val *= 10; 
					val += *c - '0'; 
					c++;
				}
				rectangle.UpperLeftCorner.Y = val;
				while (*c == L' ' || *c == L',') c++;

				val = 0;
				while (*c >= '0' && *c <= '9') 
				{ 
					val *= 10; 
					val += *c - '0'; 
					c++;
				}
				rectangle.LowerRightCorner.X = val;
				while (*c == L' ' || *c == L',') c++;

				val = 0;
				while (*c >= '0' && *c <= '9') 
				{ 
					val *= 10; 
					val += *c - '0'; 
					c++;
				}
				rectangle.LowerRightCorner.Y = val;

				CharacterMap.insert(ch,Areas.size());

				// make frame
				f.rectNumber = 	SpriteBank->getPositions().size();
				f.textureNumber = texno;

				// add frame to sprite
				s.Frames.push_back(f);
				s.frameTime = 0;

				// add rectangle to sprite bank
				SpriteBank->getPositions().push_back(rectangle);
				a.width = rectangle.getWidth();

				// add sprite to sprite bank
				SpriteBank->getSprites().push_back(s);

				// add character to font
				Areas.push_back(a);
			}
		}
	}

	// set bad character
	WrongCharacter = getAreaFromCharacter(L' ');

	setMaxHeight();

	return true;
}


void CGUIFont::setMaxHeight()
{
	MaxHeight = 0;
	s32 t;

	core::array< core::rect<s32> >& p = SpriteBank->getPositions();

	for (u32 i=0; i<p.size(); ++i)
	{
		t = p[i].getHeight();
		if (t>MaxHeight)
			MaxHeight = t;
	}

}


//! loads a font file, native file needed, for texture parsing
bool CGUIFont::load(io::IReadFile* file)
{
	if (!Driver)
		return false;

	return loadTexture(Driver->createImageFromFile(file),
				file->getFileName());
}


//! loads a font file, native file needed, for texture parsing
bool CGUIFont::load(const c8* filename)
{
	if (!Driver)
		return false;
	return loadTexture(Driver->createImageFromFile( filename ),
				filename);
}


//! load & prepare font from ITexture
bool CGUIFont::loadTexture(video::IImage* image, const c8* name)
{
	if (!image)
		return false;

	s32 lowerRightPositions = 0;

	video::IImage* tmpImage=image;
	bool deleteTmpImage=false;
	switch(image->getColorFormat())
	{
	case video::ECF_R5G6B5:
		tmpImage =  new video::CImage(video::ECF_A1R5G5B5,image);
		deleteTmpImage=true;
	case video::ECF_A1R5G5B5:
		readPositions16bit(tmpImage, lowerRightPositions);
		break;
	case video::ECF_R8G8B8:
		tmpImage = new video::CImage(video::ECF_A8R8G8B8,image);
		deleteTmpImage=true;
	case video::ECF_A8R8G8B8:
		readPositions32bit (tmpImage, lowerRightPositions);
		break;
	}

	WrongCharacter = getAreaFromCharacter(L' ');

	// output warnings
	if (!lowerRightPositions || !SpriteBank->getSprites().size())
		os::Printer::log("The amount of upper corner pixels or lower corner pixels is == 0, font file may be corrupted.", ELL_ERROR);
	else
	if (lowerRightPositions != (s32)SpriteBank->getPositions().size())
		os::Printer::log("The amount of upper corner pixels and the lower corner pixels is not equal, font file may be corrupted.", ELL_ERROR);

	bool ret = ( !SpriteBank->getSprites().empty() && lowerRightPositions );


	if ( ret )
	{
		SpriteBank->addTexture(Driver->addTexture(name, tmpImage));
	}
	if (deleteTmpImage)
		tmpImage->drop();
	image->drop();

	setMaxHeight();

	return ret;
}


void CGUIFont::readPositions32bit(video::IImage* image, s32& lowerRightPositions)
{
	const core::dimension2d<s32>& size = image->getDimension();

	s32* p = (s32*)image->lock();
	if (!p)
	{
		os::Printer::log("Could not lock texture while preparing texture for a font.", ELL_ERROR);
		return;
	}

	// fix half alpha of top left pixel in some font textures
	p[0] |= 0xFF000000;

	s32 colorTopLeft = p[0];
	s32 colorLowerRight = *(p+1);
	s32 colorBackGround = *(p+2);
	s32 colorBackGroundTransparent = 0; // 0x00FFFFFF & colorBackGround;

	*(p+1) = colorBackGround;

	// start parsing

	core::position2d<s32> pos(0,0);
	for (pos.Y=0; pos.Y<size.Height; ++pos.Y)
	{
		for (pos.X=0; pos.X<size.Width; ++pos.X)
		{
			if ( *p == colorTopLeft)
			{
				*p = colorBackGroundTransparent;
				SpriteBank->getPositions().push_back(core::rect<s32>(pos, pos));
			}
			else
			if (*p == colorLowerRight)
			{
				if (SpriteBank->getPositions().size()<=(u32)lowerRightPositions)
				{
					image->unlock();
					lowerRightPositions = 0;
					return;
				}

				*p = colorBackGroundTransparent;
				SpriteBank->getPositions()[lowerRightPositions].LowerRightCorner = pos;
				// add frame to sprite bank
				SGUISpriteFrame f;
				f.rectNumber = lowerRightPositions;
				f.textureNumber = 0;
				SGUISprite s;
				s.Frames.push_back(f);
				s.frameTime = 0;
				SpriteBank->getSprites().push_back(s);
				// add character to font
				SFontArea a;
				a.overhang = 0;
				a.underhang = 0;
				a.spriteno = lowerRightPositions;
				a.width = SpriteBank->getPositions()[lowerRightPositions].getWidth();
				Areas.push_back(a);
				// map letter to character
				wchar_t ch = (wchar_t)(lowerRightPositions + 32);
				CharacterMap.set(ch, lowerRightPositions);

				++lowerRightPositions;
			}
			else
			if (*p == colorBackGround)
			{
				*p = colorBackGroundTransparent;
			}
			++p;
		}
	}

	// Positions parsed.

	image->unlock();
}


void CGUIFont::readPositions16bit(video::IImage* image, s32& lowerRightPositions)
{
	core::dimension2d<s32> size = image->getDimension();

	s16* p = (s16*)image->lock();
	if (!p)
	{
		os::Printer::log("Could not lock texture while preparing texture for a font.", ELL_ERROR);
		return;
	}

	// fix half alpha of top left pixel in some font textures
	p[0] |= 0x8000;

	s16 colorTopLeft = p[0];
	s16 colorLowerRight = *(p+1);
	s16 colorBackGround = *(p+2);
	s16 colorBackGroundTransparent = 0; // 0x7FFF & colorBackGround;

	*(p+1) = colorBackGround;

	// start parsing

	core::position2d<s32> pos(0,0);
	for (pos.Y=0; pos.Y<size.Height; ++pos.Y)
	{
		for (pos.X=0; pos.X<size.Width; ++pos.X)
		{
			if (*p == colorTopLeft)
			{
				*p = colorBackGroundTransparent;
				SpriteBank->getPositions().push_back(core::rect<s32>(pos, pos));
			}
			else
			if (*p == colorLowerRight)
			{
				// too many lower right points
				if (SpriteBank->getPositions().size()<=(u32)lowerRightPositions)
				{
					image->unlock();
					lowerRightPositions = 0;
					return;
				}

				*p = colorBackGroundTransparent;
				SpriteBank->getPositions()[lowerRightPositions].LowerRightCorner = pos;
				// add frame to sprite bank
				SGUISpriteFrame f;
				f.rectNumber = lowerRightPositions;
				f.textureNumber = 0;
				SGUISprite s;
				s.Frames.push_back(f);
				s.frameTime = 0;
				SpriteBank->getSprites().push_back(s);
				// add character to font
				SFontArea a;
				a.overhang = 0;
				a.underhang = 0;
				a.spriteno = lowerRightPositions;
				a.width = SpriteBank->getPositions()[lowerRightPositions].getWidth();
				Areas.push_back(a);
				// map letter to character
				wchar_t ch = (wchar_t)(lowerRightPositions + 32);
				CharacterMap.set(ch, lowerRightPositions);

				++lowerRightPositions;
			}
			else
			if (*p == colorBackGround)
				*p = colorBackGroundTransparent;
			++p;
		}
	}

	// Positions parsed.

	image->unlock();
}


//! returns the dimension of text
core::dimension2d<s32> CGUIFont::getDimension(const wchar_t* text) const
{
	core::dimension2d<s32> dim(0, 0);
	core::dimension2d<s32> thisLine(0, MaxHeight);

	for (const wchar_t* p = text; *p; ++p)
	{
		bool lineBreak=false;
		if (*p == L'\r') // Mac or Windows breaks
		{
			lineBreak = true;
			if (p[1] == L'\n') // Windows breaks
				++p;
		}
		else if (*p == L'\n') // Unix breaks
		{
			lineBreak = true;
		}
		if (lineBreak)
		{
			dim.Height += thisLine.Height;
			if (dim.Width < thisLine.Width)
				dim.Width = thisLine.Width;
			thisLine.Width = 0;
			continue;
		}

		const SFontArea &area = Areas[getAreaFromCharacter(*p)];

		thisLine.Width += area.underhang;
		thisLine.Width += area.width + area.overhang + GlobalKerningWidth;
	}

	dim.Height += thisLine.Height;
	if (dim.Width < thisLine.Width)
		dim.Width = thisLine.Width;

	return dim;
}


//! set an Pixel Offset on Drawing ( scale position on width )
void CGUIFont::setKerningWidth ( s32 kerning )
{
	GlobalKerningWidth = kerning;
}


//! set an Pixel Offset on Drawing ( scale position on width )
s32 CGUIFont::getKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter) const
{
	s32 ret = GlobalKerningWidth;

	if (thisLetter)
	{
		ret += Areas[getAreaFromCharacter(*thisLetter)].overhang;

		if (previousLetter)
		{
			ret += Areas[getAreaFromCharacter(*previousLetter)].underhang;
		}
	}

	return ret;
}


//! set an Pixel Offset on Drawing ( scale position on height )
void CGUIFont::setKerningHeight ( s32 kerning )
{
	GlobalKerningHeight = kerning;
}


//! set an Pixel Offset on Drawing ( scale position on height )
s32 CGUIFont::getKerningHeight () const
{
	return GlobalKerningHeight;
}


//! returns the sprite number from a given character
u32 CGUIFont::getSpriteNoFromChar(const wchar_t *c) const
{
	return Areas[getAreaFromCharacter(*c)].spriteno;
}


s32 CGUIFont::getAreaFromCharacter(const wchar_t c) const
{
	core::map<wchar_t, s32>::Node* n = CharacterMap.find(c);
	if (n)
		return n->getValue();
	else
		return WrongCharacter;
}


/*
//! draws an text and clips it to the specified rectangle if wanted
void CGUIFont::draw(const wchar_t* text, const core::rect<s32>& position, video::SColor color, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
	if (!Driver)
		return;

	core::dimension2d<s32> textDimension;
	core::position2d<s32> offset = position.UpperLeftCorner;

	if (hcenter || vcenter)
	{
		textDimension = getDimension(text);

		if (hcenter)
			offset.X = ((position.getWidth() - textDimension.Width)>>1) + offset.X;

		if (vcenter)
			offset.Y = ((position.getHeight() - textDimension.Height)>>1) + offset.Y;
	}

	core::array<s32> indices;
	indices.reallocate(core::stringw(text).size());
	u32 n;
	while(*text)
	{
		n = (*text) - 32;
		if ( n > Positions.size())
			n = WrongCharacter;
		indices.push_back(n);
		++text;
	}
	Driver->draw2DImage(Texture, offset, Positions, indices, GlobalKerningWidth, clip, color, true);
}
*/


//! draws some text and clips it to the specified rectangle if wanted
void CGUIFont::draw(const wchar_t* text, const core::rect<s32>& position, video::SColor color, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
	if (!Driver)
		return;

	core::dimension2d<s32> textDimension;
	core::position2d<s32> offset = position.UpperLeftCorner;
	core::rect<s32> pos;

	if (hcenter || vcenter || clip)
		textDimension = getDimension(text);

	if (hcenter)
		offset.X = ((position.getWidth() - textDimension.Width)>>1) + offset.X;

	if (vcenter)
		offset.Y = ((position.getHeight() - textDimension.Height)>>1) + offset.Y;

	if (clip)
	{
		core::rect<s32> clippedRect(offset, textDimension);
		clippedRect.clipAgainst(*clip);
		if (!clippedRect.isValid())
			return;
	}

	while(*text)
	{
		SFontArea& area = Areas[getAreaFromCharacter(*text)];

		offset.X += area.underhang;

		SpriteBank->draw2DSprite(area.spriteno, offset, clip, color);

		offset.X += area.width + area.overhang + GlobalKerningWidth;

		++text;
	}
}


//! Calculates the index of the character in the text which is on a specific position.
s32 CGUIFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
	s32 x = 0;
	s32 idx = 0;

	while (text[idx])
	{
		const SFontArea& a = Areas[getAreaFromCharacter(text[idx])];

		x += a.width + a.overhang + a.underhang;

		if (x >= pixel_x)
			return idx;

		++idx;
	}

	return -1;
}


IGUISpriteBank* CGUIFont::getSpriteBank() const
{
	return SpriteBank;
}

} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_
