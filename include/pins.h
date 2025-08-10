#ifndef __PINS_H__
#define __PINS_H__

// Pin definitions

// Serial1 piout átdefiniálása a PICO ZERO-hoz
// Meg kell hívni a Serial1.setRX(PIN_SERIAL1_RX_NEW) és a Serial1.setTX(PIN_SERIAL1_TX_NEW)-et a Serial1.begin(9600) előtt
#define PIN_SERIAL1_TX_NEW (12u)
#define PIN_SERIAL1_RX_NEW (13u)

// TFT háttérvilágítás
#define PIN_TFT_BACKGROUND_LED 1
#define PIN_LIGHT_SENSOR A1

#define PIN_TEMP_SENSOR 8
#define PIN_BATTERRY_MEASURE A0
#define PIN_BUZZER 9

#endif // __PINS_H__