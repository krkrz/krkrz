
#include "WindowMenu.h"
#include "MenuItemIntf.h"
#include <algorithm>
#include <string.h>
#include <tchar.h>

int WindowMenuItem::CURRENT_MENU_ID = WindowMenuItem::MENU_ID_MIN;
std::vector<int> WindowMenuItem::FREE_ID_LIST;
std::map<int, WindowMenuItem*> WindowMenuItem::ID_TO_ITEM;

/**
 * 削除時にID使い回すようにした方がいい
 */
int WindowMenuItem::GetNewMenuId( WindowMenuItem* item ) {
	// 要素がある場合はそれを返す
	if( FREE_ID_LIST.size() > 0 ) {
		int result = FREE_ID_LIST.back();
		FREE_ID_LIST.pop_back();
		ID_TO_ITEM.insert( std::map<int, WindowMenuItem*>::value_type( result, item ) );
		return result;
	}

	int result = CURRENT_MENU_ID;
	CURRENT_MENU_ID++;
	if( result > MENU_ID_MAX ) {
		// exception
	}
	ID_TO_ITEM.insert( std::map<int, WindowMenuItem*>::value_type( result, item ) );
	return result;
}
void WindowMenuItem::ReleaseMenuId( int id ) {
	FREE_ID_LIST.push_back( id );
	ID_TO_ITEM.erase(id);
}
WindowMenuItem::WindowMenuItem( tTJSNI_MenuItem* owner, HWND hWnd, HMENU hMenu ) : owner_(owner), hWnd_(hWnd), hMenu_(hMenu), short_cut_key_(0), group_no_(-1), parent_(NULL), is_visible_(true)  {
	ZeroMemory( &menu_item_info_, sizeof(MENUITEMINFO) );
	menu_item_info_.cbSize = sizeof(MENUITEMINFO);
	menu_item_info_.dwItemData = (ULONG_PTR)this;
	menu_item_info_.fMask = MIIM_DATA|MIIM_ID;
	menu_item_info_.fType = MFT_STRING;
	menu_item_info_.wID = GetNewMenuId(this);
	if( hMenu == NULL ) {
		hMenu_ = ::CreatePopupMenu();
	}
}

WindowMenuItem::~WindowMenuItem() {
	if( parent_ ) {
		parent_->Remove( this );
		parent_ = NULL;
	}
	if( menu_item_info_.dwTypeData ) {
		delete[] (TCHAR*)menu_item_info_.dwTypeData;
		menu_item_info_.dwTypeData = NULL;
		menu_item_info_.cch = 0;
	}
	ReleaseMenuId( menu_item_info_.wID );
}
int WindowMenuItem::GetMenuIndex() const {
	if( parent_ == NULL ) return -1;
	size_t count = parent_->children_.size();
	for( size_t i = 0; i < count; i++ ) {
		if( parent_->children_[i] == this ) {
			return i;
		}
	}
	return -1;
}
void WindowMenuItem::SetMenuIndex( int index ) {
	if( GetMenuIndex() != index ) {
		parent_->Remove( this, false );
		parent_->Insert( index, this );
		// UpdateMenu();
	}
}

void WindowMenuItem::UpdateChildren() {
	if( hMenu_ != NULL ) {
		int count = ::GetMenuItemCount(hMenu_);
		while( count ) {
			::RemoveMenu(hMenu_,0,MF_BYPOSITION);
			count--;
		}
		count = children_.size();
		int index = 0;
		for( int i = 0; i < count; i++ ) {
			if( children_[i]->GetVisible() ) {
				//::SetMenuItemInfo( hMenu_, index, TRUE, &(children_[i]->menu_item_info_) );
				::InsertMenuItem( hMenu_, index, TRUE, &(children_[i]->menu_item_info_) );
				index++;
			}
		}
	}
	::DrawMenuBar( hWnd_ );
}
void WindowMenuItem::UpdateMenu() {
	if( parent_ ) {
		parent_->UpdateChildren();
	}
}

void WindowMenuItem::CheckRadioItem( WindowMenuItem* item ) {
	int radioindex = item->GetGroupIndex();
	UncheckRadioItem( radioindex );
	item->menu_item_info_.fState |= MFS_CHECKED;
}

void WindowMenuItem::UncheckRadioItem( int group ) {
	int count = children_.size();
	for( int i = 0; i < count; i++ ) {
		if( children_[i]->GetGroupIndex() == group && children_[i]->GetRadioItem() ) {
			children_[i]->menu_item_info_.fState &= ~MFS_CHECKED;
		}
	}
}
int WindowMenuItem::GetCheckRadioIndex( int group ) {
	int count = children_.size();
	for( int i = 0; i < count; i++ ) {
		if( children_[i]->GetGroupIndex() == group && children_[i]->GetRadioItem() ) {
			if( children_[i]->GetChecked() == true ) {
				return i;
			}
		}
	}
	return -1;
}

