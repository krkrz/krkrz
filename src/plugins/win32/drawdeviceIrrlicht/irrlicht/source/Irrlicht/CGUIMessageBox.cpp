// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIMessageBox.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIButton.h"
#include "IGUIFont.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIMessageBox::CGUIMessageBox(IGUIEnvironment* environment, const wchar_t* caption,
	const wchar_t* text, s32 flags,
	IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: CGUIWindow(environment, parent, id, rectangle),
	OkButton(0), CancelButton(0), YesButton(0), NoButton(0), StaticText(0),
	Flags(flags), MessageText(text), Pressed(false)
{
	#ifdef _DEBUG
	setDebugName("CGUIMessageBox");
	#endif

	// set element type
	Type = EGUIET_MESSAGE_BOX;

	// remove focus
	Environment->setFocus(0);

	// remove buttons

	getMaximizeButton()->remove();
	getMinimizeButton()->remove();

	if (caption)
		setText(caption);

	Environment->setFocus(this);

	refreshControls();
}


//! destructor
CGUIMessageBox::~CGUIMessageBox()
{
	if (StaticText)
		StaticText->drop();

	if (OkButton)
		OkButton->drop();

	if (CancelButton)
		CancelButton->drop();

	if (YesButton)
		YesButton->drop();

	if (NoButton)
		NoButton->drop();
}


void CGUIMessageBox::refreshControls()
{
	const IGUISkin* skin = Environment->getSkin();
	IGUIElement* focusMe = 0;

	const s32 buttonHeight = skin->getSize(EGDS_BUTTON_HEIGHT);
	const s32 buttonWidth = skin->getSize(EGDS_BUTTON_WIDTH);
	const s32 titleHeight = skin->getSize(EGDS_WINDOW_BUTTON_WIDTH)+2;
	const s32 buttonDistance = skin->getSize(EGDS_WINDOW_BUTTON_WIDTH);

	// add static multiline text

	core::dimension2d<s32> dim(AbsoluteClippingRect.getWidth() - buttonWidth,
		AbsoluteClippingRect.getHeight() - (buttonHeight * 3));
	const core::position2d<s32> pos((AbsoluteClippingRect.getWidth() - dim.Width) / 2,
		buttonHeight / 2 + titleHeight);

	if (!StaticText)
	{
		StaticText = Environment->addStaticText(MessageText.c_str(),
						core::rect<s32>(pos, dim), false, false, this);
		StaticText->setWordWrap(true);
		StaticText->setSubElement(true);
		StaticText->grab();
	}
	else
	{
		StaticText->setRelativePosition(core::rect<s32>(pos, dim));
		StaticText->setText(MessageText.c_str());
	}

	// adjust static text height

	const s32 textHeight = StaticText->getTextHeight();
	core::rect<s32> tmp = StaticText->getRelativePosition();
	tmp.LowerRightCorner.Y = tmp.UpperLeftCorner.Y + textHeight;
	StaticText->setRelativePosition(tmp);
	dim.Height = textHeight;

	// adjust message box height

	tmp = getRelativePosition();
	s32 msgBoxHeight = textHeight +	core::floor32(2.5f * buttonHeight) + titleHeight;

	// adjust message box position

	tmp.UpperLeftCorner.Y = (Parent->getAbsolutePosition().getHeight() - msgBoxHeight) / 2;
	tmp.LowerRightCorner.Y = tmp.UpperLeftCorner.Y + msgBoxHeight;
	setRelativePosition(tmp);

	// add buttons

	s32 countButtons = 0;
	if (Flags & EMBF_OK)
		++countButtons;
	if (Flags & EMBF_CANCEL)
		++countButtons;
	if (Flags & EMBF_YES)
		++countButtons;
	if (Flags & EMBF_NO)
		++countButtons;

	core::rect<s32> btnRect;
	btnRect.UpperLeftCorner.Y = pos.Y + dim.Height + buttonHeight / 2;
	btnRect.LowerRightCorner.Y = btnRect.UpperLeftCorner.Y + buttonHeight;
	btnRect.UpperLeftCorner.X = (AbsoluteClippingRect.getWidth() -
		((buttonWidth + buttonDistance)*countButtons)) / 2;
	btnRect.LowerRightCorner.X = btnRect.UpperLeftCorner.X + buttonWidth;

	// add/remove ok button
	if (Flags & EMBF_OK)
	{
		if (!OkButton)
		{
			OkButton = Environment->addButton(btnRect, this);
			OkButton->setSubElement(true);
			OkButton->grab();
		}
		else
			OkButton->setRelativePosition(btnRect);

		OkButton->setText(skin->getDefaultText(EGDT_MSG_BOX_OK));

		btnRect.LowerRightCorner.X += buttonWidth + buttonDistance;
		btnRect.UpperLeftCorner.X += buttonWidth + buttonDistance;

		focusMe = OkButton;
	}
	else if (OkButton)
	{
		OkButton->drop();
		OkButton->remove();
		OkButton = 0;
	}

	// add cancel button
	if (Flags & EMBF_CANCEL)
	{
		if (!CancelButton)
		{
			CancelButton = Environment->addButton(btnRect, this);
			CancelButton->setSubElement(true);
			CancelButton->grab();
		}
		else
			CancelButton->setRelativePosition(btnRect);

		CancelButton->setText(skin->getDefaultText(EGDT_MSG_BOX_CANCEL));

		btnRect.LowerRightCorner.X += buttonWidth + buttonDistance;
		btnRect.UpperLeftCorner.X += buttonWidth + buttonDistance;

		if (!focusMe)
			focusMe = CancelButton;

	}
	else if (CancelButton)
	{
		CancelButton->drop();
		CancelButton->remove();
		CancelButton = 0;
	}


	// add/remove yes button
	if (Flags & EMBF_YES)
	{
		if (!YesButton)
		{
			YesButton = Environment->addButton(btnRect, this);
			YesButton->setSubElement(true);
			YesButton->grab();
		}
		else
			YesButton->setRelativePosition(btnRect);

		YesButton->setText(skin->getDefaultText(EGDT_MSG_BOX_YES));

		btnRect.LowerRightCorner.X += buttonWidth + buttonDistance;
		btnRect.UpperLeftCorner.X += buttonWidth + buttonDistance;

		if (!focusMe)
			focusMe = YesButton;
	}
	else if (YesButton)
	{
		YesButton->drop();
		YesButton->remove();
		YesButton = 0;
	}

	// add no button
	if (Flags & EMBF_NO)
	{
		if (!NoButton)
		{
			NoButton = Environment->addButton(btnRect, this);
			NoButton->setSubElement(true);
			NoButton->grab();
		}
		else
			NoButton->setRelativePosition(btnRect);

		NoButton->setText(skin->getDefaultText(EGDT_MSG_BOX_NO));

		btnRect.LowerRightCorner.X += buttonWidth + buttonDistance;
		btnRect.UpperLeftCorner.X += buttonWidth + buttonDistance;

		if (!focusMe)
			focusMe = NoButton;

	}
	else if (NoButton)
	{
		NoButton->drop();
		NoButton->remove();
		NoButton = 0;
	}

	if (Environment->hasFocus(this) && focusMe)
		Environment->setFocus(focusMe);
}


