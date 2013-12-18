// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIScrollBar.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "CGUIButton.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"
#include "os.h"

namespace irr
{
namespace gui
{


//! constructor
CGUIScrollBar::CGUIScrollBar(bool horizontal, IGUIEnvironment* environment,
				IGUIElement* parent, s32 id,
				core::rect<s32> rectangle, bool noclip)
	: IGUIScrollBar(environment, parent, id, rectangle), UpButton(0),
	DownButton(0), Dragging(false), Horizontal(horizontal),
	DraggedBySlider(false), TrayClick(false), Pos(0), DrawPos(0),
	DrawHeight(0), Max(100), SmallStep(10), LargeStep(50), DesiredPos(0),
	LastChange(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIScrollBar");
	#endif

	refreshControls();

	setNotClipped(noclip);

	// this element can be tabbed to
	setTabStop(true);
	setTabOrder(-1);

	setPos(0);
}


//! destructor
CGUIScrollBar::~CGUIScrollBar()
{
	if (UpButton)
		UpButton->drop();

	if (DownButton)
		DownButton->drop();
}


//! called if an event happened.
bool CGUIScrollBar::OnEvent(const SEvent& event)
{
	switch(event.EventType)
	{
	case EET_KEY_INPUT_EVENT:
		if (event.KeyInput.PressedDown)
		{
			const s32 oldPos = Pos;
			bool absorb = true;
			switch (event.KeyInput.Key)
			{
			case KEY_LEFT:
			case KEY_UP:
				setPos(Pos-SmallStep);
				break;
			case KEY_RIGHT:
			case KEY_DOWN:
				setPos(Pos+SmallStep);
				break;
			case KEY_HOME:
				setPos(0);
				break;
			case KEY_PRIOR:
				setPos(Pos-LargeStep);
				break;
			case KEY_END:
				setPos(Max);
				break;
			case KEY_NEXT:
				setPos(Pos+LargeStep);
				break;
			default:
				absorb = false;
			}

			if (Pos != oldPos)
			{
				SEvent newEvent;
				newEvent.EventType = EET_GUI_EVENT;
				newEvent.GUIEvent.Caller = this;
				newEvent.GUIEvent.Element = 0;
				newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
				Parent->OnEvent(newEvent);
			}
			if (absorb)
				return true;
		}
		break;
	case EET_GUI_EVENT:
		if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
		{
			if (event.GUIEvent.Caller == UpButton)
				setPos(Pos-SmallStep);
			else
			if (event.GUIEvent.Caller == DownButton)
				setPos(Pos+SmallStep);

			SEvent newEvent;
			newEvent.EventType = EET_GUI_EVENT;
			newEvent.GUIEvent.Caller = this;
			newEvent.GUIEvent.Element = 0;
			newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
			Parent->OnEvent(newEvent);

			return true;
		}
		else
		if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
		{
			if (event.GUIEvent.Caller == this)
				Dragging = false;
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		switch(event.MouseInput.Event)
		{
		case EMIE_MOUSE_WHEEL:
			if (Environment->hasFocus(this))
			{ // thanks to a bug report by REAPER
				setPos(getPos() + (s32)event.MouseInput.Wheel* -SmallStep);
				SEvent newEvent;
				newEvent.EventType = EET_GUI_EVENT;
				newEvent.GUIEvent.Caller = this;
				newEvent.GUIEvent.Element = 0;
				newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
				Parent->OnEvent(newEvent);
				return true;
			}
			break;
		case EMIE_LMOUSE_PRESSED_DOWN:
		{
			if (AbsoluteClippingRect.isPointInside(core::position2di(event.MouseInput.X, event.MouseInput.Y)))
			{
				Dragging = true;
				DraggedBySlider = SliderRect.isPointInside(core::position2di(event.MouseInput.X, event.MouseInput.Y));
				TrayClick = !DraggedBySlider;
				DesiredPos = getPosFromMousePos(event.MouseInput.X, event.MouseInput.Y);
				return true;
			}
			break;
		}
		case EMIE_LMOUSE_LEFT_UP:
		case EMIE_MOUSE_MOVED:
			if (Dragging)
			{
				if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
					Dragging = false;

				const s32 newPos = getPosFromMousePos(event.MouseInput.X, event.MouseInput.Y);
				const s32 oldPos = Pos;

				if (!DraggedBySlider)
				{
					if (AbsoluteClippingRect.isPointInside(core::position2di(event.MouseInput.X, event.MouseInput.Y)))
					{
						DraggedBySlider = SliderRect.isPointInside(core::position2di(event.MouseInput.X, event.MouseInput.Y));
						TrayClick = !DraggedBySlider;
					}
					else
					{
						TrayClick = false;
						if (event.MouseInput.Event == EMIE_MOUSE_MOVED)
							return true;
					}
				}
				
				if (DraggedBySlider)
				{
					setPos(newPos);
				}
				else
				{
					DesiredPos = newPos;
				}

				if (Pos != oldPos && Parent)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				return true;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return IGUIElement::OnEvent(event);
}

//! draws the element and its children
void CGUIScrollBar::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;

	u32 now = os::Timer::getRealTime();

	if (Dragging && !DraggedBySlider && TrayClick && now > LastChange + 200)
	{
		LastChange = now;

		const s32 oldPos = Pos;

		if (DesiredPos >= Pos + LargeStep)
			setPos(Pos + LargeStep);
		else 
		if (DesiredPos <= Pos - LargeStep)
			setPos(Pos - LargeStep);
		else 
		if (DesiredPos >= Pos - LargeStep && DesiredPos <= Pos + LargeStep)
			setPos(DesiredPos);

		if (Pos != oldPos && Parent)
		{
			SEvent newEvent;
			newEvent.EventType = EET_GUI_EVENT;
			newEvent.GUIEvent.Caller = this;
			newEvent.GUIEvent.Element = 0;
			newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
			Parent->OnEvent(newEvent);
		}
	}

	SliderRect = AbsoluteRect;

	// draws the background
	skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);

	if (Max!=0)
	{
		// recalculate slider rectangle
		if (Horizontal)
		{
			SliderRect.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X + DrawPos + RelativeRect.getHeight() - DrawHeight/2;
			SliderRect.LowerRightCorner.X = SliderRect.UpperLeftCorner.X + DrawHeight;
		}
		else
		{
			SliderRect.UpperLeftCorner.Y = AbsoluteRect.UpperLeftCorner.Y + DrawPos + RelativeRect.getWidth() - DrawHeight/2;
			SliderRect.LowerRightCorner.Y = SliderRect.UpperLeftCorner.Y + DrawHeight;
		}

		skin->draw3DButtonPaneStandard(this, SliderRect, &AbsoluteClippingRect);
	}

	// draw buttons
	IGUIElement::draw();
}

void CGUIScrollBar::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	// todo: properly resize
	refreshControls();

	if (Horizontal)
	{
		const f32 f = (RelativeRect.getWidth() - ((f32)RelativeRect.getHeight()*3.0f)) / (f32)Max;
		DrawPos = (s32)((Pos * f) + ((f32)RelativeRect.getHeight() * 0.5f));
		DrawHeight = RelativeRect.getHeight();
	}
	else
	{
		f32 f = 0.0f;
		if (Max != 0)
			f = (RelativeRect.getHeight() - ((f32)RelativeRect.getWidth()*3.0f)) / (f32)Max;

		DrawPos = (s32)((Pos * f) + ((f32)RelativeRect.getWidth() * 0.5f));
		DrawHeight = RelativeRect.getWidth();
	}
}

s32 CGUIScrollBar::getPosFromMousePos(s32 x, s32 y) const
{
	if (Horizontal)
	{
		const f32 w = RelativeRect.getWidth() - f32(RelativeRect.getHeight())*3.0f;
		const f32 p = x - AbsoluteRect.UpperLeftCorner.X - RelativeRect.getHeight()*1.5f;
		return s32( p/w * f32(Max) );
	}
	else
	{
		const f32 h = RelativeRect.getHeight() - f32(RelativeRect.getWidth())*3.0f;
		const f32 p = y - AbsoluteRect.UpperLeftCorner.Y - RelativeRect.getWidth()*1.5f;
		return s32( p/h * f32(Max) );
	}
}



//! sets the position of the scrollbar
void CGUIScrollBar::setPos(s32 pos)
{
	if (pos < 0)
		Pos = 0;
	else if (pos > Max)
		Pos = Max;
	else
		Pos = pos;

	if (Horizontal)
	{
		const f32 f = (RelativeRect.getWidth() - ((f32)RelativeRect.getHeight()*3.0f)) / (f32)Max;
		DrawPos = (s32)((Pos * f) + ((f32)RelativeRect.getHeight() * 0.5f));
		DrawHeight = RelativeRect.getHeight();
	}
	else
	{
		f32 f = 0.0f;
		if (Max != 0)
			f = (RelativeRect.getHeight() - ((f32)RelativeRect.getWidth()*3.0f)) / (f32)Max;

		DrawPos = (s32)((Pos * f) + ((f32)RelativeRect.getWidth() * 0.5f));
		DrawHeight = RelativeRect.getWidth();
	}
}


//! gets the small step value
s32 CGUIScrollBar::getSmallStep() const
{
	return SmallStep;
}


//! sets the small step value
void CGUIScrollBar::setSmallStep(s32 step)
{
	if (step > 0)
		SmallStep = step;
	else
		SmallStep = 10;
}

//! gets the small step value
s32 CGUIScrollBar::getLargeStep() const
{
	return LargeStep;
}


//! sets the small step value
void CGUIScrollBar::setLargeStep(s32 step)
{
	if (step > 0)
		LargeStep = step;
	else
		LargeStep = 50;
}



//! gets the maximum value of the scrollbar.
s32 CGUIScrollBar::getMax() const
{
	return Max;
}


//! sets the maximum value of the scrollbar.
void CGUIScrollBar::setMax(s32 max)
{
	if (max > 0)
		Max = max;
	else
		Max = 0;

	bool enable = (Max != 0);
	UpButton->setEnabled(enable);
	DownButton->setEnabled(enable);
	setPos(Pos);
}


//! gets the current position of the scrollbar
s32 CGUIScrollBar::getPos() const
{
	return Pos;
}


//! refreshes the position and text on child buttons
void CGUIScrollBar::refreshControls()
{
	video::SColor color(255,255,255,255);

	IGUISkin* skin = Environment->getSkin();
	IGUISpriteBank* sprites = 0;

	if (skin)
	{
		sprites = skin->getSpriteBank();
		color = skin->getColor(EGDC_WINDOW_SYMBOL);
	}

	if (Horizontal)
	{
		s32 h = RelativeRect.getHeight();
		if (!UpButton)
		{
			UpButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,0, h, h), NoClip);
			UpButton->setSubElement(true);
			UpButton->setTabStop(false);
		}
		if (sprites)
		{
			UpButton->setSpriteBank(sprites);
			UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_LEFT), color);
			UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_LEFT), color);
		}
		UpButton->setRelativePosition(core::rect<s32>(0,0, h, h));
		UpButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
		if (!DownButton)
		{
			DownButton = new CGUIButton(Environment, this, -1, core::rect<s32>(RelativeRect.getWidth()-h, 0, RelativeRect.getWidth(), h), NoClip);
			DownButton->setSubElement(true);
			DownButton->setTabStop(false);
		}
		if (sprites)
		{
			DownButton->setSpriteBank(sprites);
			DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_RIGHT), color);
			DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_RIGHT), color);
		}
		DownButton->setRelativePosition(core::rect<s32>(RelativeRect.getWidth()-h, 0, RelativeRect.getWidth(), h));
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	}
	else
	{
		s32 w = RelativeRect.getWidth();
		if (!UpButton)
		{
			UpButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,0, w, w), NoClip);
			UpButton->setSubElement(true);
			UpButton->setTabStop(false);
		}
		if (sprites)
		{
			UpButton->setSpriteBank(sprites);
			UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_UP), color);
			UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_UP), color);
		}
		UpButton->setRelativePosition(core::rect<s32>(0,0, w, w));
		UpButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		if (!DownButton)
		{
			DownButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,RelativeRect.getHeight()-w, w, RelativeRect.getHeight()), NoClip);
			DownButton->setSubElement(true);
			DownButton->setTabStop(false);
		}
		if (sprites)
		{
			DownButton->setSpriteBank(sprites);
			DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_DOWN), color);
			DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_DOWN), color);
		}
		DownButton->setRelativePosition(core::rect<s32>(0,RelativeRect.getHeight()-w, w, RelativeRect.getHeight()));
		DownButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
	}
}


//! Writes attributes of the element.
void CGUIScrollBar::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIScrollBar::serializeAttributes(out,options);

	out->addBool("Horizontal",	Horizontal);
	out->addInt ("Value",		Pos);
	out->addInt ("Max",		Max);
	out->addInt ("SmallStep",	SmallStep);
	out->addInt ("LargeStep",	LargeStep);
}


//! Reads attributes of the element
void CGUIScrollBar::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIScrollBar::deserializeAttributes(in,options);

	Horizontal = in->getAttributeAsBool("Horizontal");
	setMax(in->getAttributeAsInt("Max"));
	setPos(in->getAttributeAsInt("Value"));
	setSmallStep(in->getAttributeAsInt("SmallStep"));
	setLargeStep(in->getAttributeAsInt("LargeStep"));

	refreshControls();
}


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