bool WindowMenuItem::AddSubMenu() {
	if( (menu_item_info_.fMask&MIIM_SUBMENU) == 0 ) {
		menu_item_info_.fMask |= MIIM_SUBMENU;
		menu_item_info_.hSubMenu = hMenu_;
		return true;
	}
	return false;
}
bool WindowMenuItem::RemoveSubMenu() {
	if( menu_item_info_.fMask&MIIM_SUBMENU ) {
		menu_item_info_.fMask &= ~MIIM_SUBMENU;
		menu_item_info_.hSubMenu = 0;
		return true;
	}
	return false;
}
void WindowMenuItem::Add( WindowMenuItem* item ) {
	if( item->parent_ ) {
		item->parent_->Remove( item, false );
	}
	item->parent_ = this;
	children_.push_back( item );
	if( AddSubMenu() ) {
		UpdateMenu();
	}
	UpdateChildren();
}
void WindowMenuItem::Insert( int index, WindowMenuItem* item ) {
	if( item->parent_ ) {
		item->parent_->Remove( item, false );
	}
	item->parent_ = this;
	std::vector<WindowMenuItem*>::iterator it = children_.begin();
	for( int i = 0; i < index; i++ ) it++;
	children_.insert( it, item );
	if( AddSubMenu() ) {
		UpdateMenu();
	}
	UpdateChildren();
}
void WindowMenuItem::Remove( WindowMenuItem* item, bool ignoreupdate ) {
	std::vector<WindowMenuItem*>::iterator end_it = std::remove( children_.begin(), children_.end(), item );
	children_.erase( end_it, children_.end() );
	item->parent_ = NULL;
	if( children_.size() == 0 ) {
		if( RemoveSubMenu() ) {
			UpdateMenu();
		}
	}
	if( ignoreupdate == false ) UpdateChildren();
}

void WindowMenuItem::Delete( int index ) {
	if( index >= 0 && index < static_cast<int>(children_.size()) ) {
		Remove( children_[index] );
	}
}
int WindowMenuItem::IndexOf( const WindowMenuItem* item ) {
	int count = children_.size();
	int i = 0;
	for( ; i < count; i++ ) {
		if( children_[i] == item ) {
			return i;
		}
	}
	return -1;
}

void WindowMenuItem::SetCaption( const TCHAR* caption ) {
	if( menu_item_info_.dwTypeData ) {
		delete[] (TCHAR*)menu_item_info_.dwTypeData;
		menu_item_info_.dwTypeData = NULL;
		//menu_item_info_.cch = 0;
	}
	menu_item_info_.fMask &= ~(MIIM_STRING|MIIM_FTYPE);
	if( caption ) {
		int len = _tcsnlen( caption, MAX_CAPTION_LENGTH )+1;
		if( len > 0 ) {
			TCHAR* c = new TCHAR[len];
			_tcscpy_s( c, len, caption );
			menu_item_info_.dwTypeData = (LPWSTR)c;
			//menu_item_info_.cch = len;
			menu_item_info_.fMask |= MIIM_TYPE;
		}
	}
	UpdateMenu();
}

void WindowMenuItem::SetChecked( bool b ) {
	if( GetRadioItem() ) {
		if( b && parent_ ) {
			parent_->CheckRadioItem( this );
		} else {
			menu_item_info_.fState &= ~MFS_CHECKED;
		}
	} else {
		if( b ) {
			menu_item_info_.fState |= MFS_CHECKED;
		} else {
			menu_item_info_.fState &= ~MFS_CHECKED;
		}
	}
	UpdateMenu();
}
void WindowMenuItem::SetEnabled( bool b ) {
	if( b == GetEnabled() ) return;

	if( b ) {
		menu_item_info_.fState &= ~MFS_DISABLED;
	} else {
		menu_item_info_.fState |= MFS_DISABLED;
	}
	UpdateMenu();
}
void WindowMenuItem::SetGroupIndex( int group ) {
	group_no_ = group;
	if( parent_ && GetChecked() && GetRadioItem() ) {
		// 自身がcheckついていて、他にもcheckついているものがある場合は自身のcheckを外す
		int index = parent_->GetCheckRadioIndex(group);
		if( index >= 0 ) {
			menu_item_info_.fState &= ~MFS_CHECKED;
		}
	}
	UpdateMenu();
}
void WindowMenuItem::SetRadioItem( bool b ) {
	if( b == GetRadioItem() ) return;

	if( b ) {
		menu_item_info_.fType |= MFT_RADIOCHECK;
	} else {
		menu_item_info_.fType &= ~MFT_RADIOCHECK;
	}
	if( b && parent_ && GetChecked() ) {
		// 自身がcheckついていて、他にもcheckついているものがある場合は自身のcheckを外す
		int index = parent_->GetCheckRadioIndex(group_no_);
		if( index >= 0 ) {
			menu_item_info_.fState &= ~MFS_CHECKED;
		}
	}
	UpdateMenu();
}
void WindowMenuItem::SetShortCut( int key ) {
	short_cut_key_ = key;
	char virt = static_cast<char>(key >> 16);
	TVPRegisterAcceleratorKey( hWnd_, virt, static_cast<short>(key&0xffff), menu_item_info_.wID );
}
void WindowMenuItem::SetVisible( bool b ) {
	is_visible_ = b;
	UpdateMenu();
}

/*
void WindowMenuItem::Popup( int flags, int x, int y ) {
	if( hMenu_ ) {
		::TrackPopupMenu( hMenu_, flags, x, y, 0, hWnd_, NULL );
	}
}
*/
void WindowMenuItem::OnClick() {
	if( owner_ ) {
		owner_->MenuItemClick();
	}
}

void WindowMenuItem::OnClickHandler( int id ) {
	std::map<int, WindowMenuItem*>::iterator i = ID_TO_ITEM.find( id );
	if( i != ID_TO_ITEM.end() ) {
		i->second->OnClick();
	}
}
