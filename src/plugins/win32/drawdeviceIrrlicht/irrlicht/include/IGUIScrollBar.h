// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_GUI_SCROLL_BAR_H_INCLUDED__
#define __I_GUI_SCROLL_BAR_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	//! Default scroll bar GUI element.
	class IGUIScrollBar : public IGUIElement
	{
	public:

		//! constructor
		IGUIScrollBar(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(EGUIET_SCROLL_BAR, environment, parent, id, rectangle) {}

		//! gets the maximum value of the scrollbar.
		virtual s32 getMax() const = 0;

		//! sets the maximum value of the scrollbar.
		virtual void setMax(s32 max) = 0;

		//! gets the small step value
		virtual s32 getSmallStep() const = 0;

		//! Sets the small step, the amount that the value changes by when clicking
		//! on the buttons or using the cursor keys.
		virtual void setSmallStep(s32 step) = 0;

		//! gets the large step value
		virtual s32 getLargeStep() const = 0;

		//! Sets the large step, the amount that the value changes by when clicking
		//! in the tray, or using the page up and page down keys.
		virtual void setLargeStep(s32 step) = 0;

		//! gets the current position of the scrollbar
		virtual s32 getPos() const = 0;

		//! sets the current position of the scrollbar
		virtual void setPos(s32 pos) = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

