// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_WINDOW_H_INCLUDED__
#define __C_GUI_WINDOW_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIWindow.h"

namespace irr
{
namespace gui
{
	class IGUIButton;

	class CGUIWindow : public IGUIWindow
	{
	public:

		//! constructor
		CGUIWindow(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

		//! destructor
		virtual ~CGUIWindow();

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! update absolute position
		virtual void updateAbsolutePosition();

		//! draws the element and its children
		virtual void draw();

		//! Returns pointer to the close button
		virtual IGUIButton* getCloseButton() const;

		//! Returns pointer to the minimize button
		virtual IGUIButton* getMinimizeButton() const;

		//! Returns pointer to the maximize button
		virtual IGUIButton* getMaximizeButton() const;

	protected:

		core::position2d<s32> DragStart;
		bool Dragging;

		IGUIButton* CloseButton;
		IGUIButton* MinButton;
		IGUIButton* RestoreButton;
	};

} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

#endif 

