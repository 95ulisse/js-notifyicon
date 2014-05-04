#include "Menu.h"

namespace JsNotifyIcon {

    // Constructor
    Menu::Menu():
        Children()
    {
        _hmenu = ::CreatePopupMenu();
    }

    // Destructor
    Menu::~Menu()
    {
        DestroyMenu(_hmenu);
    }

    // Sets the window handle
    void Menu::SetHWND(HWND hwnd) {
        _hwnd = hwnd;
    }

    // Shows the menu and returns the clicked item
    MenuItem* Menu::Show()
    {
        // Makes sure that the menu reflects the changes in the vector
        UpdateMenu();

        // Current cursor position
        POINT p;
        GetCursorPos(&p);

        // Shows the popup menu
        unsigned int itemId = TrackPopupMenuEx(_hmenu, TPM_RETURNCMD | TPM_NONOTIFY, p.x, p.y, _hwnd, NULL);
        if (itemId == 0)
            return NULL;
        else
            return FindMenuItem(itemId);
    }

    // Rebuilds the system menu
    void Menu::UpdateMenu()
    {
        // Clears the underlying system menu
        int count = GetMenuItemCount(_hmenu);
        for (int i = 0; i < count; i++)
            RemoveMenu(_hmenu, 0, MF_BYPOSITION);

        // Rebuilds the menu
        for (auto it = Children.begin(); it < Children.end(); ++it) {
            if (it->ChildMenu) {
                it->ChildMenu->UpdateMenu();
                AppendMenu(_hmenu, MF_STRING | MF_POPUP, (UINT_PTR)it->ChildMenu->_hmenu, it->Text);
            } else if(it->Separator) {
                AppendMenu(_hmenu, MF_SEPARATOR, NULL, NULL);
            } else {
                AppendMenu(_hmenu, MF_STRING, it->ID, it->Text);
            } 
        }
    }

    // Cycles all its own children looking for the menu item with the given id
    MenuItem* Menu::FindMenuItem(unsigned int id)
    {
        for (auto it = Children.begin(); it < Children.end(); ++it) {
            if (it->ID == id) {
                return &*it;
            } else if (it->ChildMenu) {
                auto inner = it->ChildMenu->FindMenuItem(id);
                if (inner)
                    return inner;
            }
        }
        return NULL;
    }

}