#pragma once

#include <uv.h>
#include <queue>
#include "NotifyIcon.h"

#define TB_MESSAGE_CREATE       (WM_NOTIFYICON + 1)
#define TB_MESSAGE_SETICON      (WM_NOTIFYICON + 2)
#define TB_MESSAGE_SETTOOLTIP   (WM_NOTIFYICON + 3)
#define TB_MESSAGE_SETMENU      (WM_NOTIFYICON + 4)
#define TB_MESSAGE_SHOW         (WM_NOTIFYICON + 5)
#define TB_MESSAGE_HIDE         (WM_NOTIFYICON + 6)
#define TB_MESSAGE_SHOWMESSAGE  (WM_NOTIFYICON + 7)
#define TB_MESSAGE_DISPOSE      (WM_NOTIFYICON + 8)

namespace JsNotifyIcon {

    typedef void (*ThreadBridgeCallback)(int ID, NotifyIconMessage message, void* param);

    class ThreadBridge
    {
        public:

            static void Initialize();
            static void Dispose();

            static void SetCallback(ThreadBridgeCallback func);
            static int CreateNotifyIcon();
            static void SetIcon(int id, HICON icon);
            static void SetTooltip(int id, const char* text);
            static void SetMenu(int id, Menu* menu);
            static void Show(int id);
            static void Hide(int id);
            static void ShowMessage(int id, const char* title, const char* text, int icon);
            static void Dispose(int id);

        private:

            struct _ShowMessageParam {
                const char* title;
                const char* text;
                int icon;
            };

            struct _CallbackQueueItem {
                int ID;
                NotifyIconMessage message;
                void* param;
            };

            static int _idCounter;
            static uv_thread_t _thread;
            static int _threadId;
            static uv_async_t _async;
            static uv_rwlock_t _queueLock;
            static std::queue<_CallbackQueueItem*>* _callbackQueue;
            static ThreadBridgeCallback _callback;
            static void ThreadProcedure(void* arg);
            static void OnThreadMessage(uv_async_t* handle, int status);
            static void OnNotifyIconMessage(NotifyIcon* icon, NotifyIconMessage msg, void* param);
    };

}