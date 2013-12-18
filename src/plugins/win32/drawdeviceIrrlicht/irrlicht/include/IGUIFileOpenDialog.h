// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_GUI_FILE_OPEN_DIALOG_H_INCLUDED__
#define __I_GUI_FILE_OPEN_DIALOG_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	//! Standard file chooser dialog.
	class IGUIFileOpenDialog : public IGUIElement
	{
	public:

		//! constructor
		IGUIFileOpenDialog(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(EGUIET_FILE_OPEN_DIALOG, environment, parent, id, rectangle) {}

		//! Returns the filename of the selected file. Returns NULL, if no file was selected.
		virtual const wchar_t* getFileName() const = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

