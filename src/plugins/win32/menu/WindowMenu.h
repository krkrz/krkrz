

#ifndef __WINDOW_MENU_H__
#define __WINDOW_MENU_H__

#include <string>
#include <vector>
#include <map>
#include <windows.h>

class WindowMenuItem {
	static const int MAX_CAPTION_LENGTH = 1024;
	
	static const int MENU_ID_MIN = 0x2000;
	static const int MENU_ID_MAX = 0x6FFF;

	static std::vector<int> FREE_ID_LIST;
	static std::map<int, WindowMenuItem*> ID_TO_ITEM;
	static int CURRENT_MENU_ID;
	static int GetNewMenuId( WindowMenuItem* item );
	static void ReleaseMenuId( int id );

protected:
	MENUITEMINFO menu_item_info_;
	HMENU hMenu_;
	HWND hWnd_;

private:
	class tTJSNI_MenuItem* owner_;
	std::string caption_;
	int short_cut_key_;

	int group_no_;
	bool is_visible_;

	WindowMenuItem* parent_;
	// WindowMenuItem* root_;

	std::vector<WindowMenuItem*> children_;

	void Remove( WindowMenuItem* item, bool ignoreupdate = false );

	/**
	 * 全ての項目を削除してから追加し直す
	 */
	void UpdateChildren();
	void UpdateMenu();

	void CheckRadioItem( WindowMenuItem* item );

	void UncheckRadioItem( int group );

	/**
	 * 指定グループの内でcheckがついているアイテムインデックスを取得する
	 */
	int GetCheckRadioIndex( int group );
	/**
	 * ラジオアイテム化する時に
	 */
	void RadioItemSafeCheck();

	bool AddSubMenu();
	bool RemoveSubMenu();
public:
	WindowMenuItem( class tTJSNI_MenuItem* owner, HWND hWnd, HMENU hMenu=NULL );
	~WindowMenuItem();

	void Add( WindowMenuItem* item );
	void Insert( int index, WindowMenuItem* item );
	void Delete( int index );
	//void Popup( int flags, int x, int y );

	void SetCaption( const TCHAR* caption );
	const TCHAR* GetCaption() const { return menu_item_info_.dwTypeData; }

	HMENU GetHandle() { return hMenu_; }
	int IndexOf( const WindowMenuItem* item );
	int GetMenuIndex() const;
	void SetMenuIndex( int index );

	void SetChecked( bool b );
	bool GetChecked() const {
		return (menu_item_info_.fState & MFS_CHECKED) != 0;
	}
	void SetEnabled( bool b );
	bool GetEnabled() const {
		return (menu_item_info_.fState & MFS_DISABLED) == 0;
	}
	void SetGroupIndex( int index );
	int GetGroupIndex() const {
		return group_no_;
	}
	void SetRadioItem( bool b );
	bool GetRadioItem() const {
		return (menu_item_info_.fType & MFT_RADIOCHECK) != 0;
	}
	void SetShortCut( int key );
	int GetShortCut() const {
		return short_cut_key_;
	}
	void SetVisible( bool b );
	bool GetVisible() const { return is_visible_; }

	const WindowMenuItem* GetParent() const { return parent_; }
	WindowMenuItem* GetParent() { return parent_; }

	void OnClick();

	static void OnClickHandler( int id );
};


#endif
