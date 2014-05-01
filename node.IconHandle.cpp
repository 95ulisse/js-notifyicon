#include "node.IconHandle.h"

namespace JsNotifyIcon {
namespace Node {

	using namespace v8;

	Persistent<Function> IconHandle::_constructor;

	void IconHandle::Initialize(Handle<Object> exports)
	{
		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
		tpl->SetClassName(String::NewSymbol("IconHandle"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Prototype
		tpl->PrototypeTemplate()->Set(String::NewSymbol("dispose"),
			FunctionTemplate::New(Dispose)->GetFunction());

		_constructor = Persistent<Function>::New(tpl->GetFunction());

		exports->Set(String::NewSymbol("IconHandle"), _constructor);
	}

	Handle<Value> IconHandle::New(const Arguments& args) {
		HandleScope scope;

		if (args.IsConstructCall()) {
			// Invoked as constructor: `new MyObject(...)`
			IconHandle* obj = new IconHandle();
			obj->Wrap(args.This());
			return args.This();
		} else {
			// Invoked as plain function `MyObject(...)`, turn into construct call.
			return scope.Close(_constructor->NewInstance(0, NULL));
		}
	}
	Handle<Value> IconHandle::New(HICON icon) {
		HandleScope scope;
		Handle<v8::Object> iconHandle = _constructor->NewInstance(0, NULL);
		ObjectWrap::Unwrap<IconHandle>(iconHandle)->SetIconHandle(icon);
		return scope.Close(iconHandle);
	}

	Handle<Value> IconHandle::Dispose(const Arguments& args)
	{
		HandleScope scope;

		IconHandle* obj = ObjectWrap::Unwrap<IconHandle>(args.This());
		delete obj->_iconHandle;

		return scope.Close(Undefined());
	}

	IconHandle::IconHandle() {
	}

	IconHandle::~IconHandle() {
	}

	void IconHandle::SetIconHandle(HICON icon) {
		this->_iconHandle = new JsNotifyIcon::IconHandle(icon);
	}

	JsNotifyIcon::IconHandle* IconHandle::GetIconHandle() {
		return this->_iconHandle;
	}

}
}