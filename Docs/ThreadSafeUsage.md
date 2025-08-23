# Thread-Safe SatelliteDb Használat RP2040 Dual-Core Architektúrában

## Architektúra áttekintés

- **Core1**: GPS adatok olvasása és feldolgozása (`GpsManager::loop()`)
- **Core0**: UI megjelenítés és felhasználói interakció
- **Probléma**: Race condition, ha mindkét core egyszerre fér hozzá a `SatelliteDb`-hez

## Thread-Safe Megoldás

### 1. SatelliteDb új metódusai

```cpp
// Thread-safe snapshot készítése UI számára
std::vector<SatelliteDb::SatelliteData> getSnapshotForUI() const;

// Thread-safe műholdak számának lekérdezése
uint8_t countSatsForUI() const;
```

### 2. GpsManager wrapper metódusai

```cpp
// Core0-ból biztonságosan hívható
std::vector<SatelliteDb::SatelliteData> getSatelliteSnapshotForUI() const;
uint8_t getSatelliteCountForUI() const;
```

## Használati példa Core0-ban (UI)

```cpp
// ScreenMain.cpp vagy más UI komponensben
void ScreenMain::updateSatelliteDisplay() {
    // Thread-safe hozzáférés a globális gpsManager-en keresztül
    if (gpsManager != nullptr) {
        
        // Műholdak számának lekérdezése
        uint8_t satCount = gpsManager->getSatelliteCountForUI();
        
        // Teljes műhold adatok snapshot-ja
        auto satellites = gpsManager->getSatelliteSnapshotForUI();
        
        // UI frissítése a snapshot adatokkal
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Satellites: " + String(satCount), 10, 10);
        
        int y = 30;
        for (const auto& sat : satellites) {
            String satInfo = "PRN:" + String(sat.prn) + 
                           " SNR:" + String(sat.snr) + 
                           " El:" + String(sat.elevation);
            tft.drawString(satInfo, 10, y);
            y += 20;
        }
    }
}
```

## Miért biztonságos?

1. **Gyors másolás**: `getSnapshotForUI()` egy gyors `std::vector` másolatot készít
2. **Const metódusok**: Nem módosítják az eredeti adatokat
3. **STL container**: `std::list` size() művelete általában atomic
4. **Separate data**: A snapshot külön memóriaterületen van

## Core1 továbbra is írhat

```cpp
// GpsManager::loop() - Core1-ben fut
void GpsManager::processGSVMessages() {
    // Core1 továbbra is szabadon módosíthatja a satelliteDb-t
    satelliteDb.addOrUpdate(prn, elevation, azimuth, snr);
    satelliteDb.removeOldSatellites();
    // ...
}
```

## Teljesítmény optimalizálás

- **Ne hívd minden frame-ben**: Cache-eld az adatokat UI oldalon
- **Csak változáskor frissíts**: Használj dirty flag-et
- **Minimális másolás**: Csak a szükséges adatokat kérd le

```cpp
// Optimalizált UI frissítés
static uint8_t lastSatCount = 0;
static unsigned long lastUpdate = 0;

void updateSatelliteDisplayOptimized() {
    unsigned long now = millis();
    
    // Csak 500ms-ként frissítsen
    if (now - lastUpdate > 500) {
        uint8_t currentSatCount = gpsManager->getSatelliteCountForUI();
        
        // Csak ha változott a szám
        if (currentSatCount != lastSatCount) {
            auto satellites = gpsManager->getSatelliteSnapshotForUI();
            // UI frissítés...
            lastSatCount = currentSatCount;
        }
        
        lastUpdate = now;
    }
}
```

Ez a megoldás biztosítja, hogy:
- ✅ Nincs race condition
- ✅ Core1 szabadon írhat GPS adatokat  
- ✅ Core0 biztonságosan olvashat UI számára
- ✅ Minimális teljesítményvesztés
- ✅ Modern C++ best practices
