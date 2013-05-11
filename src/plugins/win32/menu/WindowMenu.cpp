
#include "WindowMenu.h"
#include <algorithm>

namespace TML {
WindowMenuItem::WindowMenuItem( Window* window, std::string& caption ) : owner_window_(window), caption_(caption), hMenu_(NULL) {
	ZeroMemory( &menu_item_info_, sizeof(MENUITEMINFO) );
	menu_item_info_.cbSize = sizeof(MENUITEMINFO);
	menu_item_info_.dwItemData = (ULONG_PTR)this;
	menu_item_info_.fMask |= MIIM_DATA;
	//hMenu_ = INVALID_HANDLE_VALUE;
}

int WindowMenuItem::GetIndex() const {
	if( parent_ == NULL ) return -1;
	size_t count = children_.size();
	for( size_t i = 0; i < count; i++ ) {
		if( children_[i] == this ) {
			return i;
		}
	}
	return -1;
}
void WindowMenuItem::SetIndex( int index ) {
	if( GetIndex() != index ) {
		parent_->remove( this );
		parent_->insert( this, index );
	}
}

void WindowMenuItem::add( WindowMenuItem* item ) {
	if( item->parent_ ) {
		item->parent_->remove( item );
	}
	item->parent_ = this;
	children_.push_back( item );
}
void WindowMenuItem::insert( WindowMenuItem* item, int index ) {
	if( item->parent_ ) {
		item->parent_->remove( item );
	}
	item->parent_ = this;
	std::vector<WindowMenuItem*>::iterator it = children_.begin();
	for( int i = 0; i < index; i++ ) it++;
	children_.insert( it, item );
}
void WindowMenuItem::popup( int flags, int x, int y ) {
}

void WindowMenuItem::remove( WindowMenuItem* item ) {
	std::vector<WindowMenuItem*>::iterator end_it = std::remove( children_.begin(), children_.end(), item );
	children_.erase( end_it, children_.end() );
	item->parent_ = NULL;
}

void WindowMenuItem::OnClick() {
}

static void OnClickHandler();
}


