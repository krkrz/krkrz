// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIStaticText.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "rect.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIStaticText::CGUIStaticText(const wchar_t* text, bool border,
			IGUIEnvironment* environment, IGUIElement* parent,
			s32 id, const core::rect<s32>& rectangle,
			bool background)
: IGUIStaticText(environment, parent, id, rectangle), Border(border), 
	HAlign(EGUIA_UPPERLEFT), VAlign(EGUIA_UPPERLEFT),
	OverrideColorEnabled(false), WordWrap(false), Background(background),
	OverrideColor(video::SColor(101,255,255,255)), BGColor(video::SColor(101,210,210,210)),
	OverrideFont(0), LastBreakFont(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIStaticText");
	#endif

	Text = text;
	if (environment && environment->getSkin())
	{
		BGColor = environment->getSkin()->getColor(gui::EGDC_3D_FACE);
	}
}


//! destructor
CGUIStaticText::~CGUIStaticText()
{
	if (OverrideFont)
		OverrideFont->drop();
}


//! draws the element and its children
void CGUIStaticText::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> frameRect(AbsoluteRect);

	// draw background

	if (Background)
	{
		driver->draw2DRectangle(BGColor, frameRect, &AbsoluteClippingRect);
	}

	// draw the border

	if (Border)
	{
		skin->draw3DSunkenPane(this, 0, true, false, frameRect, &AbsoluteClippingRect);
		frameRect.UpperLeftCorner.X += skin->getSize(EGDS_TEXT_DISTANCE_X);
	}

	// draw the text
	if (Text.size())
	{
		IGUIFont* font = OverrideFont;
		if (!OverrideFont)
			font = skin->getFont();

		if (font)
		{
			if (!WordWrap)
			{
				if (VAlign == EGUIA_LOWERRIGHT)
				{
					frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 
						font->getDimension(L"A").Height - font->getKerningHeight();
				}
				if (HAlign == EGUIA_LOWERRIGHT)
				{
					frameRect.UpperLeftCorner.X = frameRect.LowerRightCorner.X - 
						font->getDimension(Text.c_str()).Width;
				}

				font->draw(Text.c_str(), frameRect,
					OverrideColorEnabled ? OverrideColor : skin->getColor(IsEnabled ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT),
					HAlign == EGUIA_CENTER, VAlign == EGUIA_CENTER, &AbsoluteClippingRect);
			}
			else
			{
				if (font != LastBreakFont)
					breakText();

				core::rect<s32> r = frameRect;
				s32 height = font->getDimension(L"A").Height + font->getKerningHeight();
				s32 totalHeight = height * BrokenText.size();
				if (VAlign == EGUIA_CENTER)
				{
					r.UpperLeftCorner.Y = r.getCenter().Y - (totalHeight / 2);
				}
				else if (VAlign == EGUIA_LOWERRIGHT)
				{
					r.UpperLeftCorner.Y = r.LowerRightCorner.Y - totalHeight;
				}

				for (u32 i=0; i<BrokenText.size(); ++i)
				{
					if (HAlign == EGUIA_LOWERRIGHT)
					{
						r.UpperLeftCorner.X = frameRect.LowerRightCorner.X - 
							font->getDimension(BrokenText[i].c_str()).Width;
					}

					font->draw(BrokenText[i].c_str(), r,
						OverrideColorEnabled ? OverrideColor : skin->getColor(IsEnabled ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT),
						HAlign == EGUIA_CENTER, false, &AbsoluteClippingRect);

					r.LowerRightCorner.Y += height;
					r.UpperLeftCorner.Y += height;
				}
			}
		}
	}

	IGUIElement::draw();
}


//! Sets another skin independent font.
void CGUIStaticText::setOverrideFont(IGUIFont* font)
{
	if (OverrideFont == font)
		return;

	if (OverrideFont)
		OverrideFont->drop();

	OverrideFont = font;

	if (OverrideFont)
		OverrideFont->grab();

	breakText();
}


IGUIFont * CGUIStaticText::getOverrideFont() const
{
	return OverrideFont;
}


//! Sets another color for the text.
void CGUIStaticText::setOverrideColor(video::SColor color)
{
	OverrideColor = color;
	OverrideColorEnabled = true;
}


//! Sets another color for the text.
void CGUIStaticText::setBackgroundColor(video::SColor color)
{
	BGColor = color;
	Background = true;
}


//! Sets whether to draw the background
void CGUIStaticText::setDrawBackground(bool draw)
{
	Background = draw;
}


//! Sets whether to draw the border
void CGUIStaticText::setDrawBorder(bool draw)
{
	Border = draw;
}


void CGUIStaticText::setTextAlignment(EGUI_ALIGNMENT horizontal, EGUI_ALIGNMENT vertical)
{
	HAlign = horizontal;
	VAlign = vertical;
}


video::SColor const& CGUIStaticText::getOverrideColor() const
{
	return OverrideColor;
}


//! Sets if the static text should use the overide color or the
//! color in the gui skin.
void CGUIStaticText::enableOverrideColor(bool enable)
{
	OverrideColorEnabled = enable;
}


