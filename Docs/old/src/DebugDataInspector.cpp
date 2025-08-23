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
    DEBUG("  gpsTrafiAlarmEnabled: %s\n", configData.gpsTrafiAlarmEnabled ? "true" : "false");
    DEBUG("  gpsTrafiAlarmDistance: %u\n", configData.gpsTrafiAlarmDistance);
    DEBUG("====================\n");
#endif
}
