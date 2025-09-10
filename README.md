# Pico GPS Sebességmérő Motorokhoz

Ez a projekt egy fejlett Raspberry Pi Pico alapú GPS sebességmérő és navigációs asszisztens, amely egy ILI9488 TFT kijelzőt használ a sebesség, idő, magasság és a közeli sebességmérő kamerák (trafipaxok) megjelenítésére. A készülék kifejezetten motorkerékpár-vezetők számára lett tervezve, hogy segítse őket a biztonságos és szabályos közlekedésben.

![3D render a készülékről](Docs/pictures/pico-gps-spedometer-3d.png)
![3D render a készülékről 2](Docs/pictures/pico-gps-spedometer-3d-2.png)

## Főbb Jellemzők

### 🚀 **Sebességmérés és Navigáció**
- **Valós idejű sebességmérés** GPS adatok alapján nagy pontossággal
- **Lineáris és kör alakú sebességmérő** vizuális megjelenítés választható kijelzési módokkal
- **Digitális és analóg sebesség** kijelzés (km/h)
- **Tengerszint feletti magasság** mérése és megjelenítése
- **GPS koordináták** valós idejű kijelzése
- **Műholdak száma** és jelerősség megjelenítése

### 🕐 **Időkezelés és Pozicionálás**
- **Pontos idő** megjelenítése GPS szinkronizációval
- **Automatikus időzóna** beállítás
- **Téli/nyári időszámítás** támogatása
- **Koordináták megjelenítése** decimális és DMS (fok/perc/másodperc) formátumban

### 🚨 **Trafipax Figyelmeztető Rendszer**
- **Intelligens sebességmérő kamera (trafipax) adatbázis** kezelése
- **Közeledési figyelmeztetés** 800 méteres távolságon belül
- **Vizuális riasztás**: piros háttér közeledéskor, narancssárga távolodáskor
- **Hangos figyelmeztetés** szirénával közeledés esetén
- **Távolság megjelenítése** a legközelebbi trafipaxig
- **Stabil állapotváltás** GPS pontatlansággal szembeni védelemmel

### 🎨 **Megjelenítés és Felhasználói Felület**
- **3.5" ILI9488 480x320 színes TFT kijelző**
- **Anti-flicker sprite alapú** megjelenítés
- **Nagyméretű, jól olvasható** betűtípusok
- **Kontraszt optimalizált** színek nappal és éjszaka
- **Automatikusan állítható háttérvilágítás** (tervezett funkció)

### 💾 **Adatkezelés és Testre Szabás**
- **LittleFS fájlrendszer** a beépített flash memóriában
- **CSV formátumú trafipax adatbázis** egyszerű frissítéshez
- **Konfigurálható figyelmeztetési távolságok**
- **Személyre szabható megjelenítési beállítások**

## Hardver Követelmények

- **Mikrokontroller:** Raspberry Pi Pico vagy Pico Zero (RP2040 alapú)
- **Kijelző:** ILI9488 3.5" 480x320 SPI TFT kijelző érintőképernyővel
- **GPS Modul:** Bármilyen UART-alapú GPS modul (pl. NEO-6M, NEO-8M)
- **Tápellátás:** 5V USB vagy külső tápegység
- **Ház:** 3D nyomtatható ház tervekkel (STL fájlok)
- **Kábelezés:** A kapcsolási rajzok a `kicad/` mappában

## Szoftver Architektúra

