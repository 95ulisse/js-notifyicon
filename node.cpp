#include <node.h>
#include <v8.h>
#include "ThreadBridge.h"

using namespace v8;

#define THROW_V8_EXCEPTION(msg) \
	return ThrowException(Exception::Error(String::New(msg)));

#define FIRST_ARG_ID() \
	if (args.Length() == 0 || !args[0]->IsInt32()) \
		THROW_V8_EXCEPTION("First argument must be integer"); \
	int id = args[0]->Int32Value();

#define STRING_ARG(index, name) \
	if (args.Length() < index + 1 || !args[index]->IsString()) \
		THROW_V8_EXCEPTION("String argument expected"); \
	String::Utf8Value name##_v8(args[index]->ToString()); \
	char* name = *name##_v8;

#define INT_ARG(index, name) \
	if (args.Length() < index + 1 || !args[index]->IsInt32()) \
		THROW_V8_EXCEPTION("Integer argument expected"); \
	int name = args[index]->Int32Value();

Persistent<Function> _callback;

Handle<Value> SetCallback(const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 1 || !args[0]->IsFunction())
		THROW_V8_EXCEPTION("Expected just one function as parameter");

	_callback = Persistent<Function>::New(Handle<Function>::Cast(args[0]));

	return scope.Close(Undefined());
}

void OnThreadBridgeMessage(int ID, NotifyIconMessage msg) {
	HandleScope scope;

	Local<Value> args[2] = {
		Local<Value>::New(Integer::New(ID))
	};

	switch (msg)
	{
		case NotifyIconMessage::MouseMove:
			args[1] = Local<Value>::New(String::New("mousemove"));
			break;
		case NotifyIconMessage::Click:
			args[1] = Local<Value>::New(String::New("click"));
			break;
		case NotifyIconMessage::RClick:
			args[1] = Local<Value>::New(String::New("rclick"));
			break;
	}

	_callback->Call(Context::GetCurrent()->Global(), 2, args);

}

Handle<Value> CreateNotifyIcon(const Arguments& args) {
    HandleScope scope;
	return scope.Close(Integer::New(ThreadBridge::CreateNotifyIcon()));
}

Handle<Value> SetIcon(const Arguments& args) {
	THROW_V8_EXCEPTION("Not implemented");
}

Handle<Value> SetTooltip(const Arguments& args) {
    HandleScope scope;
	FIRST_ARG_ID();
	STRING_ARG(1, text);
	ThreadBridge::SetTooltip(id, text);
    return scope.Close(Undefined());
}

Handle<Value> Show(const Arguments& args) {
    HandleScope scope;
	FIRST_ARG_ID();
	ThreadBridge::Show(id);
    return scope.Close(Undefined());
}

Handle<Value> Hide(const Arguments& args) {
    HandleScope scope;
	FIRST_ARG_ID();
	ThreadBridge::Hide(id);
    return scope.Close(Undefined());
}

Handle<Value> ShowMessage(const Arguments& args) {
    HandleScope scope;
	FIRST_ARG_ID();
	STRING_ARG(1, title);
	STRING_ARG(2, text);
	INT_ARG(3, icon);
	ThreadBridge::ShowMessage(id, title, text, icon);
    return scope.Close(Undefined());
}

Handle<Value> Dispose(const Arguments& args) {
    HandleScope scope;
	ThreadBridge::Dispose();
	_callback.Dispose();
    return scope.Close(Undefined());
}

void init(Handle<Object> exports) {

    ThreadBridge::Initialize();
	ThreadBridge::SetCallback(OnThreadBridgeMessage);
	
	// Exposes to JavaScript all the methods of the ThreadBridge class
	exports->Set(String::NewSymbol("setCallback"),
		FunctionTemplate::New(SetCallback)->GetFunction());
	exports->Set(String::NewSymbol("createNotifyIcon"),
		FunctionTemplate::New(CreateNotifyIcon)->GetFunction());
	exports->Set(String::NewSymbol("setIcon"),
		FunctionTemplate::New(SetIcon)->GetFunction());
	exports->Set(String::NewSymbol("setTooltip"),
		FunctionTemplate::New(SetTooltip)->GetFunction());
	exports->Set(String::NewSymbol("show"),
		FunctionTemplate::New(Show)->GetFunction());
	exports->Set(String::NewSymbol("hide"),
		FunctionTemplate::New(Hide)->GetFunction());
	exports->Set(String::NewSymbol("showMessage"),
		FunctionTemplate::New(ShowMessage)->GetFunction());
	exports->Set(String::NewSymbol("dispose"),
		FunctionTemplate::New(Dispose)->GetFunction());
}

NODE_MODULE(NotifyIcon, init)