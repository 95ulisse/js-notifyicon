#pragma once

#include <string>
#include "NotifyIcon.h"

namespace JsNotifyIcon {

    // Static initializer
    void NotifyIcon::Initialize()
    {
        // Registers the class for the tray window
        WNDCLASSEX wx = {};
        wx.lpfnWndProc = NotifyIcon::WindowProcedure;
        wx.lpszClassName = NOTIFYICON_WINDOW_CLASSNAME;
        wx.cbSize = sizeof(WNDCLASSEX);
        RegisterClassEx(&wx);
    }

    // Static disposal function
    void NotifyIcon::Dispose()
    {
        // Unregiters the window class
        UnregisterClass(NOTIFYICON_WINDOW_CLASSNAME, NULL);
    }

    // --------------------------------------------------------------------------

    // Constructor
    NotifyIcon::NotifyIcon(int id) :
        ID(id),
        _isVisible(false),
        _callback(NULL),
        _menu(NULL)
    {
        // Creates the window
        HWND hWnd = CreateWindowEx(0, NOTIFYICON_WINDOW_CLASSNAME, "NotifyIconwindow", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)this);
        this->_windowHandle = hWnd;

        // Creates the notify icon
        NOTIFYICONDATA icon;
        memset( &icon, 0, sizeof( NOTIFYICONDATA ) ) ;
        icon.cbSize = sizeof(NOTIFYICONDATA);
        icon.hWnd = hWnd;
        icon.uID = id;
        icon.uCallbackMessage = WM_NOTIFYICON; //Set up our invented Windows Message
        icon.uFlags = NIF_MESSAGE;
        this->_iconData = std::make_shared<NOTIFYICONDATA>(icon);
    }

    // Destructor
    NotifyIcon::~NotifyIcon()
    {
        if (this->IsVisible())
            this->Hide();

        if (this->_icon)
            DestroyIcon(this->_icon);

        DestroyWindow(this->_windowHandle);
    }

    // Returns the visibility of the icon
    bool NotifyIcon::IsVisible()
    {
        return this->_isVisible;
    }

    // Shows the tray icon
    void NotifyIcon::Show()
    {
        if (this->_isVisible)
            return;

        Shell_NotifyIcon(NIM_ADD, this->_iconData.get());
        this->_isVisible = true;
    }

    // Hides the tray icon
    void NotifyIcon::Hide()
    {
        if (!this->_isVisible)
            return;

        Shell_NotifyIcon(NIM_DELETE, this->_iconData.get());
        this->_isVisible = false;
    }

    // Sets the tooltip
    void NotifyIcon::SetTooltip(const char* tip)
    {
        this->_iconData->uFlags |= NIF_TIP;
        strcpy_s(this->_iconData->szTip, tip);
        this->UpdateIcon();
    }

    // Sets the icon
    void NotifyIcon::SetIcon(HICON icon)
    {
        this->_icon = icon;
        this->_iconData->uFlags |= NIF_ICON;
        this->_iconData->hIcon = icon;
        this->UpdateIcon();
    }

    // Sets the menu
    void NotifyIcon::SetMenu(Menu* menu)
    {
        this->_menu = std::shared_ptr<Menu>(menu);
    }

    // Sets the callback
    void NotifyIcon::SetCallback(NotifyIconCallback cb)
    {
        this->_callback = cb;
    }

    // Shows a balloon mesage
    void NotifyIcon::ShowMessage(const char* title, const char* text, int icon)
    {
        this->_iconData->uFlags |= NIF_INFO;
        strcpy_s(this->_iconData->szInfoTitle, title);
        strcpy_s(this->_iconData->szInfo, text);
        this->_iconData->dwInfoFlags = icon;
        this->UpdateIcon();
        this->_iconData->uFlags &= ~NIF_INFO;
    }

    // ------------------------------------------------------------------------------

    // Updates the icon
    void NotifyIcon::UpdateIcon()
    {
        if (this->_isVisible)
            Shell_NotifyIcon(NIM_MODIFY, this->_iconData.get());
    }

    // Processes the messages received by the hidden window for each icon
    LRESULT CALLBACK NotifyIcon::WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        NotifyIcon* icon = (NotifyIcon*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        // Exits prematurely if this message has not originated from notify icon
        // or if no callback is registered on the NotifyIcon object
        if (message != WM_NOTIFYICON || !icon->_callback)
            return DefWindowProc(hwnd, message, wParam, lParam);

        switch (lParam)
        {
            case WM_MOUSEMOVE:
                icon->_callback(icon, NotifyIconMessage::MouseMove, NULL);
                break;
            case WM_LBUTTONUP:
                icon->_callback(icon, NotifyIconMessage::Click, NULL);
                break;
            case WM_RBUTTONUP:
                icon->_callback(icon, NotifyIconMessage::RClick, NULL);

                // Shows the menu if present
                if (icon->_menu) {
                    icon->_menu->SetHWND(icon->_windowHandle);
                    MenuItem* mi = icon->_menu->Show();
                    if (mi) {
                        icon->_callback(icon, NotifyIconMessage::Command, mi->Command);
                    }
                }

                break;
			case WM_LBUTTONDBLCLK:
				icon->_callback(icon, NotifyIconMessage::DoubleClick, NULL);
				break;
			case WM_RBUTTONDBLCLK:
				icon->_callback(icon, NotifyIconMessage::DoubleRClick, NULL);
				break;
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

}