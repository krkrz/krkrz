// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_CURSOR_CONTROL_H_INCLUDED__
#define __I_CURSOR_CONTROL_H_INCLUDED__

#include "IReferenceCounted.h"
#include "position2d.h"
#include "rect.h"

namespace irr
{
namespace gui
{

	//! Interface to manipulate the mouse cursor.
	class ICursorControl : public virtual IReferenceCounted
	{
	public:

		//! Changes the visible state of the mouse cursor.
		/** \param visible: The new visible state. If true, the cursor will be visible,
		if false, it will be invisible. */
		virtual void setVisible(bool visible) = 0;

		//! Returns if the cursor is currently visible.
		/** \return True if the cursor is visible, false if not. */
		virtual bool isVisible() const = 0;

		//! Sets the new position of the cursor.
		/** The position must be
		between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
		the top left corner and (1.0f, 1.0f) is the bottom right corner of the
		render window.
		\param pos New position of the cursor. */
		virtual void setPosition(const core::position2d<f32> &pos) = 0;

		//! Sets the new position of the cursor.
		/** The position must be
		between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
		the top left corner and (1.0f, 1.0f) is the bottom right corner of the
		render window.
		\param x New x-coord of the cursor.
		\param y New x-coord of the cursor. */
		virtual void setPosition(f32 x, f32 y) = 0;

		//! Sets the new position of the cursor.
		/** \param pos: New position of the cursor. The coordinates are pixel units. */
		virtual void setPosition(const core::position2d<s32> &pos) = 0;

		//! Sets the new position of the cursor.
		/** \param x New x-coord of the cursor. The coordinates are pixel units.
		\param y New y-coord of the cursor. The coordinates are pixel units. */
		virtual void setPosition(s32 x, s32 y) = 0;

		//! Returns the current position of the mouse cursor.
		/** \return Returns the current position of the cursor. The returned position
		is the position of the mouse cursor in pixel units. */
		virtual core::position2d<s32> getPosition() = 0;

		//! Returns the current position of the mouse cursor.
		/** \return Returns the current position of the cursor. The returned position
		is a value between (0.0f, 0.0f) and (1.0f, 1.0f), where (0.0f, 0.0f) is
		the top left corner and (1.0f, 1.0f) is the bottom right corner of the
		render window. */
		virtual core::position2d<f32> getRelativePosition() = 0;

		//! Sets an absolute reference rect for setting and retrieving the cursor position.
		/** If this rect is set, the cursor position is not being calculated relative to
		the rendering window but to this rect. You can set the rect pointer to 0 to disable
		this feature again. This feature is useful when rendering into parts of foreign windows
		for example in an editor.
		\param rect: A pointer to an reference rectangle or 0 to disable the reference rectangle.*/
		virtual void setReferenceRect(core::rect<s32>* rect=0) = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

