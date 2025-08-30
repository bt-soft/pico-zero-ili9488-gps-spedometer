#pragma once

#include <Arduino.h>

class DaylightSaving {

  private:
    /**
     *
     */
    static uint8_t dayOfWeek(uint8_t d, uint8_t m, uint8_t y) {

        static uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        y -= m < 3;
        // 0 Sunday, 1 Monday, etc...
        return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d + 6) % 7;
    }

    /**
     * Detects if we are in summer time
     */
    static boolean inSummerTime(uint8_t hours, uint8_t day, uint8_t month, uint8_t year) {

        if (day < 0 || month < 0 || year < 0)
            return false;

        if ((month >= 3) && (month <= 10)) {   // March to October inclusive
            if ((month > 3) && (month < 10)) { // Definitely
                return true;
            }
            if (month == 3) {    // Have we passed the last Sunday of March, 1am UT ?
                if (day >= 25) { // It's getting interesting
                    uint8_t dw = dayOfWeek(day, month, year);
                    // When is the next sunday ?
                    uint8_t dts = 6 - dw; // Number of days before the next sunday
                    if (dts == 0)
                        dts = 7;                  // We are really speaking of the next sunday, not the current one
                    if ((day + dts) > 31) {       // The next sunday is next month !
                        if (dw != 6 || hours > 0) // We finally check that we are not on the day of the change before the time of the change
                            return true;
                    }
                }
            }
            if (month == 10) {   // Have we passed the last Sunday of October 1am UT ?
                if (day >= 25) { // It's getting interesting
                    uint8_t dw = dayOfWeek(day, month, year);
                    // When is the next sunday ?
                    uint8_t dts = 6 - dw; // Number of days before the next sunday
                    if (dts == 0)
                        dts = 7;                  // We are really speaking of the next sunday, not the current one
                    if ((day + dts) > 31) {       // The next sunday is next month !
                        if (dw != 6 || hours > 0) // We finally check that we are not on the day of the change before the time of the change
                            return false;         // We have passed the change
                        else
                            return true;
                    } else
                        return true;
                } else
                    return true;
            }
        }
        return false;
    }

  public:
    /**
     * Korrigálja az időt a helyi időzóna és a nyári időszámítás figyelembevételével
     */
    static void correctTime(uint8_t &mins, uint8_t &hours, uint8_t &day, uint8_t &month, uint16_t &year) {
        int timeShift = 1; // Alapértelmezett CET (+1)
        if (inSummerTime(hours, day, month, year)) {
            timeShift += 1; // CEST (+2)
        }

        hours += timeShift;

        // Nap átlépés kezelése
        if (hours >= 24) {
            hours -= 24;
            day++;

            // Hónap végi átlépés kezelése (egyszerűsített)
            uint8_t daysInMonth = 31;
            if (month == 2) {
                // Szökőév ellenőrzése
                bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                daysInMonth = isLeap ? 29 : 28;
            } else if (month == 4 || month == 6 || month == 9 || month == 11) {
                daysInMonth = 30;
            }

            if (day > daysInMonth) {
                day = 1;
                month++;
                if (month > 12) {
                    month = 1;
                    year++;
                }
            }
        }
    }
};