//! called if an event happened.
bool CGUIMessageBox::OnEvent(const SEvent& event)
{
	SEvent outevent;
	outevent.EventType = EET_GUI_EVENT;
	outevent.GUIEvent.Caller = this;
	outevent.GUIEvent.Element = 0;

	switch(event.EventType)
	{
	case EET_KEY_INPUT_EVENT:

		if (event.KeyInput.PressedDown)
		{
			switch (event.KeyInput.Key)
			{
			case KEY_RETURN:
				if (OkButton)
				{
					OkButton->setPressed(true);
					Pressed = true;
				}
				break;
			case KEY_KEY_Y:
				if (YesButton)
				{
					YesButton->setPressed(true);
					Pressed = true;
				}
				break;
			case KEY_KEY_N:
				if (NoButton)
				{
					NoButton->setPressed(true);
					Pressed = true;
				}
				break;
			case KEY_ESCAPE:
				if (Pressed)
				{
					// cancel press
					if (OkButton) OkButton->setPressed(false);
					if (YesButton) OkButton->setPressed(false);
					if (NoButton) OkButton->setPressed(false);
					Pressed = false;
				}
				else
				if (CancelButton)
				{
					CancelButton->setPressed(true);
					Pressed = true;
				}
				else
				if (CloseButton && CloseButton->isVisible())
				{
					CloseButton->setPressed(true);
					Pressed = true;
				}
				break;
			default: // no other key is handled here
				break;
			}
		}
		else
		if (Pressed)
		{
			if (OkButton && event.KeyInput.Key == KEY_RETURN)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_OK;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if ((CancelButton || CloseButton) && event.KeyInput.Key == KEY_ESCAPE)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_CANCEL;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if (YesButton && event.KeyInput.Key == KEY_KEY_Y)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_YES;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if (NoButton && event.KeyInput.Key == KEY_KEY_N)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_NO;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
		}
		break;
	case EET_GUI_EVENT:
		if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
		{
			if (event.GUIEvent.Caller == OkButton)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_OK;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if (event.GUIEvent.Caller == CancelButton ||
				event.GUIEvent.Caller == CloseButton)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_CANCEL;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if (event.GUIEvent.Caller == YesButton)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_YES;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
			else
			if (event.GUIEvent.Caller == NoButton)
			{
				outevent.GUIEvent.EventType = EGET_MESSAGEBOX_NO;
				Parent->OnEvent(outevent);
				remove();
				return true;
			}
		}
		break;
	default:
		break;
	}

	return CGUIWindow::OnEvent(event);
}

//! Writes attributes of the element.
void CGUIMessageBox::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	CGUIWindow::serializeAttributes(out,options);

	out->addBool	("OkayButton",		(Flags & EMBF_OK)	!= 0 );
	out->addBool	("CancelButton",	(Flags & EMBF_CANCEL)	!= 0 );
	out->addBool	("YesButton",		(Flags & EMBF_YES)	!= 0 );
	out->addBool	("NoButton",		(Flags & EMBF_NO)	!= 0 );

	out->addString	("MessageText",		MessageText.c_str());
}

//! Reads attributes of the element
void CGUIMessageBox::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	Flags = 0;

	Flags  = in->getAttributeAsBool("OkayButton")  ? EMBF_OK     : 0;
	Flags |= in->getAttributeAsBool("CancelButton")? EMBF_CANCEL : 0;
	Flags |= in->getAttributeAsBool("YesButton")   ? EMBF_YES    : 0;
	Flags |= in->getAttributeAsBool("NoButton")    ? EMBF_NO     : 0;

	MessageText = in->getAttributeAsStringW("MessageText").c_str();

	CGUIWindow::deserializeAttributes(in,options);

	refreshControls();
}


} // end namespace gui
} // end namespace irr


#endif // _IRR_COMPILE_WITH_GUI_

