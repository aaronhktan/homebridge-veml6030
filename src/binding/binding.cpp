extern "C" {
#include "veml6030.h"
}

#include "binding_utils.h"

#include <napi.h>

#include <string>

Napi::Object init(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string spiAdaptor{"/dev/i2c-3"}; 
  if (info.Length() >= 1) {
    spiAdaptor = static_cast<std::string>(info[0].As<Napi::String>());
  }

  uint8_t address = 0x48;
  if (info.Length() >= 2) {
    address = static_cast<uint32_t>(info[1].As<Napi::Number>()) & 0xFF;
  }

  int err = VEML6030_init(spiAdaptor.c_str(), address);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not initialize VEML6030 module; are you using the right port?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "returnCode"), Napi::Number::New(env, err));
  return returnObject;
}

Napi::Object deinit(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int err = VEML6030_deinit();
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not deinitialize VEML6030 module; are you using the right port?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "returnCode"), Napi::Number::New(env, err));
  return returnObject;
}

Napi::Object access_als(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  double als;
  int err = VEML6030_access_als(&als);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not access ALS measureed by VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "als"), Napi::Number::New(env, als));
  return returnObject;
}

Napi::Object access_white(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  double white;
  int err = VEML6030_access_white(&white);
  if (err) {
    return BindingUtils::errFactory(env, err,
        "Could not access white lux measured by the VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "white"), Napi::Number::New(env, white));
  return returnObject;
}

Napi::Object access_config(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint16_t gain, it, pers, sd;
  int err = VEML6030_access_config(&gain, &it, &pers, &sd);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not get config from VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "gain"), Napi::Number::New(env, gain));
  returnObject.Set(Napi::String::New(env, "integration_time"), Napi::Number::New(env, it));
  returnObject.Set(Napi::String::New(env, "persistence"), Napi::Number::New(env, pers));
  returnObject.Set(Napi::String::New(env, "shutdown"), Napi::Number::New(env, sd));
  return returnObject;
}

Napi::Object access_ps(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint16_t refresh, psm;
  int err = VEML6030_access_ps(&refresh, &psm);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not get power-saving settings from VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "power_saving_mode"), Napi::Number::New(env, refresh));
  returnObject.Set(Napi::String::New(env, "power_saving_enable"), Napi::Number::New(env, psm));
  return returnObject;
}

Napi::Object set_config(const Napi::CallbackInfo &info) {
  uint16_t gain = static_cast<uint32_t>(info[0].As<Napi::Number>()) & 0xFFFF;
  uint16_t integration_time = static_cast<uint32_t>(info[1].As<Napi::Number>()) & 0xFFFF;
  uint16_t persistence = static_cast<uint32_t>(info[2].As<Napi::Number>()) & 0xFFFF;
  uint16_t shutdown = static_cast<uint32_t>(info[3].As<Napi::Number>()) & 0xFFFF; 
  Napi::Env env = info.Env();

  int err = VEML6030_set_config(gain, integration_time, persistence, shutdown);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not set config for VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "returnCode"), Napi::Number::New(env, err));
  return returnObject;
}

Napi::Object set_ps(const Napi::CallbackInfo &info) {
  uint16_t psm = static_cast<uint32_t>(info[0].As<Napi::Number>()) & 0xFFFF;
  uint16_t psm_en = static_cast<uint32_t>(info[1].As<Napi::Number>()) & 0xFFFF;
  Napi::Env env = info.Env();

  int err = VEML6030_set_ps(psm, psm_en);
  if (err) {
    return BindingUtils::errFactory(env, err,
      "Could not set power-saving modes on the VEML6030 module; did you run init() first?");
  }

  Napi::Object returnObject = Napi::Object::New(env);
  returnObject.Set(Napi::String::New(env, "returnCode"), Napi::Number::New(env, err));
  return returnObject;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "init"),
              Napi::Function::New(env, init));
  exports.Set(Napi::String::New(env, "deinit"),
              Napi::Function::New(env, deinit));
  exports.Set(Napi::String::New(env, "accessALS"),
              Napi::Function::New(env, access_als));
  exports.Set(Napi::String::New(env, "accessWhite"),
              Napi::Function::New(env, access_white));
  exports.Set(Napi::String::New(env, "accessConfig"),
              Napi::Function::New(env, access_config));
  exports.Set(Napi::String::New(env, "accessPowerSaving"),
              Napi::Function::New(env, access_ps));
  exports.Set(Napi::String::New(env, "setConfig"),
              Napi::Function::New(env, set_config));
  exports.Set(Napi::String::New(env, "setPowerSaving"),
              Napi::Function::New(env, set_ps));
  return exports;
}

NODE_API_MODULE(homebridgeveml6030, Init)

