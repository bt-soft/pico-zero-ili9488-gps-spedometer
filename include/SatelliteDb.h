#ifndef __SATELLITES_DB_H__
#define __SATELLITES_DB_H__

#include <TinyGPS++.h>

class SatelliteDb {
  public:
    struct SatelliteData {
        uint8_t prn;
        int16_t elevation;
        int16_t azimuth;
        int16_t snr;
        long timeStamp;

        struct SatelliteData *next;
    };
    typedef enum { BY_PRN, BY_SNR } SortType_t;

    SatelliteDb();
    ~SatelliteDb();
    uint8_t countSats();
    SatelliteData *searchSat(uint8_t prnNo);
    void deleteSat(SatelliteData *p);
    void deleteUntrackedSatellites();
    void insertSatellite(uint8_t prn, int16_t elevation, int16_t azimuth, int16_t snr);
    void sortSatellites();

    SortType_t getSortType() { return sortType; }
    void setSortType(SortType_t type) { sortType = type; }

    struct SatelliteData *getStatsHead() { return pSatsHead; }

  private:
    SatelliteData *pSatsHead = nullptr;
    SortType_t sortType = BY_PRN;
};

#endif // __SATELLITES_DB_H__