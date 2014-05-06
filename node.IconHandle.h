#pragma once

#include <node.h>
#include <v8.h>

namespace JsNotifyIcon {
namespace Node {

    using namespace v8;

    class IconHandle : public node::ObjectWrap {

        public:
            static void Initialize(Handle<Object> exports);
            static Handle<Value> New(HICON icon);

            IconHandle();
            ~IconHandle();
            HICON GetIconHandle();

        private:
            HICON _iconHandle;
            void SetIconHandle(HICON iconHandle);

            static Handle<Value> New(const Arguments& args);
            static Handle<Value> Dispose(const Arguments& args);
            static Persistent<Function> _constructor;

    };

}
}