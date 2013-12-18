// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIComboBox.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IGUIButton.h"
#include "CGUIListBox.h"
#include "os.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIComboBox::CGUIComboBox(IGUIEnvironment* environment, IGUIElement* parent,
	s32 id, core::rect<s32> rectangle)
	: IGUIComboBox(environment, parent, id, rectangle),
	ListButton(0), ListBox(0), Selected(-1), HasFocus(false), LastFocus(0)
{
	#ifdef _DEBUG
	setDebugName("CGUICheckBox");
	#endif

	IGUISkin* skin = Environment->getSkin();

	s32 width = 15;
	if (skin)
		width = skin->getSize(EGDS_WINDOW_BUTTON_WIDTH);

	core::rect<s32> r;
	r.UpperLeftCorner.X = rectangle.getWidth() - width - 2;
	r.LowerRightCorner.X = rectangle.getWidth() - 2;
	
	r.UpperLeftCorner.Y = 2;
	r.LowerRightCorner.Y = rectangle.getHeight() - 2;

	ListButton = Environment->addButton(r, this, -1, L"");
	if (skin && skin->getSpriteBank())
	{
		ListButton->setSpriteBank(skin->getSpriteBank());
		ListButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_DOWN), skin->getColor(EGDC_WINDOW_SYMBOL));
		ListButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_DOWN), skin->getColor(EGDC_WINDOW_SYMBOL));
	}
	ListButton->setSubElement(true);
	ListButton->setTabStop(false);

	// this element can be tabbed to
	setTabStop(true);
	setTabOrder(-1);
}


//! Returns amount of items in box
u32 CGUIComboBox::getItemCount() const
{
	return Items.size();
}


//! returns string of an item. the idx may be a value from 0 to itemCount-1
const wchar_t* CGUIComboBox::getItem(u32 idx) const
{
	if (idx >= Items.size())
		return 0;

	return Items[idx].c_str();
}


//! Removes an item from the combo box.
void CGUIComboBox::removeItem(u32 idx)
{
	if (idx >= Items.size())
		return;

	if (Selected == (s32)idx)
		Selected = -1;

	Items.erase(idx);
}

//! Returns caption of this element.
const wchar_t* CGUIComboBox::getText() const
{
	return getItem(Selected);
}


//! adds an item and returns the index of it
u32 CGUIComboBox::addItem(const wchar_t* text)
{
	Items.push_back(core::stringw(text));

	if (Selected == -1)
		Selected = 0;

	return Items.size() - 1;
}



//! deletes all items in the combo box
void CGUIComboBox::clear()
{
	Items.clear();
	Selected = -1;
}



//! returns id of selected item. returns -1 if no item is selected.
s32 CGUIComboBox::getSelected() const
{
	return Selected;
}



//! sets the selected item. Set this to -1 if no item should be selected
void CGUIComboBox::setSelected(s32 idx)
{
	if (idx < -1 || idx >= (s32)Items.size())
		return;

	Selected = idx;
}

void CGUIComboBox::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();

	const s32 width = Environment->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);

	ListButton->setRelativePosition(core::rect<s32>(RelativeRect.getWidth() - width - 2, 2,
													RelativeRect.getWidth() - 2, RelativeRect.getHeight() - 2));
}


//! called if an event happened.
bool CGUIComboBox::OnEvent(const SEvent& event)
{
	switch(event.EventType)
	{

	case EET_KEY_INPUT_EVENT:
		if (ListBox && event.KeyInput.PressedDown && event.KeyInput.Key == KEY_ESCAPE)
		{
			// hide list box
			openCloseMenu();
			return true;
		}
		else
		if (event.KeyInput.Key == KEY_RETURN || event.KeyInput.Key == KEY_SPACE)
		{
			if (!event.KeyInput.PressedDown)
				openCloseMenu();

			ListButton->setPressed(ListBox == 0);

			return true;
		}
		else
		if (event.KeyInput.PressedDown)
		{
			s32 oldSelected = Selected;
			bool absorb = true;
			switch (event.KeyInput.Key)
			{
				case KEY_DOWN: 
					Selected += 1; 
					break;
				case KEY_UP: 
					Selected -= 1;
					break;
				case KEY_HOME:
				case KEY_PRIOR:
					Selected = 0;
					break;
				case KEY_END:
				case KEY_NEXT:
					Selected = (s32)Items.size()-1;
					break;
				default:
					absorb = false;
			}

			if (Selected <0)
				Selected = 0;

			if (Selected >= (s32)Items.size())
				Selected = (s32)Items.size() -1;

			if (Selected != oldSelected)
				sendSelectionChangedEvent();

			if (absorb)
				return true;
		}
		break;

	case EET_GUI_EVENT:

		switch(event.GUIEvent.EventType)
		{
		case EGET_ELEMENT_FOCUS_LOST:
			if (ListBox && 
				(Environment->hasFocus(ListBox) || ListBox->isMyChild(event.GUIEvent.Caller) ) && 
				event.GUIEvent.Element != this && 
				event.GUIEvent.Element != ListButton && 
				event.GUIEvent.Element != ListBox && 
				!ListBox->isMyChild(event.GUIEvent.Element))
			{
				openCloseMenu();
			}
			break;
		case EGET_BUTTON_CLICKED:
			if (event.GUIEvent.Caller == ListButton)
			{
				openCloseMenu();
				return true;
			}
			break;
		case EGET_LISTBOX_SELECTED_AGAIN:
		case EGET_LISTBOX_CHANGED:
			if (event.GUIEvent.Caller == ListBox)
			{
				Selected = ListBox->getSelected();
				if (Selected <0 || Selected >= (s32)Items.size())
					Selected = -1;
				openCloseMenu();

				sendSelectionChangedEvent();
			}
			return true;
		default:
			break;
		}
		break;
	case EET_MOUSE_INPUT_EVENT:

		switch(event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
			{
				core::position2d<s32> p(event.MouseInput.X, event.MouseInput.Y);
				
				// send to list box
				if (ListBox && ListBox->isPointInside(p) && ListBox->OnEvent(event))
					return true;

				return true;
			}
		case EMIE_LMOUSE_LEFT_UP:
			{
				core::position2d<s32> p(event.MouseInput.X, event.MouseInput.Y);

				// send to list box
				if (!(ListBox &&
						ListBox->getAbsolutePosition().isPointInside(p) &&
						ListBox->OnEvent(event)))
					openCloseMenu();
				return true;
			}
		case EMIE_MOUSE_WHEEL:
			{
				s32 oldSelected = Selected;
				Selected += (event.MouseInput.Wheel < 0) ? 1 : -1;

				if (Selected <0)
					Selected = 0;

				if (Selected >= (s32)Items.size())
					Selected = (s32)Items.size() -1;

				if (Selected != oldSelected)
					sendSelectionChangedEvent();
			}
		default:
			break;
		}
		break;
	default:
		break;
	}

	return Parent ? Parent->OnEvent(event) : false;
}



