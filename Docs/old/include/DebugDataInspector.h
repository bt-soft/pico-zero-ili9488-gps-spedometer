#ifndef __DEBUGDATAINSPECTOR_H
#define __DEBUGDATAINSPECTOR_H

#include <Arduino.h>

struct Config_t; // Forward declare Config_t to break the include cycle

class DebugDataInspector {
  public:
    /**
     * @brief Kiírja a Config struktúra tartalmát a soros portra.
     * @param config A Config objektum.
     */
    static void printConfigData(const Config_t &configData); // Csak a deklaráció marad
};

#endif // __DEBUGDATAINSPECTOR_H