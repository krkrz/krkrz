// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_GUI_TAB_CONTROL_H_INCLUDED__
#define __I_GUI_TAB_CONTROL_H_INCLUDED__

#include "IGUIElement.h"
#include "SColor.h"

namespace irr
{
namespace gui
{
	//! A tab, onto which other gui elements could be added.
	class IGUITab : public IGUIElement
	{
	public:

		//! constructor
		IGUITab(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(EGUIET_TAB, environment, parent, id, rectangle) {}

		//! Returns number of tab if in tabcontrol.
		/** Can be accessed later IGUITabControl::getTab() by this number. */
		virtual s32 getNumber() const = 0;

		//! sets if the tab should draw its background
		virtual void setDrawBackground(bool draw=true) = 0;

		//! sets the color of the background, if it should be drawn.
		virtual void setBackgroundColor(video::SColor c) = 0;

		//! returns true if the tab is drawing its background, false if not
		virtual bool isDrawingBackground() const = 0;

		//! returns the color of the background
		virtual video::SColor getBackgroundColor() const = 0;
	};

	//! A standard tab control
	class IGUITabControl : public IGUIElement
	{
	public:

		//! constructor
		IGUITabControl(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(EGUIET_TAB_CONTROL, environment, parent, id, rectangle) {}

		//! Adds a tab
		virtual IGUITab* addTab(const wchar_t* caption, s32 id=-1) = 0;

		//! Returns amount of tabs in the tabcontrol
		virtual s32 getTabCount() const = 0;

		//! Returns a tab based on zero based index
		/** \param idx: zero based index of tab. Is a value betwenn 0 and getTabcount()-1;
		\return Returns pointer to the Tab. Returns 0 if no tab
		is corresponding to this tab. */
		virtual IGUITab* getTab(s32 idx) const = 0;

		//! Brings a tab to front.
		/** \param idx: number of the tab.
		\return Returns true if successful. */
		virtual bool setActiveTab(s32 idx) = 0;

		//! Brings a tab to front.
		/** \param tab: pointer to the tab.
		\return Returns true if successful. */
		virtual bool setActiveTab(IGUIElement *tab) = 0;

		//! Returns which tab is currently active
		virtual s32 getActiveTab() const = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

