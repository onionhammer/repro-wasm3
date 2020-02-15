#include "Script.hpp"

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

/*
 * Wasm3 - high performance WebAssembly interpreter written in C.
 * Copyright © 2020 Volodymyr Shymanskyy, Steven Massey.
 * All rights reserved.
 */

#include <wasm3.h>
#include <m3_api_defs.h>
#include <m3_env.h>

#define WASM_STACK_SLOTS    1024
#define NATIVE_STACK_SIZE   (32*1024)

// For (most) devices that cannot allocate a 64KiB wasm page
#define WASM_MEMORY_LIMIT   4096

//////////////////////////
// Declare device API

m3ApiRawFunction(m3_arduino_millis) {
    m3ApiReturnType (uint32_t)
    m3ApiReturn(millis());
}

m3ApiRawFunction(m3_arduino_delay) {
    m3ApiGetArg (uint32_t, ms)

    // You can also trace API calls
    //Serial.print("api: delay "); Serial.println(ms);

    delay(ms);

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_pinMode) {
    m3ApiGetArg(uint32_t, pin)
    m3ApiGetArg(uint32_t, mode)

    pinMode(pin, mode);

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_digitalWrite) {
    m3ApiGetArg(uint32_t, pin)
    m3ApiGetArg(uint32_t, value)

    digitalWrite(pin, value);

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_print) {
    m3ApiGetArgMem  (const uint8_t *, buf)
    m3ApiGetArg     (uint32_t,        len)

    Serial.write(buf, len);
    m3ApiSuccess();
}

#define FATAL(func, msg) { Serial.print("Fatal: " func " "); Serial.println(msg); return; }

M3Result LinkDeviceLibrary(IM3Module module) {
    const auto device = "device";

    m3_LinkRawFunction(module, device, "millis", "i()", &m3_arduino_millis);
    m3_LinkRawFunction(module, device, "delay", "v(i)", &m3_arduino_delay);
    m3_LinkRawFunction(module, device, "pinMode", "v(ii)", &m3_arduino_pinMode);
    m3_LinkRawFunction(module, device, "digitalWrite", "v(ii)", &m3_arduino_digitalWrite);
    m3_LinkRawFunction(module, device, "print", "v(*i)", &m3_arduino_print);

    return m3Err_none;
}

//////////////////////////

Script::Script() {}

void Script::Setup(const String& filename) {
    // Load WASM script
    auto app_wasm_file = SPIFFS.open(filename, "r");
    auto app_wasm_len = app_wasm_file.size();
    byte app_wasm[app_wasm_len];
    app_wasm_file.readBytes((char *)app_wasm, app_wasm_len);
    app_wasm_file.close();

    // Create a new environment, runtime
    _env = m3_NewEnvironment();
    if (!_env) FATAL("NewEnvironment", "failed");

    _runtime = m3_NewRuntime(_env, WASM_STACK_SLOTS, NULL);
    if (!_runtime) FATAL("NewRuntime", "failed");

    #ifdef WASM_MEMORY_LIMIT
    _runtime->memoryLimit = WASM_MEMORY_LIMIT;
    #endif
    
    // Parse the wasm module
    auto result = m3_ParseModule (_env, &_module, app_wasm, app_wasm_len-1);
    if (result) FATAL("ParseModule", result);
    
    result = m3_LoadModule(_runtime, _module);
    if (result) FATAL("LoadModule", result);

    // Link device library
    result = LinkDeviceLibrary(_module);
    if (result) FATAL("LinkDeviceLibrary", result);

    result = m3_FindFunction (&_setup, _runtime, "_setup");
    if (result) FATAL("FindFunction: setup", result);

    result = m3_FindFunction (&_loop, _runtime, "_loop");
    if (result) FATAL("FindFunction: loop", result);

    // Run setup function
    const char* i_argv[1] = { NULL };
    result = m3_CallWithArgs(_setup, 0, i_argv);
    
    if (result) {
        M3ErrorInfo info;
        m3_GetErrorInfo(_runtime, &info);
        Serial.print("Error: ");
        Serial.print(result);
        Serial.print(" (");
        Serial.print(info.message);
        Serial.println(")");
    }
}

Script::~Script() {
    if (_module)
        m3_FreeModule(_module);

    if (_runtime)
        m3_FreeRuntime(_runtime);

    if (_env)
        m3_FreeEnvironment(_env);
}

void Script::Loop() {
    // Run loop function
    const char* i_argv[1] = { NULL };
    auto result = m3_CallWithArgs(_loop, 0, i_argv);
    
    if (result) {
        M3ErrorInfo info;
        m3_GetErrorInfo (_runtime, &info);
        Serial.print("Error: ");
        Serial.print(result);
        Serial.print(" (");
        Serial.print(info.message);
        Serial.println(")");
    }
}