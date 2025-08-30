#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>
#include <OneWire.h>

#include "defines.h"
#include "pins.h"

// Cache konstans
#define SENSORS_CACHE_TIMEOUT_MS (1 * 1000) // 1 másodperc a cache idő

// Cache struktúra
class SensorUtils {
  private:
    float vBusExtValue;            // VBUS KÜLSŐ feszültség utolsó mért értéke (Volt)
    unsigned long vBusExtLastRead; // VBUS utolsó mérésének időpontja (ms)
    bool vBusExtValid;             // VBUS cache érvényessége

    float coreTemperatureValue;            // Hőmérséklet utolsó mért értéke (Celsius)
    unsigned long coreTemperatureLastRead; // Hőmérséklet utolsó mérésének időpontja (ms)
    bool coreTemperatureValid;             // Hőmérséklet cache érvényessége

    // DS18B20
    static volatile float externalTemperatureValue; // Külső hőmérséklet utolsó mért értéke (Celsius)
    static void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW);

  public:
    SensorUtils();

    /**
     * Inicializálja az osztályt
     */
    void init();

    /**
     * ADC olvasás és VBUS feszültség kiszámítása KÜLSŐ osztóval
     * @return A VBUS mért feszültsége Voltban.
     */
    float readVBusExternal();

    /**
     * Kiolvassa a processzor hőmérsékletét
     * @return processzor hőmérséklete Celsius fokban
     */
    float readCoreTemperature();

    /**
     * Kiolvassa a külső hőmérsékletét
     * @return külső hőmérséklete Celsius fokban
     */
    float readExternalTemperature();

    /**
     * Loop
     */
    void loop();
};

// Globális cache példány
extern SensorUtils sensorUtils;
