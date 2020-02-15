#pragma once

#include <Arduino.h>
#include <wasm3.h>

class Script
{
private:
    /* data */
    IM3Environment _env;
    IM3Runtime _runtime;
    IM3Module _module;

    IM3Function _setup, _loop;

public:
    Script();
    ~Script();

    void Setup(const String& filename);

    void Loop();
};