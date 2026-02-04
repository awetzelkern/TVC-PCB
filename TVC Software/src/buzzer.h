#pragma once
#include <Arduino.h>
#include "pins.h"
// Header only file of buzzer beeps

void BUZZ_STARTUP() {
    tone(BUZZER, 3000, 100);
    delay(100);
    tone(BUZZER, 3200, 100);
    delay(100);
    tone(BUZZER, 3400, 100);
    delay(100);
    tone(BUZZER, 3600, 100);
    delay(100);
    tone(BUZZER, 3800, 100);
}

void BUZZ_FAIL_MEM() {
    tone(BUZZER, 3000, 100);
    delay(200);
    tone(BUZZER, 3000, 100);
    delay(200);
}

void BUZZ_FAIL_LORA() {
    tone(BUZZER, 3000, 100);
    delay(100);
}

void BUZZ_FAIL_TRANSMIT() {
    tone(BUZZER, 3000, 50);
}

void BUZZ_LAUNCH() {
    tone(BUZZER, 5200, 350);
}

void BUZZ_LANDED() {
    tone(BUZZER, 4000, 500);
}

