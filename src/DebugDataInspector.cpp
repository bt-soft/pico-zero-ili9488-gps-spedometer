#include "DebugDataInspector.h"

#include "Config.h"
#include "defines.h"
#include "utils.h"

/**
 * @brief Kiírja a Config struktúra tartalmát a soros portra.
 * @param config A Config objektum.
 */
void DebugDataInspector::printConfigData(const Config_t &configData) {
#ifdef __DEBUG
    DEBUG("=== DebugDataInspector -> Config Data ===\n");
    DEBUG("  tftCalibrateData: [%u, %u, %u, %u, %u]\n", configData.tftCalibrateData[0], configData.tftCalibrateData[1], configData.tftCalibrateData[2], configData.tftCalibrateData[3], configData.tftCalibrateData[4]);
    DEBUG("  tftManualBrightnessValue: %u\n", configData.tftManualBrightnessValue);
    DEBUG("  tftAutoBrightnessActive: %s\n", configData.tftAutoBrightnessActive ? "true" : "false");
    DEBUG("  beeperEnabled: %s\n", configData.beeperEnabled ? "true" : "false");
    DEBUG("  gpsTraffiAlarmEnabled: %s\n", configData.gpsTraffiAlarmEnabled ? "true" : "false");
    DEBUG("  gpsTraffiAlarmDistance: %u\n", configData.gpsTraffiAlarmDistance);
    DEBUG("  gpsTraffiSirenAlarmEnabled: %s\n", configData.gpsTraffiSirenAlarmEnabled ? "true" : "false");
    DEBUG("  debugGpsSerialOnInternalFastLed: %s\n", configData.debugGpsSerialOnInternalFastLed ? "true" : "false");
    DEBUG("  debugGpsSerialData: %s\n", configData.debugGpsSerialData ? "true" : "false");
    DEBUG("  debugGpsSatellitesDatabase: %s\n", configData.debugGpsSatellitesDatabase ? "true" : "false");

    DEBUG("====================\n");
#endif
}
