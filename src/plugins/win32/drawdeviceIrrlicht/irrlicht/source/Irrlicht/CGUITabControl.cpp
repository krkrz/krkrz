// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUITabControl.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "rect.h"
#include "os.h"

namespace irr
{
namespace gui
{

// ------------------------------------------------------------------
// Tab
// ------------------------------------------------------------------

//! constructor
CGUITab::CGUITab(s32 number, IGUIEnvironment* environment,
	IGUIElement* parent, const core::rect<s32>& rectangle, 
	s32 id)
	: IGUITab(environment, parent, id, rectangle), Number(number),
		DrawBackground(false), BackColor(0,0,0,0)
{
	#ifdef _DEBUG
	setDebugName("CGUITab");
	#endif
}


//! Returns number of tab in tabcontrol. Can be accessed
//! later IGUITabControl::getTab() by this number.
s32 CGUITab::getNumber() const
{
	return Number;
}


//! Sets the number
void CGUITab::setNumber(s32 n)
{
	Number = n;
}


//! draws the element and its children
void CGUITab::draw()
{
	if (!IsVisible)
		return;

	IGUISkin *skin = Environment->getSkin();

	if (skin && DrawBackground)
		skin->draw2DRectangle(this, BackColor, AbsoluteRect, &AbsoluteClippingRect);

	IGUIElement::draw();
}


//! sets if the tab should draw its background
void CGUITab::setDrawBackground(bool draw)
{
	DrawBackground = draw;
}


//! sets the color of the background, if it should be drawn.
void CGUITab::setBackgroundColor(video::SColor c)
{
	BackColor = c;
}


//! returns true if the tab is drawing its background, false if not
bool CGUITab::isDrawingBackground() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return DrawBackground;
}


//! returns the color of the background
video::SColor CGUITab::getBackgroundColor() const
{
	return BackColor;
}


//! Writes attributes of the element.
void CGUITab::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUITab::serializeAttributes(out,options);

	out->addInt	("TabNumber",		Number);
	out->addBool	("DrawBackground",	DrawBackground);
	out->addColor	("BackColor",		BackColor);

}


//! Reads attributes of the element
void CGUITab::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUITab::deserializeAttributes(in,options);

	setNumber(in->getAttributeAsInt("TabNumber"));
	setDrawBackground(in->getAttributeAsBool("DrawBackground"));
	setBackgroundColor(in->getAttributeAsColor("BackColor"));

	if (Parent && Parent->getType() == EGUIET_TAB_CONTROL)
	{
		((CGUITabControl*)Parent)->addTab(this);
		if (isVisible())
			((CGUITabControl*)Parent)->setActiveTab(this);
	}
}


// ------------------------------------------------------------------
// Tabcontrol
// ------------------------------------------------------------------

//! destructor
CGUITabControl::CGUITabControl(IGUIEnvironment* environment,
	IGUIElement* parent, const core::rect<s32>& rectangle, 
	bool fillbackground, bool border, s32 id)
	: IGUITabControl(environment, parent, id, rectangle), ActiveTab(-1),
	Border(border), FillBackground(fillbackground)
{
	#ifdef _DEBUG
	setDebugName("CGUITabControl");
	#endif
}


//! destructor
CGUITabControl::~CGUITabControl()
{
	for (u32 i=0; i<Tabs.size(); ++i)
	{
		if (Tabs[i])
			Tabs[i]->drop();
	}
}


//! Adds a tab
IGUITab* CGUITabControl::addTab(const wchar_t* caption, s32 id)
{
	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return 0;

	s32 tabheight = skin->getSize(gui::EGDS_BUTTON_HEIGHT) + 2;
	core::rect<s32> r(1,tabheight,
		AbsoluteRect.getWidth()-1,
		AbsoluteRect.getHeight()-1);

	CGUITab* tab = new CGUITab(Tabs.size(), Environment, this, r, id);
	tab->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);

	tab->setText(caption);
	tab->setVisible(false);
	Tabs.push_back(tab);

	if (ActiveTab == -1)
	{
		ActiveTab = 0;
		tab->setVisible(true);
	}

	return tab;
}


//! adds a tab which has been created elsewhere
void CGUITabControl::addTab(CGUITab* tab)
{
	if (!tab)
		return;

	// check if its already added
	for (u32 i=0; i < Tabs.size(); ++i)
	{
		if (Tabs[i] == tab)
			return;
	}

	tab->grab();

	if (tab->getNumber() == -1)
		tab->setNumber((s32)Tabs.size());

	while (tab->getNumber() >= (s32)Tabs.size())
		Tabs.push_back(0);

	if (Tabs[tab->getNumber()])
	{
		Tabs.push_back(Tabs[tab->getNumber()]);
		Tabs[Tabs.size()-1]->setNumber(Tabs.size());
	}
	Tabs[tab->getNumber()] = tab;

	if (ActiveTab == -1)
		ActiveTab = tab->getNumber();


	if (tab->getNumber() == ActiveTab)
	{
		setActiveTab(ActiveTab);
	}
}


//! Returns amount of tabs in the tabcontrol
s32 CGUITabControl::getTabCount() const
{
	return Tabs.size();
}


//! Returns a tab based on zero based index
IGUITab* CGUITabControl::getTab(s32 idx) const
{
	if ((u32)idx >= Tabs.size())
		return 0;

	return Tabs[idx];
}


