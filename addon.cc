#include <node.h>
#include <nan.h>
#include <iostream>
#include "lib/libgo.h"
#include "lib/types.h"

using v8::Local;
using v8::Object;
using v8::Value;
using v8::String;
using v8::Number;
using v8::Isolate;
using v8::HandleScope;
using Nan::Maybe;
using Nan::Callback;
using Nan::AsyncWorker;
using Nan::AsyncQueueWorker;
using Nan::FunctionCallbackInfo;
using Nan::Persistent;


class MyWorker : public AsyncWorker {
public:
  // constructor
  MyWorker (GoArgs args, Persistent<v8::Promise::Resolver> *persistentResolver, Callback *callback) :
    AsyncWorker(callback),
    args(args),
    persistentResolver(persistentResolver) {}

  // destructor
  ~MyWorker () {
    delete callback;
    callback = 0;
  }

  void Execute () {
    result = sum(args);
  }

  void HandleOKCallback () {
    Local<Object> res = Nan::New<Object>();
    Nan::Set(res, Nan::New<String>("num").ToLocalChecked(), Nan::New<Number>(result.num));
    Nan::Set(res, Nan::New<String>("str").ToLocalChecked(), Nan::New<String>(result.str).ToLocalChecked());

    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // resolve promise
    auto resolver = Nan::New(*persistentResolver);
    resolver->Resolve(context, res);

    // delete persisted resolver
    persistentResolver->Reset();
    delete persistentResolver;
  }
private:
  GoArgs args;
  GoArgs result;
  Persistent<v8::Promise::Resolver> *persistentResolver;
};

char* ToCString(Local<String> str) {
  Nan::Utf8String value(str);
  return *value;
}

void SumAsync (const FunctionCallbackInfo<Value> &info) {
  Isolate *isolate = info.GetIsolate();
  HandleScope scope(isolate);
  auto context = isolate->GetCurrentContext();

  Local<Object> obj = info[0]->ToObject(context).ToLocalChecked();
  Local<Value> value = Nan::Get(obj, Nan::New<String>("num").ToLocalChecked()).ToLocalChecked();
  Local<Value> value2 = Nan::Get(obj, Nan::New<String>("str").ToLocalChecked()).ToLocalChecked();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  auto persistientResolver = new  Persistent<v8::Promise::Resolver>(resolver);
  auto promise = resolver->GetPromise();

  if (!value->IsNumber() || !value2->IsString()) {
    // if nothing, invoke callback with error
    Local<Value> err = Nan::Error("A number value for 'num' or A string value for 'str' must be supplied");
    resolver->Reject(context, err);
  } else {
    // otherwise perform work
    GoArgs args;
    args.num = value->NumberValue(context).ToChecked();
    args.str = ToCString(value2->ToString(context).ToLocalChecked());
    std::cout << args.num << " is the value for num\n";
    std::cout << args.str << " is the value for str\n";
    AsyncQueueWorker(new MyWorker(args, persistientResolver, 0));
  }

  // return the promise
  info.GetReturnValue().Set(promise);
}

void Init (Local<Object> exports) {
  Nan::SetMethod(exports, "sumAsync", SumAsync);
}

NODE_MODULE(asyncModule, Init)