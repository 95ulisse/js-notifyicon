#pragma once

#include <vector>
#include <Windows.h>

namespace JsNotifyIcon {

    class Menu;
    struct MenuItem;

    // Encapsulates a list of MenuItems and provides methods to popup the menu to the user
    class Menu {
        
        public:
            Menu();
            ~Menu();

            std::vector<MenuItem> Children;
            MenuItem* Show();
            void SetHWND(HWND hwnd);

        private:
            HWND _hwnd;
            HMENU _hmenu;
            void UpdateMenu();
            MenuItem* FindMenuItem(unsigned int id);

    };

    // Item of a menu
    struct MenuItem {
        unsigned int ID;
        bool Separator;
        char* Command;
        char* Text;
        Menu* ChildMenu;
    };

}