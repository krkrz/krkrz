

#ifndef __WINDOW_MENU_H__
#define __WINDOW_MENU_H__

#include <string>
#include <vector>
#include <window.h>


namespace TML {
class Window;

class WindowMenuItem {
protected:
	MENUITEMINFO menu_item_info_;
	HMENU hMenu_;

private:
	std::string caption_;
	std::string short_cut_key_;

	bool has_checked_;
	bool is_enabled_;
	bool is_radio_;
	bool is_visible_;

	int group_no_;

	int GetIndex() const;
	void SetIndex( int index );

	WindowMenuItem* parent_;
	WindowMenuItem* root_;
	Window* owner_window_;
	std::vector<WindowMenuItem*> children_;
public:
	WindowMenuItem( Window* window, std::string& caption );
	void add( WindowMenuItem* item );
	void insert( WindowMenuItem* item, int index );
	void popup( int flags, int x, int y );
	void remove( WindowMenuItem* item );

	void OnClick();

	static void OnClickHandler();
};

class WindowMenu : public WindowMenuItem {
public:
	HMENU GetHandle() { return hMenu_; }
};

} // namespace

#endif