void CGUIComboBox::sendSelectionChangedEvent()
{
	if (Parent)
	{
		SEvent event;

		event.EventType = EET_GUI_EVENT;
		event.GUIEvent.Caller = this;
		event.GUIEvent.Element = 0;
		event.GUIEvent.EventType = EGET_COMBO_BOX_CHANGED;
		Parent->OnEvent(event);
	}
}



//! draws the element and its children
void CGUIComboBox::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	IGUIElement *currentFocus = Environment->getFocus();
	if (currentFocus != LastFocus)
	{
		HasFocus = currentFocus == this || isMyChild(currentFocus);
		LastFocus = currentFocus;
	}

	core::rect<s32> frameRect(AbsoluteRect);

	// draw the border

	skin->draw3DSunkenPane(this, skin->getColor(EGDC_3D_HIGH_LIGHT),
		true, true, frameRect, &AbsoluteClippingRect);

	// Draw text

	if (Selected != -1)
	{
		frameRect = AbsoluteRect;
		frameRect.UpperLeftCorner.X += 2;
		frameRect.UpperLeftCorner.Y += 2;
		frameRect.LowerRightCorner.X -= ListButton->getAbsolutePosition().getWidth() + 2;
		frameRect.LowerRightCorner.Y -= 2;
		if (HasFocus)
			skin->draw2DRectangle(this, skin->getColor(EGDC_HIGH_LIGHT), frameRect, &AbsoluteClippingRect);

		IGUIFont* font = skin->getFont();
		if (font)
			font->draw(Items[Selected].c_str(), frameRect, 
				skin->getColor(HasFocus ? EGDC_HIGH_LIGHT_TEXT : EGDC_BUTTON_TEXT),
				false, true, &AbsoluteClippingRect);
	}

	// draw buttons
	IGUIElement::draw();
}


void CGUIComboBox::openCloseMenu()
{
	if (ListBox)
	{
		// close list box
		Environment->setFocus(this);
		ListBox->remove();
		ListBox = 0;
	}
	else
	{
		if (Parent)
			Parent->bringToFront(this);

		IGUISkin* skin = Environment->getSkin();		
		s32 h = Items.size();

		if (h > 5)
			h = 5;
		if (h == 0)
			h = 1;

		IGUIFont* font = skin->getFont();
		if (font)
			h *= (font->getDimension(L"A").Height + 4);

		// open list box
		core::rect<s32> r(0, AbsoluteRect.getHeight(),
			AbsoluteRect.getWidth(), AbsoluteRect.getHeight() + h);

		ListBox = new CGUIListBox(Environment, this, -1, r, false, true, true);
		ListBox->setSubElement(true);
		ListBox->drop();

		for (s32 i=0; i<(s32)Items.size(); ++i)
			ListBox->addItem(Items[i].c_str());

		ListBox->setSelected(Selected);

		// set focus
		Environment->setFocus(ListBox);
	}
}

//! Writes attributes of the element.
void CGUIComboBox::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIComboBox::serializeAttributes(out,options);

	out->addInt	("Selected",	Selected );
	out->addArray	("Items",	Items);
}

//! Reads attributes of the element
void CGUIComboBox::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	Items = in->getAttributeAsArray("Items");
	IGUIComboBox::deserializeAttributes(in,options);

	setSelected(in->getAttributeAsInt("Selected"));
}

} // end namespace gui
} // end namespace irr


#endif // _IRR_COMPILE_WITH_GUI_