//! called if an event happened.
bool CGUITabControl::OnEvent(const SEvent& event)
{
	if (!IsEnabled)
		return Parent ? Parent->OnEvent(event) : false;

	switch(event.EventType)
	{
	case EET_MOUSE_INPUT_EVENT:
		switch(event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
			Environment->setFocus(this);
			return true;
		case EMIE_LMOUSE_LEFT_UP:
			Environment->removeFocus(this);
			selectTab(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y));
			return true;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return Parent ? Parent->OnEvent(event) : false;
}


void CGUITabControl::selectTab(core::position2d<s32> p)
{
	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	s32 tabheight = skin->getSize(gui::EGDS_BUTTON_HEIGHT);
	frameRect.UpperLeftCorner.Y += 2;
	frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + tabheight;
	s32 pos = frameRect.UpperLeftCorner.X + 2;

	for (u32 i=0; i<Tabs.size(); ++i)
	{
		// get Text
		const wchar_t* text = 0;
		if (Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = 20;
		if (font)
			len += font->getDimension(text).Width;

		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;
		pos += len;

		if (frameRect.isPointInside(p))
		{
			setActiveTab(i);
			return;
		}
	}
}


//! draws the element and its children
void CGUITabControl::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;

	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	if (Tabs.empty())
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_HIGH_LIGHT),
		frameRect, &AbsoluteClippingRect);

	if (!font)
		return;

	s32 tabheight = skin->getSize(gui::EGDS_BUTTON_HEIGHT);
	frameRect.UpperLeftCorner.Y += 2;
	frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + tabheight;
	core::rect<s32> tr;
	s32 pos = frameRect.UpperLeftCorner.X + 2;

	// left and right pos of the active tab
	s32 left = 0;
	s32 right = 0;
	const wchar_t* activetext = 0;
	
	for (u32 i=0; i<Tabs.size(); ++i)
	{
		// get Text
		const wchar_t* text = 0;
		if (Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = font->getDimension(text).Width + 20;
		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;
		pos += len;

		if ((s32)i == ActiveTab)
		{
			left = frameRect.UpperLeftCorner.X;
			right = frameRect.LowerRightCorner.X;
			activetext = text;
		}
		else
		{
			skin->draw3DTabButton(this, false, frameRect, &AbsoluteClippingRect);
					
			// draw text
			font->draw(text, frameRect, skin->getColor(EGDC_BUTTON_TEXT),
				true, true, &AbsoluteClippingRect);
		}
	}

	// draw active tab
	if (left != 0 && right != 0)
	{
		frameRect.UpperLeftCorner.X = left-2;
		frameRect.LowerRightCorner.X = right+2;
		frameRect.UpperLeftCorner.Y -= 2;

		skin->draw3DTabButton(this, true, frameRect, &AbsoluteClippingRect);
		
		// draw text
		font->draw(activetext, frameRect, skin->getColor(EGDC_BUTTON_TEXT),
			true, true, &AbsoluteClippingRect);

		// draw upper highlight frame
		tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
		tr.LowerRightCorner.X = left - 1;
		tr.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 1;
		tr.LowerRightCorner.Y = frameRect.LowerRightCorner.Y;
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);

		tr.UpperLeftCorner.X = right;
		tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);
	}

	skin->draw3DTabBody(this, Border, FillBackground, AbsoluteRect, &AbsoluteClippingRect);

	IGUIElement::draw();
}


//! Returns which tab is currently active
s32 CGUITabControl::getActiveTab() const
{
	return ActiveTab;
}


bool CGUITabControl::setActiveTab(IGUIElement *tab)
{
	for (s32 i=0; i<(s32)Tabs.size(); ++i)
		if (Tabs[i] == tab)
			return setActiveTab(i);
	return false;
}


//! Brings a tab to front.
bool CGUITabControl::setActiveTab(s32 idx)
{
	if ((u32)idx >= Tabs.size())
		return false;

	bool changed = (ActiveTab != idx);

	ActiveTab = idx;

	for (s32 i=0; i<(s32)Tabs.size(); ++i)
		if (Tabs[i])
			Tabs[i]->setVisible( i == ActiveTab );

	if (changed)
	{
		SEvent event;
		event.EventType = EET_GUI_EVENT;
		event.GUIEvent.Caller = this;
		event.GUIEvent.Element = 0;
		event.GUIEvent.EventType = EGET_TAB_CHANGED;
		Parent->OnEvent(event);		
	}

	return true;
}


//! Removes a child.
void CGUITabControl::removeChild(IGUIElement* child)
{
	bool isTab = false;

	u32 i=0;
	// check if it is a tab
	while (i<Tabs.size())
	{
		if (Tabs[i] == child)
		{
			Tabs[i]->drop();
			Tabs.erase(i);
			isTab = true;
		}
		else
			++i;
	}

	// reassign numbers
	if (isTab)
	{
		for (i=0; i<Tabs.size(); ++i)
			if (Tabs[i])
				Tabs[i]->setNumber(i);
	}

	// remove real element
	IGUIElement::removeChild(child);
}


//! Writes attributes of the element.
void CGUITabControl::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUITabControl::serializeAttributes(out,options);

	out->addInt("ActiveTab",	ActiveTab);
	out->addBool("Border",		Border);
	out->addBool("FillBackground",	FillBackground);
}


//! Reads attributes of the element
void CGUITabControl::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	Border		= in->getAttributeAsBool("Border");
	FillBackground  = in->getAttributeAsBool("FillBackground");

	ActiveTab = -1;

	IGUITabControl::deserializeAttributes(in,options);

	setActiveTab(in->getAttributeAsInt("ActiveTab"));
}


} // end namespace irr
} // end namespace gui

#endif // _IRR_COMPILE_WITH_GUI_

