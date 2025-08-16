# Pico GPS Sebességmérő Motorokhoz

Ez a projekt egy Raspberry Pi Pico alapú GPS sebességmérő, amely egy ILI9488 TFT kijelzőt használ a sebesség, idő, magasság és a közeli sebességmérő kamerák (trafipaxok) megjelenítésére.

![3D render a készülékről](Docs/pictures/pico-gps-spedometer-3d.png)
![3D render a készülékről 2](Docs/pictures/pico-gps-spedometer-3d-2.png)

## Főbb Jellemzők

- **Valós idejű sebességmérés** GPS adatok alapján.
- **Pontos idő** megjelenítése (GPS szinkronizációval).
- Tengerszint feletti **magasság** kijelzése.
- **Sebességmérő kamera (trafipax) adatbázis** kezelése és figyelmeztetés a közeli kamerákra.
- **Lineáris és kör alakú sebességmérő** vizuális megjelenítés.
- **Automatikusan állítható háttérvilágítás** a környezeti fényviszonyokhoz igazodva (tervezett funkció).
- A hardver tervei **KiCad** szoftverrel készültek.

## Hardver

- **Mikrokontroller:** Raspberry Pi Pico vagy Pico Zero
- **Kijelző:** ILI9488 3.5" 480x320 SPI TFT kijelző
- **GPS Modul:** Bármilyen UART-alapú GPS modul (pl. NEO-6M)
- **Egyéb:** A projekt KiCad tervei a `kicad/` mappában találhatóak.

## Szoftver

- **Fejlesztői környezet:** [PlatformIO](https://platformio.org/) a [Visual Studio Code](https://code.visualstudio.com/)-ban.
- **Framework:** Arduino
- **Főbb könyvtárak:**
  - `bodmer/TFT_eSPI`
  - `mikalhart/TinyGPSPlus`
  - `LittleFS`

## Telepítés

1.  Klónozza a repository-t.
2.  Nyissa meg a projektet a Visual Studio Code-ban (telepített PlatformIO kiterjesztéssel).
3.  Fordítsa le és töltse fel a kódot a `PlatformIO: Upload` paranccsal.

## Trafipax Adatbázis Frissítése

A készülék a `data` mappában található `trafipaxes.csv` fájlból olvassa be a sebességmérő kamerák pozícióit. A frissítés menete a következő:

1.  **Szerezze be a frissített `trafipaxes.csv` fájlt.**
    - A fájlnak a következő formátumúnak kell lennie: `szélesség,hosszúság,típus,sebesség` (pl. `47.123456,19.123456,fix,50`).
2.  **Cserélje le a fájlt.**
    - Másolja az új `trafipaxes.csv` fájlt a projekt `data/` mappájába, felülírva a régit.
3.  **Töltse fel a fájlrendszert.**
    - Nyissa meg a `platformio.ini` fájlt.
    - Keresse meg az `extra_scripts` sort, és távolítsa el a megjegyzésjelet (`#`) előle, hogy így nézzen ki:
      ```ini
      extra_scripts = upload_fs.py
      ```
    - Mentse el a `platformio.ini` fájlt.
    - Futtassa a **`PlatformIO: Upload`** parancsot. A beállításnak köszönhetően a firmware feltöltése *előtt* a PlatformIO automatikusan feltölti a `data` mappa tartalmát (a LittleFS fájlrendszerbe).
4.  **(Opcionális) Állítsa vissza a konfigurációt.**
    - A feltöltés után ismét megjegyzésbe teheti az `extra_scripts` sort a `platformio.ini` fájlban, hogy a későbbi firmware feltöltések gyorsabbak legyenek (ne töltse fel minden alkalommal a fájlrendszert).

Ezzel a módszerrel az adatbázis frissül a készüléken.