- **Fejlesztői környezet:** [PlatformIO](https://platformio.org/) a [Visual Studio Code](https://code.visualstudio.com/)-ban
- **Framework:** Arduino Framework Raspberry Pi Pico támogatással
- **Többmagos feldolgozás:** Dual-core RP2040 kihasználása
  - **Core 0:** Fő alkalmazás logika, kijelzőkezelés
  - **Core 1:** GPS adatfeldolgozás, háttér számítások

### Főbb Könyvtárak
- `bodmer/TFT_eSPI` - Kijelzőkezelés és sprite renderelés
- `mikalhart/TinyGPSPlus` - GPS adatfeldolgozás és koordináta számítások  
- `LittleFS` - Beépített fájlrendszer kezelés
- `NonBlockingDallas` - Hőmérséklet szenzorok (opcionális)
- `FastLED` - LED vezérlés (státusz jelzéshez)

## Telepítés és Üzembe Helyezés

### 1. Fejlesztői Környezet Beállítása
```bash
# Git repository klónozása
git clone https://github.com/bt-soft/pico-zero-ili9488-gps-spedometer.git
cd pico-zero-ili9488-gps-spedometer

# Visual Studio Code megnyitása
code .
```

### 2. Firmware Fordítása és Feltöltése
1. Nyissa meg a projektet Visual Studio Code-ban
2. Telepítse a PlatformIO kiterjesztést
3. Csatlakoztassa a Pico-t USB-n keresztül BOOTSEL módban
4. Futtassa: `PlatformIO: Upload`

### 3. Trafipax Adatbázis Feltöltése
A készülék kezdeti használatához fel kell tölteni a trafipax adatbázist:

```bash
# Adatbázis feltöltése LittleFS-be
pio run --target uploadfs
```

## Trafipax Adatbázis Kezelése

### Adatbázis Formátum
A `data/trafipaxes.csv` fájl formátuma:
```csv
Vármegye,Település neve,Útszám,Kilométer-szelvény/utca,GPS koordináta szélesség,GPS koordináta hosszúság
Pest,Alsónémedi,,Haraszti út 6.,47.315308,19.163705
Pest,Biatorbágy,,M1 17+000,47.468751,18.864466
Pest,Budapest,,M0,47.589436,19.142904
Pest,Budapest,,M0 14+450,47.400208,19.011840
...
```

### Adatbázis Frissítése

#### Módszer 1: Teljes Újrafeltöltés (Ajánlott)
```bash
# 1. Új trafipaxes.csv elhelyezése a data/ mappában
# 2. Fájlrendszer feltöltése
pio run --target uploadfs

# 3. Készülék újraindítása (automatikus)
```

#### Módszer 2: Fejlesztői Módszer
```ini
# platformio.ini fájlban:
[env:pico]
extra_scripts = upload_fs.py  # Uncomment this line

# Ezután minden "pio run --target upload" automatikusan feltölti a fájlrendszert is
```

### Adatforrások
- **Magyarország:** [AutópályaMatrica.hu](https://www.autopalyamatrica.hu/fix-traffipax-lista-veda-terkep) adatai

**⚠️ Jogi figyelmeztetés:** A trafipax adatok tájékoztató jellegűek. A pontos és aktuális információkért mindig a hivatalos forrásokat használja!

## Konfiguráció és Testreszabás

### GPS Beállítások
```cpp
// include/pins.h fájlban
#define GPS_RX_PIN 0
#define GPS_TX_PIN 1
#define GPS_BAUD_RATE 9600
```

### Kijelző Beállítások  
```cpp
// Docs/TFT_eSPI/User_Setup.h fájlban
#define ILI9488_DRIVER
#define TFT_WIDTH  480
#define TFT_HEIGHT 320
```

### Trafipax Figyelmeztetési Távolságok
```cpp
// src/main.cpp fájlban
static constexpr double CRITICAL_DISTANCE = 800.0;  // 800m figyelmeztető távolság
static constexpr unsigned long SIREN_INTERVAL = 10000;  // 10 sec szirénázás
```

## Fejlesztői Információk

### Projekt Struktúra
```
├── src/                    # Fő forráskód
│   ├── main.cpp           # Főprogram
│   ├── TafipaxList.cpp    # Trafipax adatbázis kezelő
│   └── TafipaxList.h
├── include/               # Header fájlok
│   ├── pins.h            # Pin definíciók
│   ├── commons.h         # Közös definíciók
│   └── *.h
├── data/                  # LittleFS fájlok
│   └── trafipaxes.csv    # Trafipax adatbázis
├── kicad/                 # Hardver tervek
│   ├── *.kicad_sch       # Kapcsolási rajz
│   └── *.kicad_pcb       # Nyomtatott áramkör
├── Docs/                  # Dokumentáció
│   ├── pictures/         # Képek
│   └── connections/      # Kábelezési útmutatók
└── platformio.ini        # PlatformIO konfiguráció
```
## Licenc

Ez a projekt MIT licenc alatt áll - lásd a [LICENSE](LICENSE) fájlt a részletekért.

## Kapcsolat

**Fejlesztő:** BT-Soft  
**GitHub:** [https://github.com/bt-soft](https://github.com/bt-soft)  
**Email:** [email cím]

---

**⚠️ Biztonsági figyelmeztetés:** Ez az eszköz csak tájékoztató célokat szolgál. A közlekedési szabályok betartása a vezető felelőssége. A készülék használata nem mentesít a figyelmes és szabályos vezetés kötelezettsége alól.


## Képek 

<img src="Docs/pictures/20250910_185151.jpg" width="50%">
<img src="Docs/pictures/20250910_185202.jpg" width="50%">
<img src="Docs/pictures/20250910_185213.jpg" width="50%">
<img src="Docs/pictures/20250910_185229.jpg" width="50%">
<img src="Docs/pictures/20250910_185239.jpg" width="50%">
<img src="Docs/pictures/20250910_185308.jpg" width="50%">
<img src="Docs/pictures/20250910_185341.jpg" width="30%">
<img src="Docs/pictures/20250910_185345.jpg" width="30%">
<img src="Docs/pictures/20250910_185408.jpg" width="30%">