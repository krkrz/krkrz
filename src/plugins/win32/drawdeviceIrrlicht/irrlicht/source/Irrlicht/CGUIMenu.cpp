// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIMenu.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIFont.h"

#include "os.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIMenu::CGUIMenu(IGUIEnvironment* environment, IGUIElement* parent,
		 s32 id, core::rect<s32> rectangle)
		 : CGUIContextMenu(environment, parent, id, rectangle, false, true)
{
	#ifdef _DEBUG
	setDebugName("CGUIMenu");
	#endif

	Type = EGUIET_MENU;

	setNotClipped(false);

	recalculateSize();
}


//! draws the element and its children
void CGUIMenu::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont(EGDF_MENU);
	
	if (font != LastFont)
	{
		if (LastFont)
			LastFont->drop();
		LastFont = font;
		if (LastFont)
			LastFont->grab();

		recalculateSize();
	}

	core::rect<s32> rect = AbsoluteRect;

	// draw frame

	skin->draw3DToolBar(this, rect, &AbsoluteClippingRect);

	// loop through all menu items

	rect = AbsoluteRect;

	for (s32 i=0; i<(s32)Items.size(); ++i)
	{
		if (!Items[i].IsSeparator)
		{
			rect = getRect(Items[i], AbsoluteRect);

			// draw highlighted
			if (i == HighLighted && Items[i].Enabled)
			{
				skin->draw3DSunkenPane(this, skin->getColor(EGDC_3D_DARK_SHADOW),
					true, true, rect, &AbsoluteClippingRect);
			}
			// draw text

			EGUI_DEFAULT_COLOR c = EGDC_BUTTON_TEXT;

			if (i == HighLighted)
				c = EGDC_HIGH_LIGHT_TEXT;

			if (!Items[i].Enabled)
				c = EGDC_GRAY_TEXT;

			if (font)
				font->draw(Items[i].Text.c_str(), rect, 
					skin->getColor(c), true, true, &AbsoluteClippingRect);
		}
	}

	IGUIElement::draw();
}


//! called if an event happened.
bool CGUIMenu::OnEvent(const SEvent& event)
{
	if (!IsEnabled)
		return Parent ? Parent->OnEvent(event) : false;

	switch(event.EventType)
	{
	case EET_GUI_EVENT:
		switch(event.GUIEvent.EventType)
		{
		case gui::EGET_ELEMENT_FOCUS_LOST:
			if (event.GUIEvent.Caller == this && !isMyChild(event.GUIEvent.Element))
			{
				closeAllSubMenus();
				HighLighted = -1;
			}
			break;
		case gui::EGET_ELEMENT_FOCUSED:
			if (event.GUIEvent.Caller == this && Parent)
			{
				Parent->bringToFront(this);
			}
			break;
		default:
			break;
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		switch(event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
		{
			if (!Environment->hasFocus(this))
			{
				Environment->setFocus(this);
			}

			if (Parent)
				Parent->bringToFront(this); 

			core::position2d<s32> p(event.MouseInput.X, event.MouseInput.Y);
			bool shouldCloseSubMenu = hasOpenSubMenu();
			if (!AbsoluteClippingRect.isPointInside(p))
			{
				shouldCloseSubMenu = false;
				s32 t = sendClick(p);
				if ((t==0 || t==1) && Environment->hasFocus(this))
					Environment->removeFocus(this);
			}
			highlight(core::position2d<s32>(event.MouseInput.X,	event.MouseInput.Y), true);
			if ( shouldCloseSubMenu )
				closeAllSubMenus();
			
			return true;
		}
		case EMIE_MOUSE_MOVED:
			if (Environment->hasFocus(this))
				highlight(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y), hasOpenSubMenu());
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


void CGUIMenu::recalculateSize()
{
	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont(EGDF_MENU);

	if (!font)
	{
		if (Parent && skin)
			RelativeRect = core::rect<s32>(0,0,
					Parent->getAbsolutePosition().LowerRightCorner.X,
					skin->getSize(EGDS_MENU_HEIGHT));
		return;
	}

	core::rect<s32> rect;
	rect.UpperLeftCorner.X = 0;
	rect.UpperLeftCorner.Y = 0;
	s32 height = font->getDimension(L"A").Height + 5;
	//if (skin && height < skin->getSize ( EGDS_MENU_HEIGHT ))
	//	height = skin->getSize(EGDS_MENU_HEIGHT);
	s32 width = 0;
	s32 i;

	for (i=0; i<(s32)Items.size(); ++i)
	{
		if (Items[i].IsSeparator)
		{
			Items[i].Dim.Width = 0;
			Items[i].Dim.Height = height;
		}
		else
		{
			Items[i].Dim = font->getDimension(Items[i].Text.c_str());
			Items[i].Dim.Width += 20;
		}

		Items[i].PosY = width;
		width += Items[i].Dim.Width;
	}

	if (Parent)
		width = Parent->getAbsolutePosition().getWidth();

	rect.LowerRightCorner.X = width;
	rect.LowerRightCorner.Y = height;

	setRelativePosition(rect);

	// recalculate submenus
	for (i=0; i<(s32)Items.size(); ++i)
		if (Items[i].SubMenu)
		{
			// move submenu
			s32 w = Items[i].SubMenu->getAbsolutePosition().getWidth();
			s32 h = Items[i].SubMenu->getAbsolutePosition().getHeight();

			Items[i].SubMenu->setRelativePosition(
				core::rect<s32>(Items[i].PosY, height ,
					Items[i].PosY+w-5, height+h));
		}
}


//! returns the item highlight-area
core::rect<s32> CGUIMenu::getHRect(const SItem& i, const core::rect<s32>& absolute) const
{
	core::rect<s32> r = absolute;
	r.UpperLeftCorner.X += i.PosY;
	r.LowerRightCorner.X = r.UpperLeftCorner.X + i.Dim.Width;
	return r;
}

//! Gets drawing rect of Item
core::rect<s32> CGUIMenu::getRect(const SItem& i, const core::rect<s32>& absolute) const
{
	return getHRect(i, absolute);
}

void CGUIMenu::updateAbsolutePosition()
{
	if (Parent)
		DesiredRect.LowerRightCorner.X = Parent->getAbsolutePosition().getWidth();

	IGUIElement::updateAbsolutePosition();
}


} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_GUI_

