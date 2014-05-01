#pragma once

#include <Windows.h>

namespace JsNotifyIcon {

    class IconHandle {
        
        public:
            IconHandle(HICON handle);
            ~IconHandle();
            HICON Handle();

        private:
            HICON _handle;

    };

}