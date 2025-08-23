#include "SensorUtils.h"
#include "Utils.h"

#define DEBUG_DS18B20             // DB18B20 debug
#define DS18B20_TEMP_SENSOR_NDX 0 // Dallas DS18B20 hõmérõ szenzor indexe
#include <OneWire.h>
#define REQUIRESALARMS true // nem kell a DallasTemperature ALARM supportja
#include <DallasTemperature.h>

// Hőmérés
OneWire oneWire(PIN_DS18B20_TEMP_SENSOR);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas nonBlockingDallasTemperatureSensor(&dallasTemp); // Create a new instance of the NonBlockingDallas class

// --- Konstansok ---
#define AD_RESOLUTION 12 // 12 bites az ADC
#define V_REFERENCE 3.3f
#define CONVERSION_FACTOR (1 << AD_RESOLUTION)

// Külső feszültségosztó ellenállásai a VBUS méréshez
#define VBUS_DIVIDER_R1 15.0f
#define VBUS_DIVIDER_R2 4.7f
#define EXTERNAL_VBUSDIVIDER_RATIO ((VBUS_DIVIDER_R1 + VBUS_DIVIDER_R2) / VBUS_DIVIDER_R2) // Feszültségosztó aránya

// Statikus változók definíciói
float SensorUtils::externalTemperatureValue = 0.0f;
NonBlockingDallas *SensorUtils::nonBlockingDallasTemperatureSensor = nullptr;

/**
 *
 */
SensorUtils::SensorUtils() : vBusExtValue(0.0f), vBusExtLastRead(0), vBusExtValid(false), coreTemperatureValue(0.0f), coreTemperatureLastRead(0), coreTemperatureValid(false) {

    // Belső AD felbontás beállítása
    analogReadResolution(AD_RESOLUTION);
}

void SensorUtils::init() {
    // --------------------------------------------------------------------------------------------------------
    // Figyelem!!!
    // NEM LEHET a loop() és a Non-blocking Dallas között hosszú idő, mert a lib leáll az idő méréssel.
    //  Szerintem hibás a matek a NonBlockingDallas::waitNextReading() metódusban
    // --------------------------------------------------------------------------------------------------------

    nonBlockingDallasTemperatureSensor = new NonBlockingDallas(new DallasTemperature(new OneWire(PIN_DS18B20_TEMP_SENSOR)));

    //  Hőmérséklet szenzor inicializálása
    //  Non-blocking Dallas temperature sensor - 1500ms ajánlott 12 bites felbontáshoz
    nonBlockingDallasTemperatureSensor->begin(NonBlockingDallas::resolution_12, 1500);

    // Non-blocking Dallas temperature sensor callback-ek
    nonBlockingDallasTemperatureSensor->onTemperatureChange(handleTemperatureChange);

    // Azonnal le is kérjük a hőmérsékletet
    nonBlockingDallasTemperatureSensor->requestTemperature();
}

/**
 * @brief callback function a hőmérséklet változás kezelésére
 * CSAK akkor hívódik meg, ha a hőmérséklet két ÉRVÉNYES szenzorleolvasás között megváltozik.
 * @param deviceIndex A szenzor eszköz indexe
 * @param temperatureRAW A nyers hőmérséklet érték
 */
void SensorUtils::handleTemperatureChange(int deviceIndex, int32_t temperatureRAW) {
    externalTemperatureValue = nonBlockingDallasTemperatureSensor->rawToCelsius(temperatureRAW);
    DEBUG("handleTemperatureChange -> temperature: %s °C\n", Utils::floatToString(externalTemperatureValue, 2).c_str());
}

/**
 * ADC olvasás és VBUS feszültség kiszámítása KÜLSŐ osztóval
 * @return A VBUS mért feszültsége Voltban.
 */
float SensorUtils::readVBusExternal() {
    unsigned long currentTime = millis();

    // Ellenőrizzük, hogy a cache még érvényes-e
    if (vBusExtValid && (currentTime - vBusExtLastRead < SENSORS_CACHE_TIMEOUT_MS)) {
        return vBusExtValue;
    }

    // Cache lejárt vagy nem érvényes, új mérés
    float voltageOut = (analogRead(PIN_VBUS_EXTERNAL_MEASURE_INPUT) * V_REFERENCE) / CONVERSION_FACTOR;
    float vBusExtVoltage = voltageOut * EXTERNAL_VBUSDIVIDER_RATIO;

    // Cache frissítése
    vBusExtValue = vBusExtVoltage;
    vBusExtLastRead = currentTime;
    vBusExtValid = true;

    return vBusExtVoltage;
}

/**
 * @brief Kiolvassa a processzor hőmérsékletét
 * @details A hőmérsékletet az ADC0 bemeneten keresztül olvassa, és cache-eli az értéket.
 * @return A processzor hőmérséklete Celsius fokban
 */
float SensorUtils::readCoreTemperature() {
    unsigned long currentTime = millis();
    if (coreTemperatureValid && (currentTime - coreTemperatureLastRead < SENSORS_CACHE_TIMEOUT_MS)) {
        return coreTemperatureValue;
    }
    float temperature = analogReadTemp(); // A4
    coreTemperatureValue = temperature;
    coreTemperatureLastRead = currentTime;
    coreTemperatureValid = true;
    return temperature;
}

/**
 * @brief visszaadja a külső hőmérsékletet
 */
float SensorUtils::readExternalTemperature() { return externalTemperatureValue; }

/**
 * Loop
 */
void SensorUtils::loop() {

    // Update temperature
    nonBlockingDallasTemperatureSensor->update();
}
