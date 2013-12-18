// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_TAB_CONTROL_H_INCLUDED__
#define __C_GUI_TAB_CONTROL_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUITabControl.h"
#include "irrArray.h"

namespace irr
{
namespace gui
{
	// A tab, onto which other gui elements could be added.
	class CGUITab : public IGUITab
	{
	public:

		//! constructor
		CGUITab(s32 number, IGUIEnvironment* environment,
			IGUIElement* parent, const core::rect<s32>& rectangle,
			s32 id);

		//! Returns number of this tab in tabcontrol. Can be accessed
		//! later IGUITabControl::getTab() by this number.
		virtual s32 getNumber() const;

		//! Sets the number
		virtual void setNumber(s32 n);

		//! draws the element and its children
		virtual void draw();

		//! sets if the tab should draw its background
		virtual void setDrawBackground(bool draw=true);

		//! sets the color of the background, if it should be drawn.
		virtual void setBackgroundColor(video::SColor c);

		//! returns true if the tab is drawing its background, false if not
		virtual bool isDrawingBackground() const;

		//! returns the color of the background
		virtual video::SColor getBackgroundColor() const;

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);


	private:

		s32 Number;
		bool DrawBackground;
		video::SColor BackColor;
	};



	//! A standard tab control
	class CGUITabControl : public IGUITabControl
	{
	public:

		//! destructor
		CGUITabControl(IGUIEnvironment* environment,
			IGUIElement* parent, const core::rect<s32>& rectangle,
			bool fillbackground=true, bool border=true, s32 id=-1);

		//! destructor
		virtual ~CGUITabControl();

		//! Adds a tab
		virtual IGUITab* addTab(const wchar_t* caption, s32 id=-1);

		//! Adds a tab that has already been created
		virtual void addTab(CGUITab* tab);

		//! Returns amount of tabs in the tabcontrol
		virtual s32 getTabCount() const;

		//! Returns a tab based on zero based index
		virtual IGUITab* getTab(s32 idx) const;

		//! Brings a tab to front.
		virtual bool setActiveTab(s32 idx);

		//! Brings a tab to front.
		virtual bool setActiveTab(IGUIElement *tab);

		//! Returns which tab is currently active
		virtual s32 getActiveTab() const;

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! draws the element and its children
		virtual void draw();

		//! Removes a child.
		virtual void removeChild(IGUIElement* child);

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

	private:

		void selectTab(core::position2d<s32> p);

		core::array<CGUITab*> Tabs;
		s32 ActiveTab;
		bool Border;
		bool FillBackground;
	};


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

#endif

