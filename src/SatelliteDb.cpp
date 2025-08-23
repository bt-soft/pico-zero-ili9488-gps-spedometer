#include "SatelliteDb.h"

#define SAT_MAX_AGE_MSEC (20 * 1000) /* Xmp után töröljük a sat-ot ha nem frissült */

/**
 * Konstruktor
 */
SatelliteDb::SatelliteDb() {
    pSatsHead = nullptr;
    sortType = BY_PRN;
}

/**
 * Destruktor
 */
SatelliteDb::~SatelliteDb() {
    // Töröljük az összes node-ot
    while (pSatsHead != nullptr) {
        SatelliteData *temp = pSatsHead;
        pSatsHead = pSatsHead->next;
        delete temp;
    }
}

/**
 *
 */
uint8_t SatelliteDb::countSats() {
    SatelliteData *p = pSatsHead;
    uint8_t cnt = 0;

    while (p != NULL) {
        cnt++;
        p = p->next;
    }

    return cnt;
}

/**
 * Sat keresése
 */
SatelliteDb::SatelliteData *SatelliteDb::searchSat(uint8_t prnNo) {
    SatelliteData *p = pSatsHead;
    while (p != NULL) {
        if (p->prn == prnNo) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/**
 *
 */
void SatelliteDb::deleteSat(SatelliteData *p) {
    if (pSatsHead == NULL) {
        return;
    }

    if (p == pSatsHead) {
        pSatsHead = p->next;
        delete p;
        return;
    }
    SatelliteData *curr = pSatsHead;
    SatelliteData *prev = pSatsHead;
    while (curr != NULL) {
        if (curr == p) {
            prev->next = curr->next;
            delete p;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/**
 *
 */
void SatelliteDb::deleteUntrackedSatellites() {

    SatelliteData *p = pSatsHead;
    while (p != NULL) {
        SatelliteData *next = p->next;
        if ((millis() - p->timeStamp) > SAT_MAX_AGE_MSEC) {
            deleteSat(p);
        }
        p = next;
    }
}

/**
 *
 */
void SatelliteDb::insertSatellite(uint8_t prn, int16_t elevation, int16_t azimuth, int16_t snr) {

    // Frissítés lesz csak?
    SatelliteData *p = searchSat(prn);
    if (p != NULL) {

        if (snr == 0) {
            deleteSat(p);
        } else {
            // Friss/új SNR adat
            p->timeStamp = millis();
            p->elevation = elevation;
            p->azimuth = azimuth;
            p->snr = snr;
        }

        return;
    }

    // Nincs még ilyen node  -> Új node lesz
    p = new SatelliteData;
    p->prn = prn;
    p->elevation = elevation;
    p->azimuth = azimuth;
    p->snr = snr;
    p->timeStamp = millis();
    p->next = NULL;

    if (pSatsHead == NULL) {
        pSatsHead = p;
        return;
    }
    SatelliteData *curr = pSatsHead;
    while (curr != NULL) {
        if (curr->next == NULL) {
            curr->next = p;
            break;
        }
        curr = curr->next;
    }
}

/**
 *
 */
void SatelliteDb::sortSatellites() {
    if (pSatsHead == NULL) {
        return;
    }

    SatelliteData *curr = pSatsHead;
    SatelliteData *index = NULL;
    while (curr != NULL) {
        index = curr->next;
        while (index != NULL) {

            bool doSwap = false;
            if (sortType == BY_PRN) {
                doSwap = curr->prn > index->prn;
            } else if (sortType == BY_SNR) {
                doSwap = curr->snr < index->snr; // SNR csökkenő sorrendben
            }

            if (doSwap) {
                uint8_t tmpPrn = curr->prn;
                int16_t tmpElevation = curr->elevation;
                int16_t tmpAzimuth = curr->azimuth;
                int16_t tmpSnr = curr->snr;
                long tmpTimeStamp = curr->timeStamp;

                curr->prn = index->prn;
                curr->elevation = index->elevation;
                curr->azimuth = index->azimuth;
                curr->snr = index->snr;
                curr->timeStamp = index->timeStamp;

                index->prn = tmpPrn;
                index->elevation = tmpElevation;
                index->azimuth = tmpAzimuth;
                index->snr = tmpSnr;
                index->timeStamp = tmpTimeStamp;
            }
            index = index->next;
        }
        curr = curr->next;
    }
}
