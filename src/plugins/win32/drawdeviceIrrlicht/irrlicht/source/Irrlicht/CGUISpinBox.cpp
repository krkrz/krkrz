// Copyright (C) 2006-2008 Michael Zeilfelder
// This file uses the licence of the Irrlicht Engine.

#include "CGUISpinBox.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "CGUIEditBox.h"
#include "CGUIButton.h"
#include "IGUIEnvironment.h"
#include "IEventReceiver.h"
#include "fast_atof.h"
#include <float.h>
#include <wchar.h>


namespace irr
{
namespace gui
{

//! constructor
CGUISpinBox::CGUISpinBox(const wchar_t* text, IGUIEnvironment* environment,
			IGUIElement* parent, s32 id, const core::rect<s32>& rectangle)
: IGUISpinBox(environment, parent, id, rectangle),
	EditBox(0), ButtonSpinUp(0), ButtonSpinDown(0), StepSize(1.f),
	RangeMin(-FLT_MAX), RangeMax(FLT_MAX), FormatString(L"%f"),
	DecimalPlaces(-1)
{
	s32 ButtonWidth = 16;
	IGUISpriteBank *sb = 0;
	if (environment && environment->getSkin())
	{
		ButtonWidth = environment->getSkin()->getSize(EGDS_SCROLLBAR_SIZE);
		sb = environment->getSkin()->getSpriteBank();
	}

	ButtonSpinDown = Environment->addButton(
		core::rect<s32>(rectangle.getWidth() - ButtonWidth, rectangle.getHeight()/2 +1,
						rectangle.getWidth(), rectangle.getHeight()), this);
	ButtonSpinDown->grab();
	ButtonSpinDown->setSubElement(true);
	ButtonSpinDown->setTabStop(false);
	ButtonSpinDown->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_CENTER, EGUIA_LOWERRIGHT);

	ButtonSpinUp = Environment->addButton(
		core::rect<s32>(rectangle.getWidth() - ButtonWidth, 0,
						rectangle.getWidth(), rectangle.getHeight()/2), this);
	ButtonSpinUp->grab();
	ButtonSpinUp->setSubElement(true);
	ButtonSpinUp->setTabStop(false);
	ButtonSpinUp->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_CENTER);
	if (sb)
	{
		IGUISkin *skin = environment->getSkin();
		ButtonSpinDown->setSpriteBank(sb);
		ButtonSpinDown->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_SMALL_CURSOR_DOWN), skin->getColor(EGDC_WINDOW_SYMBOL));
		ButtonSpinDown->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_SMALL_CURSOR_DOWN), skin->getColor(EGDC_WINDOW_SYMBOL));
		ButtonSpinUp->setSpriteBank(sb);
		ButtonSpinUp->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_SMALL_CURSOR_UP), skin->getColor(EGDC_WINDOW_SYMBOL));
		ButtonSpinUp->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_SMALL_CURSOR_UP), skin->getColor(EGDC_WINDOW_SYMBOL));
	}
	else
	{
		ButtonSpinDown->setText(L"-");
		ButtonSpinUp->setText(L"+");
	}

	const core::rect<s32> rectEdit(0, 0, rectangle.getWidth() - ButtonWidth - 1, rectangle.getHeight());
	EditBox = Environment->addEditBox(text, rectEdit, true, this, -1);
	EditBox->grab();
	EditBox->setSubElement(true);
	EditBox->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
}


//! destructor
CGUISpinBox::~CGUISpinBox()
{
	if (ButtonSpinUp)
		ButtonSpinUp->drop();
	if (ButtonSpinDown)
		ButtonSpinDown->drop();
	if (EditBox)
		EditBox->drop();
}


IGUIEditBox* CGUISpinBox::getEditBox() const
{
	return EditBox;
}


void CGUISpinBox::setValue(f32 val)
{
	wchar_t str[100];

	swprintf(str, 99, FormatString.c_str(), val);
	EditBox->setText(str);
	verifyValueRange();
}


f32 CGUISpinBox::getValue() const
{
	const wchar_t* val = EditBox->getText();
	if ( !val )
		return 0.f;
	core::stringc tmp(val);
	return core::fast_atof(tmp.c_str());
}


void CGUISpinBox::setRange(f32 min, f32 max)
{
	RangeMin = min;
	RangeMax = max;
	verifyValueRange();
}


f32 CGUISpinBox::getMin() const
{
	return RangeMin;
}


f32 CGUISpinBox::getMax() const
{
	return RangeMax;
}


f32 CGUISpinBox::getStepSize() const
{
	return StepSize;
}


void CGUISpinBox::setStepSize(f32 step)
{
	StepSize = step;
}


//! Sets the number of decimal places to display.
void CGUISpinBox::setDecimalPlaces(s32 places)
{
	DecimalPlaces = places;
	if (places == -1)
		FormatString = "%f";
	else
	{
		FormatString = "%.";
		FormatString += places;
		FormatString += "f";
	}
	setValue(getValue());
}


bool CGUISpinBox::OnEvent(const SEvent& event)
{
	bool changeEvent = false;
	switch(event.EventType)
	{
	case EET_GUI_EVENT:
		if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
		{
			if (event.GUIEvent.Caller == ButtonSpinUp)
			{
				f32 val = getValue();
				val += StepSize;
				setValue(val);
				changeEvent = true;
			}
			else if ( event.GUIEvent.Caller == ButtonSpinDown)
			{
				f32 val = getValue();
				val -= StepSize;
				setValue(val);
				changeEvent = true;
			}
		}
		if ( event.GUIEvent.EventType == EGET_EDITBOX_ENTER )
		{
			if (event.GUIEvent.Caller == EditBox)
			{
				verifyValueRange();
				changeEvent = true;
			}
		}
		break;
	default:
	break;
	}

	if ( changeEvent )
	{
		SEvent e;
		e.EventType = EET_GUI_EVENT;
		e.GUIEvent.Caller = this;
		e.GUIEvent.Element = 0;

		e.GUIEvent.EventType = EGET_SPINBOX_CHANGED;
		if ( Parent )
			Parent->OnEvent(e);
		return true;
	}

	return IGUIElement::OnEvent(event);
}


void CGUISpinBox::verifyValueRange()
{
	f32 val = getValue();
	if ( val < RangeMin )
		val = RangeMin;
	else if ( val > RangeMax )
		val = RangeMax;
	else
		return;

	setValue(val);
}


//! Sets the new caption of the element
void CGUISpinBox::setText(const wchar_t* text)
{
	EditBox->setText(text);
	setValue(getValue());
	verifyValueRange();
}


//! Returns caption of this element.
const wchar_t* CGUISpinBox::getText() const
{
	return EditBox->getText();
}


//! Writes attributes of the element.
void CGUISpinBox::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	IGUIElement::serializeAttributes(out, options);
	out->addFloat("Min", getMin());
	out->addFloat("Max", getMax());
	out->addFloat("Step", getStepSize());
	out->addInt("DecimalPlaces", DecimalPlaces);
}

//! Reads attributes of the element
void CGUISpinBox::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	IGUIElement::deserializeAttributes(in, options);
	setRange(in->getAttributeAsFloat("Min"), in->getAttributeAsFloat("Max"));
	setStepSize(in->getAttributeAsFloat("Step"));
	setDecimalPlaces(in->getAttributeAsInt("DecimalPlaces"));
}


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

