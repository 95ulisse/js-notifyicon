#pragma once

#include <memory>
#include <Windows.h>
#include <shellapi.h>
#include "IconHandle.h"

// Constants
#define NOTIFYICON_WINDOW_CLASSNAME "NotifyIcon"
#define WM_NOTIFYICON              (WM_USER + 1)

namespace JsNotifyIcon {

    enum class NotifyIconMessage : int {
        MouseMove = 0,
        Click = 1,
        RClick = 2
    };

    class NotifyIcon; // This is just to make the compiler happy
    typedef void (*NotifyIconCallback)(NotifyIcon*, NotifyIconMessage);

    // NotifyIcon class
    class NotifyIcon {

        public:
            NotifyIcon(int id);
            NotifyIcon(const NotifyIcon& other);
            ~NotifyIcon();

            int ID;

            void SetIcon(IconHandle* icon);
            void SetTooltip(const char* text);
            void SetCallback(NotifyIconCallback cb);
            void Show();
            void Hide();
            bool IsVisible();
            void ShowMessage(const char* title, const char* text, int icon);

            static void Initialize();
            static void Dispose();


        private:
            HWND _windowHandle;
            std::shared_ptr<IconHandle> _icon;
            std::shared_ptr<NOTIFYICONDATA> _iconData;
            bool _isVisible;
            NotifyIconCallback _callback;

            void UpdateIcon();
            static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    };

}