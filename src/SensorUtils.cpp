#include "SensorUtils.h"
#include "Utils.h"

// #define DEBUG_DS18B20             // DB18B20 debug
#define DS18B20_TEMP_SENSOR_NDX 0 // Dallas DS18B20 hõmérõ szenzor indexe
#include <OneWire.h>
#define REQUIRESALARMS true // nem kell a DallasTemperature ALARM supportja
#include <DallasTemperature.h>

// --- Konstansok ---
#define AD_RESOLUTION 12 // 12 bites az ADC
#define V_REFERENCE 3.3f
#define CONVERSION_FACTOR (1 << AD_RESOLUTION)

// Külső feszültségosztó ellenállásai a VBUS méréshez
#define VBUS_DIVIDER_R1 15.0f
#define VBUS_DIVIDER_R2 4.7f
#define EXTERNAL_VBUSDIVIDER_RATIO ((VBUS_DIVIDER_R1 + VBUS_DIVIDER_R2) / VBUS_DIVIDER_R2) // Feszültségosztó aránya

// Külső feszültségosztó ellenállásai a VBUS méréshez
#define VSYS_DIVIDER_R1 4.7f
#define VSYS_DIVIDER_R2 10.0f
#define EXTERNAL_VSYS_DIVIDER_RATIO ((VSYS_DIVIDER_R1 + VSYS_DIVIDER_R2) / VSYS_DIVIDER_R2) // Feszültségosztó aránya

// Dallas DS18B20 hőmérséklet szenzor - STATIKUS OBJEKTUMOK (memóriafragmentáció elkerülése)
static OneWire oneWire(PIN_DS18B20_TEMP_SENSOR);
static DallasTemperature dallasTemp(&oneWire);
static NonBlockingDallas nonBlockingDallasTemp(&dallasTemp);

// Statikus változók definíciói
volatile float SensorUtils::externalTemperatureValue = 0.0f;

/**
 * Konstruktor
 */
SensorUtils::SensorUtils()
    : vBusExtValue(0.0f), vBusExtLastRead(0), vBusExtValid(false),                          //
      vSysExtValue(0.0f), vSysExtLastRead(0), vSysExtValid(false),                          //
      coreTemperatureValue(0.0f), coreTemperatureLastRead(0), coreTemperatureValid(false) { //
    // Belső AD felbontás beállítása a feszültségméréshez
    analogReadResolution(AD_RESOLUTION);
}

/**
 * Inicializálja az osztályt
 */
void SensorUtils::init() {

    // --------------------------------------------------------------------------------------------------------
    // Figyelem!!!
    // NEM LEHET a loop() és a Non-blocking Dallas között hosszú idő, mert a lib leáll az idő méréssel.
    //  Szerintem hibás a matek a NonBlockingDallas::waitNextReading() metódusban
    // --------------------------------------------------------------------------------------------------------

    // Hőmérséklet szenzor inicializálása - 12 bites felbontás, 1500ms olvasási ciklus
    nonBlockingDallasTemp.begin(NonBlockingDallas::resolution_12, 1500);

    // Callback beállítása
    nonBlockingDallasTemp.onTemperatureChange(handleTemperatureChange);

    // Azonnal le is kérjük a hőmérsékletet
    nonBlockingDallasTemp.requestTemperature();
}

/**
 * @brief callback function a hőmérséklet változás kezelésére
 * CSAK akkor hívódik meg, ha a hőmérséklet két ÉRVÉNYES szenzorleolvasás között megváltozik.
 * @param deviceIndex A szenzor eszköz indexe
 * @param temperatureRAW A nyers hőmérséklet érték
 */
void SensorUtils::handleTemperatureChange(int deviceIndex, int32_t temperatureRAW) {
    externalTemperatureValue = nonBlockingDallasTemp.rawToCelsius(temperatureRAW);
    // char tempBuffer[16];
    // Utils::floatToString(externalTemperatureValue, 2, tempBuffer, sizeof(tempBuffer));
    // DEBUG("SensorUtils::handleTemperatureChange -> temperature: %s °C\n", tempBuffer);
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
    if (vBusExtVoltage > 1.0f) {
        vBusExtVoltage += 0.6f; // Ha 1V-nál nagyobb a mért feszültség, akkor a D1 dióda nyitófeszültségét hozzá kell adni
    }

    // Cache frissítése
    vBusExtValue = vBusExtVoltage;
    vBusExtLastRead = currentTime;
    vBusExtValid = true;

    return vBusExtVoltage;
}

/**
 * ADC olvasás és VSYS feszültség kiszámítása KÜLSŐ osztóval
 * @return A VSYS mért feszültsége Voltban.
 */
float SensorUtils::readVSysExternal() {
    unsigned long currentTime = millis();

    // Ellenőrizzük, hogy a cache még érvényes-e
    if (vSysExtValid && (currentTime - vSysExtLastRead < SENSORS_CACHE_TIMEOUT_MS)) {
        return vSysExtValue;
    }

    // Cache lejárt vagy nem érvényes, új mérés
    float voltageOut = (analogRead(PIN_VSYS_EXTERNAL_MEASURE_INPUT) * V_REFERENCE) / CONVERSION_FACTOR;
    float vSysExtVoltage = voltageOut * EXTERNAL_VSYS_DIVIDER_RATIO + 0.6f; // A D1 védő dióda nyitási feszültségével kompenzálva a mért feszültséget

    // Cache frissítése
    vSysExtValue = vSysExtVoltage;
    vSysExtLastRead = currentTime;
    vSysExtValid = true;

    return vSysExtVoltage;
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

    float temperature = analogReadTemp();
    coreTemperatureValue = temperature;
    coreTemperatureLastRead = currentTime;
    coreTemperatureValid = true;

    return temperature;
}

/**
 * @brief visszaadja a külső hőmérsékletet
 */
float SensorUtils::readExternalTemperature() {
    // Itt nem kell cache-t használni, mert csak a változáskor frissül az externalTemperatureValue értéke
    return externalTemperatureValue;
}

/**
 * Loop - TIMEOUT VÉDELEMMEL
 */
void SensorUtils::loop() {
    static uint32_t lastUpdate = 0;

    // Max 10Hz frissítés (100ms minimális idő)
    if (millis() - lastUpdate >= 100) {
        lastUpdate = millis();
        nonBlockingDallasTemp.update();
    }
}
