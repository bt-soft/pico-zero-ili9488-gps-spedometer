#ifndef __SATELLITES_DB_H__
#define __SATELLITES_DB_H__

#include <TinyGPS++.h>
#include <algorithm>
#include <list>

class SatelliteDb {
  public:
    struct SatelliteData {
        uint8_t prn;
        int16_t elevation;
        int16_t azimuth;
        int16_t snr;
        unsigned long timeStamp;

        // Konstruktor
        SatelliteData(uint8_t p, int16_t e, int16_t a, int16_t s, unsigned long t = 0) : prn(p), elevation(e), azimuth(a), snr(s), timeStamp(t) {}
    };

    typedef enum { BY_PRN, BY_SNR } SortType_t;

    SatelliteDb();
    ~SatelliteDb() = default; // std::list automatikusan felszabadítja a memóriát

    uint8_t countSats() const;
    SatelliteData *searchSat(uint8_t prnNo);
    void deleteSat(uint8_t prn);
    void deleteUntrackedSatellites();
    void insertSatellite(uint8_t prn, int16_t elevation, int16_t azimuth, int16_t snr);
    void sortSatellites();
    void clear();

    SortType_t getSortType() const { return sortType; }
    void setSortType(SortType_t type) { sortType = type; }

    // Iterator támogatás a külső kód számára
    auto begin() { return satellites.begin(); }
    auto end() { return satellites.end(); }
    auto begin() const { return satellites.begin(); }
    auto end() const { return satellites.end(); }

    void debugSatDb(uint8_t num_sats_in_view);

  private:
    std::list<SatelliteData> satellites;
    SortType_t sortType = BY_PRN;

    // Segédfüggvény a kereséshez
    auto findSatellite(uint8_t prn) {
        return std::find_if(satellites.begin(), satellites.end(), [prn](const SatelliteData &sat) { return sat.prn == prn; });
    }
};

#endif // __SATELLITES_DB_H__