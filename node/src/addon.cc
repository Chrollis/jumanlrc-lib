//
// addon.cc — Node.js N-API native addon for JumanLRC
//
// Uses the C++ API (jumanlrc::lib::JumanppLib) directly via node-addon-api.
//

#include <napi.h>
#include <string>
#include <vector>

#include "jumanlrc_lib.h"

namespace jpp = jumanlrc::lib;

// ─────────────────────────────────────────────────────────────────
// Helper
// ─────────────────────────────────────────────────────────────────
static std::string napiStr(const Napi::Value &val)
{
    return val.As<Napi::String>().Utf8Value();
}

// ─────────────────────────────────────────────────────────────────
// Wrapper — uses jumanlrc::lib::JumanppLib directly
// ─────────────────────────────────────────────────────────────────
class JumanLRCAddon : public Napi::ObjectWrap<JumanLRCAddon>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    JumanLRCAddon(const Napi::CallbackInfo &info);

private:
    jpp::JumanppLib impl_;
    bool initialized_ = false;

    Napi::Value LoadModel(const Napi::CallbackInfo &info);
    Napi::Value Analyze(const Napi::CallbackInfo &info);
    Napi::Value AnalyzeRuby(const Napi::CallbackInfo &info);
    void       SetKatakanaAnnotation(const Napi::CallbackInfo &info);
    void       Destroy(const Napi::CallbackInfo &info);
    Napi::Value GetError(const Napi::CallbackInfo &info);
};

// ─────────────────────────────────────────────────────────────────
// Init
// ─────────────────────────────────────────────────────────────────
Napi::Object JumanLRCAddon::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "JumanLRCAddon", {
        InstanceMethod("loadModel",             &JumanLRCAddon::LoadModel),
        InstanceMethod("analyze",               &JumanLRCAddon::Analyze),
        InstanceMethod("analyzeRuby",           &JumanLRCAddon::AnalyzeRuby),
        InstanceMethod("setKatakanaAnnotation", &JumanLRCAddon::SetKatakanaAnnotation),
        InstanceMethod("destroy",               &JumanLRCAddon::Destroy),
        InstanceMethod("getError",              &JumanLRCAddon::GetError),
    });

    exports.Set("JumanLRCAddon", func);
    return exports;
}

// ─────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────
JumanLRCAddon::JumanLRCAddon(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<JumanLRCAddon>(info) {}

// ─────────────────────────────────────────────────────────────────
// loadModel
// ─────────────────────────────────────────────────────────────────
Napi::Value JumanLRCAddon::LoadModel(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String argument 'modelPath' expected")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string modelPath = napiStr(info[0]);

    int beamSize   = (info.Length() > 1 && info[1].IsNumber()) ? info[1].As<Napi::Number>().Int32Value() : 5;
    int globalBeam = (info.Length() > 2 && info[2].IsNumber()) ? info[2].As<Napi::Number>().Int32Value() : 6;
    int rightBeam  = (info.Length() > 3 && info[3].IsNumber()) ? info[3].As<Napi::Number>().Int32Value() : 5;
    int rightCheck = (info.Length() > 4 && info[4].IsNumber()) ? info[4].As<Napi::Number>().Int32Value() : 1;

    initialized_ = impl_.loadModel(modelPath, beamSize, globalBeam, rightBeam, rightCheck);
    return Napi::Boolean::New(env, initialized_);
}

// ─────────────────────────────────────────────────────────────────
// analyze → Array<{ surface, reading }>
// ─────────────────────────────────────────────────────────────────
Napi::Value JumanLRCAddon::Analyze(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!initialized_)
    {
        Napi::Error::New(env, "Model not loaded. Call loadModel() first.")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String argument 'sentence' expected")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!impl_.analyze(napiStr(info[0])))
        return env.Null();

    const auto &words = impl_.lastResult();
    Napi::Array result = Napi::Array::New(env, words.size());
    for (size_t i = 0; i < words.size(); ++i)
    {
        Napi::Object w = Napi::Object::New(env);
        w.Set("surface", Napi::String::New(env, words[i].surface));
        w.Set("reading", Napi::String::New(env, words[i].reading));
        result.Set(static_cast<uint32_t>(i), w);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────
// analyzeRuby → Array<{ surface, readings[] }>
// ─────────────────────────────────────────────────────────────────
Napi::Value JumanLRCAddon::AnalyzeRuby(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!initialized_)
    {
        Napi::Error::New(env, "Model not loaded. Call loadModel() first.")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String argument 'sentence' expected")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!impl_.analyzeRuby(napiStr(info[0])))
        return env.Null();

    const auto &ruby = impl_.lastRubyResult();
    Napi::Array result = Napi::Array::New(env, ruby.size());
    for (size_t i = 0; i < ruby.size(); ++i)
    {
        Napi::Object unit = Napi::Object::New(env);
        unit.Set("surface", Napi::String::New(env, ruby[i].surface));
        Napi::Array reads = Napi::Array::New(env);
        for (size_t k = 0; k < ruby[i].readings.size(); ++k)
            reads.Set(static_cast<uint32_t>(k), Napi::String::New(env, ruby[i].readings[k]));
        unit.Set("readings", reads);
        result.Set(static_cast<uint32_t>(i), unit);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────
// setKatakanaAnnotation
// ─────────────────────────────────────────────────────────────────
void JumanLRCAddon::SetKatakanaAnnotation(const Napi::CallbackInfo &info)
{
    if (info.Length() > 0 && info[0].IsBoolean())
        impl_.setKatakanaAnnotation(info[0].As<Napi::Boolean>().Value());
}

// ─────────────────────────────────────────────────────────────────
// destroy
// ─────────────────────────────────────────────────────────────────
void JumanLRCAddon::Destroy(const Napi::CallbackInfo &)
{
    initialized_ = false;
}

// ─────────────────────────────────────────────────────────────────
// getError → string
// ─────────────────────────────────────────────────────────────────
Napi::Value JumanLRCAddon::GetError(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), impl_.error());
}

// ─────────────────────────────────────────────────────────────────
// Entry point
// ─────────────────────────────────────────────────────────────────
Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
    return JumanLRCAddon::Init(env, exports);
}

NODE_API_MODULE(jumanlrc_node, InitAll)
