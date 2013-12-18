// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_CONTEXT_MENU_H_INCLUDED__
#define __C_GUI_CONTEXT_MENU_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIContextMenu.h"
#include "irrString.h"
#include "irrArray.h"
#include "IGUIFont.h"

namespace irr
{
namespace gui
{

	//! GUI Context menu interface.
	class CGUIContextMenu : public IGUIContextMenu
	{
	public:

		//! constructor
		CGUIContextMenu(IGUIEnvironment* environment,
			IGUIElement* parent, s32 id, core::rect<s32> rectangle,
			bool getFocus = true, bool allowFocus = true);

		//! destructor
		virtual ~CGUIContextMenu();

		//! Returns amount of menu items
		virtual u32 getItemCount() const;

		//! Adds a menu item.
		virtual u32 addItem(const wchar_t* text, s32 commandid,
				bool enabled, bool hasSubMenu, bool checked);

		//! Adds a separator item to the menu
		virtual void addSeparator();

		//! Returns text of the menu item.
		virtual const wchar_t* getItemText(u32 idx) const;

		//! Sets text of the menu item.
		virtual void setItemText(u32 idx, const wchar_t* text);

		//! Returns if a menu item is enabled
		virtual bool isItemEnabled(u32 idx) const;

		//! Sets if the menu item should be enabled.
		virtual void setItemEnabled(u32 idx, bool enabled);

		//! Returns if a menu item is checked
		virtual bool isItemChecked(u32 idx) const;

		//! Sets if the menu item should be checked.
		virtual void setItemChecked(u32 idx, bool enabled);

		//! Removes a menu item
		virtual void removeItem(u32 idx);

		//! Removes all menu items
		virtual void removeAllItems();

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! draws the element and its children
		virtual void draw();

		//! Returns the selected item in the menu
		virtual s32 getSelectedItem() const;

		//! Returns a pointer to the submenu of an item.
		//! \return Pointer to the submenu of an item.
		virtual IGUIContextMenu* getSubMenu(u32 idx) const;

		//! Sets the visible state of this element.
		virtual void setVisible(bool visible);

		//! Returns command id of a menu item
		virtual s32 getItemCommandId(u32 idx) const;

		//! Sets the command id of a menu item
		virtual void setItemCommandId(u32 idx, s32 id);

		//! Adds a sub menu from an element that already exists.
		virtual void setSubMenu(u32 index, CGUIContextMenu* menu);

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

	protected:

		void closeAllSubMenus();
		bool hasOpenSubMenu() const;

		struct SItem
		{
			core::stringw Text;
			bool IsSeparator;
			bool Enabled;
			bool Checked;
			core::dimension2d<s32> Dim;
			s32 PosY;
			CGUIContextMenu* SubMenu;
			s32 CommandId;
		};

		virtual void recalculateSize();

		//! returns true, if an element was highlighted
		virtual bool highlight(const core::position2d<s32>& p, bool canOpenSubMenu);

		//! sends a click Returns:
		//! 0 if click went outside of the element,
		//! 1 if a valid button was clicked,
		//! 2 if a nonclickable element was clicked
		virtual u32 sendClick(const core::position2d<s32>& p);

		//! returns the item highlight-area
		virtual core::rect<s32> getHRect(const SItem& i, const core::rect<s32>& absolute) const;

		//! Gets drawing rect of Item
		virtual core::rect<s32> getRect(const SItem& i, const core::rect<s32>& absolute) const;

		void setEventParent(IGUIElement *parent);

		s32 HighLighted;
		core::array<SItem> Items;
		core::position2d<s32> Pos;
		u32 ChangeTime;
		IGUIElement* EventParent;
		bool AllowFocus;
		IGUIFont *LastFont;
	};


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

#endif // __C_GUI_CONTEXT_MENU_H_INCLUDED__

