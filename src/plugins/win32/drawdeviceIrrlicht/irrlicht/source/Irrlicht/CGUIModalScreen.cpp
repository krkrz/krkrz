// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIModalScreen.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIEnvironment.h"
#include "os.h"
#include "IVideoDriver.h"
#include "IGUISkin.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIModalScreen::CGUIModalScreen(IGUIEnvironment* environment, IGUIElement* parent, s32 id)
: IGUIElement(EGUIET_MODAL_SCREEN, environment, parent, id, parent->getAbsolutePosition()),
	MouseDownTime(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIModalScreen");
	#endif
	setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	
	// this element is a tab group
	setTabGroup(true);
}


//! called if an event happened.
bool CGUIModalScreen::OnEvent(const SEvent& event)
{
    switch(event.EventType)
	{
	case EET_GUI_EVENT:
		switch(event.GUIEvent.EventType)
		{
		case EGET_ELEMENT_FOCUSED:
			// only children are allowed the focus
			if (event.GUIEvent.Caller != this && !isMyChild(event.GUIEvent.Caller))
				Environment->setFocus(this);
			return false;
		case EGET_ELEMENT_FOCUS_LOST:
			// only children are allowed the focus
			if (!(isMyChild(event.GUIEvent.Element) || event.GUIEvent.Element == this))
			{
				MouseDownTime = os::Timer::getTime();
				return true;
			}
			else
			{
				return IGUIElement::OnEvent(event);
			}
		case EGET_ELEMENT_CLOSED:
			// do not interfere with children being removed
			return IGUIElement::OnEvent(event);
		default:
			break;
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			MouseDownTime = os::Timer::getTime();
        }
	default:
		break;
	}
	
	IGUIElement::OnEvent(event);

	return true; // absorb everything else
}


//! draws the element and its children
void CGUIModalScreen::draw()
{
	IGUISkin *skin = Environment->getSkin();

	if (!skin)
		return;

	u32 now = os::Timer::getTime();
	if (now - MouseDownTime < 300 && (now / 70)%2)
	{
		core::list<IGUIElement*>::Iterator it = Children.begin();
		core::rect<s32> r;
		video::SColor c = Environment->getSkin()->getColor(gui::EGDC_3D_HIGH_LIGHT);

		for (; it != Children.end(); ++it)
		{
			r = (*it)->getAbsolutePosition();
			r.LowerRightCorner.X += 1;
			r.LowerRightCorner.Y += 1;
			r.UpperLeftCorner.X -= 1;
			r.UpperLeftCorner.Y -= 1;

			skin->draw2DRectangle(this, c, r, &AbsoluteClippingRect);
		}
	}

	IGUIElement::draw();
}



//! Removes a child.
void CGUIModalScreen::removeChild(IGUIElement* child)
{
	IGUIElement::removeChild(child);

	if (Children.empty())
		remove();
}

//! adds a child
void CGUIModalScreen::addChild(IGUIElement* child)
{
	IGUIElement::addChild(child);
	Environment->setFocus(child);
}


void CGUIModalScreen::updateAbsolutePosition()
{
	core::rect<s32> parentRect(0,0,0,0);

	if (Parent)
	{
		parentRect = Parent->getAbsolutePosition();
		RelativeRect.UpperLeftCorner.X = 0;
		RelativeRect.UpperLeftCorner.Y = 0;
		RelativeRect.LowerRightCorner.X = parentRect.getWidth();
		RelativeRect.LowerRightCorner.Y = parentRect.getHeight();
	}

	IGUIElement::updateAbsolutePosition();
}

//! Writes attributes of the element.
void CGUIModalScreen::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	// these don't get serialized, their status is added to their children.
}

//! Reads attributes of the element
void CGUIModalScreen::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	// these don't get deserialized. children create them if required
}


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_