bool CGUIStaticText::isOverrideColorEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return OverrideColorEnabled;
}


//! Enables or disables word wrap for using the static text as
//! multiline text control.
void CGUIStaticText::setWordWrap(bool enable)
{
	WordWrap = enable;
	breakText();
}


bool CGUIStaticText::isWordWrapEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return WordWrap;
}


//! Breaks the single text line.
void CGUIStaticText::breakText()
{
	IGUISkin* skin = Environment->getSkin();

	if (!WordWrap || !skin)
		return;

	BrokenText.clear();

	IGUIFont* font = OverrideFont;
	if (!OverrideFont)
		font = skin->getFont();

	if (!font)
		return;

	LastBreakFont = font;

	core::stringw line;
	core::stringw word;
	core::stringw whitespace;
	s32 size = Text.size();
	s32 length = 0;
	s32 elWidth = RelativeRect.getWidth() - 6;
	wchar_t c;

	for (s32 i=0; i<size; ++i)
	{
		c = Text[i];
		bool lineBreak = false;

		if (c == L'\r') // Mac or Windows breaks
		{
			lineBreak = true;
			c = ' ';
			if (Text[i+1] == L'\n') // Windows breaks
			{
				Text.erase(i+1);
				--size;
			}
		}
		else if (c == L'\n') // Unix breaks
		{
			lineBreak = true;
			c = ' ';
		}

		if (c == L' ' || c == 0 || i == (size-1))
		{
			if (word.size())
			{
				// here comes the next whitespace, look if
				// we can break the last word to the next line.
				s32 whitelgth = font->getDimension(whitespace.c_str()).Width;
				s32 worldlgth = font->getDimension(word.c_str()).Width;

				if (length + worldlgth + whitelgth > elWidth)
				{
					// break to next line
					length = worldlgth;
					BrokenText.push_back(line);
					line = word;
				}
				else
				{
					// add word to line
					line += whitespace;
					line += word;
					length += whitelgth + worldlgth;
				}

				word = L"";
				whitespace = L"";
			}

			whitespace += c;

			// compute line break
			if (lineBreak)
			{
				line += whitespace;
				line += word;
				BrokenText.push_back(line);
				line = L"";
				word = L"";
				whitespace = L"";
				length = 0;
			}
		}
		else
		{
			// yippee this is a word..
			word += c;
		}
	}

	line += whitespace;
	line += word;
	BrokenText.push_back(line);
}


//! Sets the new caption of this element.
void CGUIStaticText::setText(const wchar_t* text)
{
	IGUIElement::setText(text);
	breakText();
}

void CGUIStaticText::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	breakText();
}


//! Returns the height of the text in pixels when it is drawn.
s32 CGUIStaticText::getTextHeight() const
{
	IGUISkin* skin = Environment->getSkin();

	if (!skin)
		return 0;

	IGUIFont* font = OverrideFont;
	if (!OverrideFont)
		font = skin->getFont();

	if (!font)
		return 0;

	s32 height = font->getDimension(L"A").Height + font->getKerningHeight();

	if (WordWrap)
		height *= BrokenText.size();

	return height;
}


s32 CGUIStaticText::getTextWidth() const
{
	IGUIFont * font = OverrideFont;

	if(!OverrideFont)
	{
		IGUISkin * skin = Environment->getSkin();
		if(skin)
			font = skin->getFont();
	}

	if(!font)
		return 0;

	if(WordWrap)
	{
		s32 widest = 0;

		for(u32 line = 0; line < BrokenText.size(); ++line)
		{
			s32 width = font->getDimension(BrokenText[line].c_str()).Width;

			if(width > widest)
				widest = width;
		}

		return widest;
	}
	else
	{
		return font->getDimension(Text.c_str()).Width;
	}
}


//! Writes attributes of the element.
//! Implement this to expose the attributes of your element for
//! scripting languages, editors, debuggers or xml serialization purposes.
void CGUIStaticText::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIStaticText::serializeAttributes(out,options);

	out->addBool	("Border",              Border);
	out->addBool	("OverrideColorEnabled",OverrideColorEnabled);
	out->addBool	("WordWrap",		WordWrap);
	out->addBool	("Background",          Background);
	out->addColor	("OverrideColor",       OverrideColor);
	out->addEnum	("HTextAlign",          HAlign, GUIAlignmentNames);
	out->addEnum	("VTextAlign",          VAlign, GUIAlignmentNames);

	// out->addFont ("OverrideFont",	OverrideFont);
}


//! Reads attributes of the element
void CGUIStaticText::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIStaticText::deserializeAttributes(in,options);

	Border = in->getAttributeAsBool("Border");
	OverrideColor = in->getAttributeAsColor("OverrideColor");

	enableOverrideColor(in->getAttributeAsBool("OverrideColorEnabled"));
	setWordWrap(in->getAttributeAsBool("WordWrap"));
	Background = in->getAttributeAsBool("Background");

	setTextAlignment( (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("HTextAlign", GUIAlignmentNames),
                      (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("VTextAlign", GUIAlignmentNames));

	// OverrideFont = in->getAttributeAsFont("OverrideFont");
}


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

