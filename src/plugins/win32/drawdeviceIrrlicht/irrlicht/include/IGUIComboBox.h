// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_GUI_COMBO_BOX_H_INCLUDED__
#define __I_GUI_COMBO_BOX_H_INCLUDED__

#include "IGUIElement.h"

namespace irr
{
namespace gui
{

	//! Combobox widget
	class IGUIComboBox : public IGUIElement
	{
	public:

		//! constructor
		IGUIComboBox(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
			: IGUIElement(EGUIET_COMBO_BOX, environment, parent, id, rectangle) {}

		//! Returns amount of items in box
		virtual u32 getItemCount() const = 0;

		//! Returns string of an item. the idx may be a value from 0 to itemCount-1
		virtual const wchar_t* getItem(u32 idx) const = 0;

		//! Adds an item and returns the index of it
		virtual u32 addItem(const wchar_t* text) = 0;

		//! Removes an item from the combo box.
		/** Warning. This will change the index of all following items */
		virtual void removeItem(u32 idx) = 0;

		//! Deletes all items in the combo box
		virtual void clear() = 0;

		//! Returns id of selected item. returns -1 if no item is selected.
		virtual s32 getSelected() const = 0;

		//! Sets the selected item. Set this to -1 if no item should be selected
		virtual void setSelected(s32 idx) = 0;
	};


} // end namespace gui
} // end namespace irr

#endif

