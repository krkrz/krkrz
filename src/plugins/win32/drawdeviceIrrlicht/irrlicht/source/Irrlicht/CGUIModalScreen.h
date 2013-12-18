// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_MODAL_SCREEN_H_INCLUDED__
#define __C_GUI_MODAL_SCREEN_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	class CGUIModalScreen : public IGUIElement
	{
	public:

		//! constructor
		CGUIModalScreen(IGUIEnvironment* environment, IGUIElement* parent, s32 id);

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! Removes a child.
		virtual void removeChild(IGUIElement* child);

		//! Adds a child
		virtual void addChild(IGUIElement* child);


		//! draws the element and its children
		virtual void draw();

		//! Updates the absolute position.
		virtual void updateAbsolutePosition();

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

	private:

		u32 MouseDownTime;
	};


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

#endif
