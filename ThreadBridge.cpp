#include "ThreadBridge.h"
#include "NotifyIcon.h"
#include <unordered_map>
#include <Windows.h>
#include <memory>

namespace JsNotifyIcon {

    using namespace std;

    int ThreadBridge::_idCounter = 0;
    uv_thread_t ThreadBridge::_thread;
    int ThreadBridge::_threadId;
    uv_async_t ThreadBridge::_async;
    ThreadBridgeCallback ThreadBridge::_callback;
    std::queue<ThreadBridge::_CallbackQueueItem*>* ThreadBridge::_callbackQueue;
    uv_rwlock_t ThreadBridge::_queueLock;
    void ThreadBridge::Initialize()
    {
        // Starts a whatcher on this thread (v8 thread)
        uv_async_init(uv_default_loop(), &ThreadBridge::_async, ThreadBridge::OnThreadMessage);

        // Initializes the lock that will be used to synchronize data exchange between the threads
        ThreadBridge::_callbackQueue = new std::queue<_CallbackQueueItem*>();
        uv_rwlock_init(&ThreadBridge::_queueLock);

        // Creates a new thread that will run the window message pump
        uv_thread_create(&ThreadBridge::_thread, ThreadBridge::ThreadProcedure, NULL);
        _threadId = GetThreadId(ThreadBridge::_thread);
    }

    void ThreadBridge::Dispose()
    {
        // Posts a WM_QUIT message to the thread and waits for it to terminate
        PostThreadMessage((DWORD)ThreadBridge::_threadId, WM_QUIT, 0, 0);
        uv_thread_join(&ThreadBridge::_thread);

        // Destroys the lock and the queue
        uv_rwlock_destroy(&ThreadBridge::_queueLock);
        delete ThreadBridge::_callbackQueue;

        // Stops the async whatcher
        uv_close((uv_handle_t*) &ThreadBridge::_async, NULL);
    }

    // ---------------------------------------------------------------------------------

    void ThreadBridge::SetCallback(ThreadBridgeCallback func)
    {
        ThreadBridge::_callback = func;
    }

    int ThreadBridge::CreateNotifyIcon()
    {
        int id = ThreadBridge::_idCounter++;
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_CREATE, id, 0);
        return id;
    }

    void ThreadBridge::SetIcon(int id, IconHandle* icon)
    {
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_SETICON, id, (LPARAM)icon);
    }

    void ThreadBridge::SetTooltip(int id, const char* text)
    {
        char* textCopy = strdup(text);
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_SETTOOLTIP, id, (LPARAM)textCopy);
    }

    void ThreadBridge::SetMenu(int id, Menu* menu)
    {
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_SETMENU, id, (LPARAM)menu);
    }

    void ThreadBridge::Show(int id)
    {
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_SHOW, id, 0);
    }

    void ThreadBridge::Hide(int id)
    {
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_HIDE, id, 0);
    }

    void ThreadBridge::ShowMessage(int id, const char* title, const char* text, int icon)
    {
        _ShowMessageParam* param = (_ShowMessageParam*)malloc(sizeof(_ShowMessageParam));
        param->title = strdup(title);
        param->text = strdup(text);
        param->icon = icon;
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_SHOWMESSAGE, id, (LPARAM)param);
    }

    void ThreadBridge::Dispose(int id)
    {
        PostThreadMessage((DWORD)ThreadBridge::_threadId, TB_MESSAGE_DISPOSE, id, 0);
    }


    // ---------------------------------------------------------------------------------

    void ThreadBridge::ThreadProcedure(void* arg)
    {
        // Initializes the NotifyIcon class
        NotifyIcon::Initialize();

        // Map that matches IDs and NotifyIcon objects
        unordered_map<int, unique_ptr<NotifyIcon>> iconMap;

        // Standard windows message pump
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {

            switch (msg.message)
            {
                case TB_MESSAGE_CREATE:
                    iconMap.emplace((int)msg.wParam, unique_ptr<NotifyIcon>(new NotifyIcon((int)msg.wParam)));
                    iconMap[(int)msg.wParam]->SetCallback(ThreadBridge::OnNotifyIconMessage);
                    continue;

                case TB_MESSAGE_DISPOSE:
                    iconMap.erase((int)msg.wParam);
                    continue;
        
                case TB_MESSAGE_SETICON:
                    iconMap[(int)msg.wParam]->SetIcon((IconHandle*)msg.lParam);
                    continue;
        
                case TB_MESSAGE_SETTOOLTIP:
                    iconMap[(int)msg.wParam]->SetTooltip((const char*)msg.lParam);
                    free((char*)msg.lParam);
                    continue;
        
                case TB_MESSAGE_SETMENU:
                    iconMap[(int)msg.wParam]->SetMenu((Menu*)msg.lParam);
                    continue;
        
                case TB_MESSAGE_SHOW:
                    iconMap[(int)msg.wParam]->Show();
                    continue;
        
                case TB_MESSAGE_HIDE:
                    iconMap[(int)msg.wParam]->Hide();
                    continue;
        
                case TB_MESSAGE_SHOWMESSAGE:
                    _ShowMessageParam* param = (_ShowMessageParam*)msg.lParam;
                    iconMap[(int)msg.wParam]->ShowMessage(param->title, param->text, param->icon);
                    free((char*)param->title);
                    free((char*)param->text);
                    free(param);
                    continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // We get here when the thread receives a WM_QUIT message,
        // so that the thread can terminate and everything can be disposed
        NotifyIcon::Dispose();

        // `iconMap` and all the icons will be destroyed at the end of the method
    }

    void ThreadBridge::OnNotifyIconMessage(NotifyIcon* icon, NotifyIconMessage msg, void* param)
    {
        // Creates the message to pass to the v8 thread
        _CallbackQueueItem* item = (_CallbackQueueItem*)malloc(sizeof(_CallbackQueueItem));
        item->ID = icon->ID;
        item->message = msg;
        item->param = param;

        // Stores the item in the queue and wakes up the main thread
        uv_rwlock_wrlock(&ThreadBridge::_queueLock);
            ThreadBridge::_callbackQueue->push(item);
        uv_rwlock_wrunlock(&ThreadBridge::_queueLock);
        uv_async_send(&ThreadBridge::_async);
    }

    void ThreadBridge::OnThreadMessage(uv_async_t* handle, int status)
    {
        // Processes all the elements in the quque
        uv_rwlock_rdlock(&ThreadBridge::_queueLock);
            while(!ThreadBridge::_callbackQueue->empty()) {
                _CallbackQueueItem* item = ThreadBridge::_callbackQueue->front();
                ThreadBridge::_callbackQueue->pop();
                ThreadBridge::_callback(item->ID, item->message, item->param);
                free(item);
            }
        uv_rwlock_rdunlock(&ThreadBridge::_queueLock);
    }

}