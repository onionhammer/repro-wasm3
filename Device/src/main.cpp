#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "Script.hpp"

Script script;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Ensure filesystem works
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    script.Setup("/blink.wasm");
}

void loop() {
    script.Loop();
    delay(1000);
}
