#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "Script.hpp"

#define NATIVE_STACK_SIZE   (32*1024)

void wasm_task(void *) {
    Script script;
    script.Setup("/blink.wasm");

    // while (true) {
    //     script.Loop();
    // }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Ensure filesystem works
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    xTaskCreate(&wasm_task, "wasm3", NATIVE_STACK_SIZE, NULL, 5, NULL);
}

void loop() {
    // script.Loop();
    delay(100);
}
