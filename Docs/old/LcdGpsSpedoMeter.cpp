#include <Arduino.h>
#include <TinyGPS++.h>
#include <NeoSWSerial.h>

#define SERIAL_RX_PIN 			8
#define SERIAL_TX_PIN 			9
NeoSWSerial gpsSerial(SERIAL_RX_PIN, SERIAL_TX_PIN);
TinyGPSPlus gps;

//---LCD Display ------------------------
#include <U8g2lib.h>
#define PIN_LCD_BACKLIGHT_LED 	11
#define PIN_LCD_SCLK 			6
#define PIN_LCD_DIN 			5
#define PIN_LCD_DC 				4
#define PIN_LCD_CS 				3
#define PIN_LCD_RST 			2
U8G2_PCD8544_84X48_1_4W_SW_SPI lcd(U8G2_R0, PIN_LCD_SCLK, PIN_LCD_DIN, PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
/**
 * Fontok nevei
 */
#define FONT_TINY 				u8g2_font_4x6_mr
#define FONT_SMALL 				u8g2_font_6x10_mr
#define FONT_BIG_NUM 			u8g2_font_ncenB24_tn

//--- LCD háttérvilágítás ---
#include "LcdBackLightAdjuster.h"
#define PIN_PHOTO_SENSOR		A0
LcdBackLightAdjuster lcdBackLightAdjuster(PIN_PHOTO_SENSOR, PIN_LCD_BACKLIGHT_LED);

//--- Nyomógomb kezelés  ---
#include <OneButton.h>
OneButton button = OneButton(A1, true, true);

/**
 * Aktuális működési mód
 */
typedef enum {
	STATE_NORMAL, STATE_INTERNAL, STATE_SATELITES
} State_t;
State_t currentState = STATE_NORMAL;

//Általános buffer a kiírogatáshoz
char buff[32];

//A  GPS mikor kezdett adni érvényes sebesség adatokat?
uint16_t gpsBootTime = 0; //indulás óta eltel másodpercek száma

//XBMP formátum!!
#include "logo.h"
extern const uint8_t LogoXBmp[];


#include "Utc2Cest.h";
Utc2Cet utc2Cet;

/**
 * Véda GPS koordináták
 */
//
#include "VedaGpsLocations.h"
#define VEDA_WARN_DISTANCE		1500
#define VEDA_ALARM_DISTANCE 	700
#define PIN_VEDA_ALARM_LED 		10
unsigned long vedaMinDistance = VEDA_WARN_DISTANCE; //legközelebbi VÉDA távolsága légvonalban
bool isVedaApproaching = false; //közelítünk hozzá?

/**
 * A legközelebbi véda kikeresése
 */
void findVeda() {

	//Ha eddig közelítettünk, és most épp állunk, akkor nem piszkáljuk a VEDA állapotokat, lehet hogy előtte meg kellett állnunk egy piros lámpánál
	if(isVedaApproaching && gps.speed.isValid() && gps.speed.age() < 3000 && gps.speed.kmph() <= 1 ) {
		return;
	}

	vedaMinDistance = VEDA_WARN_DISTANCE;
	static unsigned long prevMinDistance = VEDA_WARN_DISTANCE;
	isVedaApproaching = false;

	//Ha nincs vagy nem értelmes a GPS adat, akkor nem megyünk tovább
	if (!gps.location.isValid() || gps.location.age() > 3000) {
		return;
	}

	//Aktuális helyzet lekérése
	float gpsLat = gps.location.lat();
	float gpsLon = gps.location.lng();

	//Kikeressük a legközelebbi VEDA-t
	for (byte i = 0; i < (sizeof(VEDA_ARRAY) / sizeof(vedaPoint)); i++) {
		vedaPoint point;
		memcpy_P(&point, &VEDA_ARRAY[i], sizeof(vedaPoint));

		unsigned long distance = (unsigned long) TinyGPSPlus::distanceBetween(gpsLat, gpsLon, point.lat, point.lon);
		vedaMinDistance = min(distance, vedaMinDistance);
	}

	//Közelítünk?
	isVedaApproaching = prevMinDistance > vedaMinDistance;

	//Megjegyezzük a jelenlegi távolságot a távolságot
	prevMinDistance = vedaMinDistance;
}

/**
 * Feszmérés
 */
float readVccInVolts() {

	// Read 1.1V reference against AVcc
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(2);             // Wait for Vref to settle
	ADCSRA |= _BV(ADSC);  // Convert
	while (bit_is_set(ADCSRA, ADSC))
		;
	long result = ADCL;
	result |= ADCH << 8;
	result = 1126400L / result;  // Back-calculate AVcc in mV

	return ((float) result) / 1000.0;
}

/**
 *
 */
void displayHeader() {

	//Ha a VÉDA közelében vagyunk és közeledünk hozzá
	if (isVedaApproaching && vedaMinDistance < VEDA_WARN_DISTANCE) {

		//VÉDA közeleg!!
		lcd.setColorIndex(0);

		lcd.setFont(FONT_SMALL);
		sprintf(buff, "  VEDA: %dm  ", vedaMinDistance);
		lcd.setCursor(0, 7);
		lcd.print(buff);

		//visszaállítjuk az állapotokat
		lcd.setColorIndex(1);
		lcd.setFont(FONT_TINY);

		return;
	}

	//Sat
	lcd.drawStr(0, 5, "Sat: ");
	lcd.setCursor(18, 5);
	lcd.print(gps.satellites.isValid() && gps.satellites.age() < 3000 ? gps.satellites.value() : 0);

	//Hdop
	if (gps.satellites.isValid() && gps.hdop.age() < 3000) {
		lcd.setCursor(35, 5);
		lcd.print(gps.hdop.hdop(), 2);
	} else {
		lcd.drawStr(35, 5, "-");
	}

	//Feszkó
	lcd.setCursor(64, 5);
	lcd.print(readVccInVolts(), 2);
	lcd.drawStr(81, 5, "V");

}

/**
 *
 */
void normalDisplay() {

	lcd.setFont(FONT_TINY);

	//Header
	displayHeader();

	//Speed
	lcd.setFont(FONT_BIG_NUM);
	lcd.setCursor(0, 33);
	lcd.print(gps.speed.isValid() && gps.speed.age() < 3000 && gps.speed.kmph() > 1 ? gps.speed.kmph() : 0, 0);

	lcd.setFont(FONT_SMALL);
	lcd.drawStr(61, 17, "km/h");

	//Alt
	if (gps.altitude.isValid() && gps.altitude.age() < 3000) {
		sprintf(buff, "%4dm", gps.altitude.value() / 100);
	} else {
		sprintf(buff, "-m");
	}
	lcd.drawStr(55, 30, buff);

	//Date
	lcd.setFont(FONT_TINY);
	if (gps.date.isValid() && gps.date.age() < 3000) {
		sprintf(buff, "%04d.%02d.%02d", gps.date.year(), gps.date.month(), gps.date.day());
		lcd.drawStr(44, 39, buff);
	} else {
		lcd.drawStr(44, 39, "----.--.--");
	}

	//Time
	lcd.setFont(FONT_SMALL);
	if (gps.time.isValid() && gps.time.age() < 3000) {

		//CEST kiszámítása
		//uint8_t cestTimeOttset = utc2Cet.getOffsetHour(gps.date, gps.time);
		uint8_t cestTimeOttset = 2;

		sprintf(buff, "%02d:%02d:%02d", gps.time.hour() + cestTimeOttset, gps.time.minute(), gps.time.second());
		lcd.drawStr(0, 47, buff);

		lcd.setFont(FONT_TINY);
		lcd.drawStr(50, 47, "CEST");
	} else {
		lcd.drawStr(0, 47, "--:--:--");
	}

}

/**
 *
 */
void displayStatus() {
	lcd.setFont(FONT_SMALL);
	lcd.setColorIndex(0);
	lcd.drawStr(0, 8, "Internal state");
	lcd.setColorIndex(1);

	lcd.setFont(FONT_TINY);

#define STATUS_BEGIN_ROW 18
	byte rowNum = STATUS_BEGIN_ROW;

	lcd.drawStr(0, rowNum, "Sat: ");
	lcd.setCursor(20, rowNum);
	lcd.print(gps.satellites.value());

	rowNum += 6;
	lcd.drawStr(0, rowNum, "Lat: ");
	lcd.setCursor(20, rowNum);
	lcd.print(gps.location.lat(), 4);

	rowNum += 6;
	lcd.drawStr(0, rowNum, "Lon: ");
	lcd.setCursor(20, rowNum);
	lcd.print(gps.location.lng(), 4);

	rowNum += 6;
	lcd.drawStr(0, rowNum, "Alt: ");
	lcd.setCursor(20, rowNum);
	lcd.print(gps.altitude.meters(), 2);

	rowNum += 6;
	lcd.drawStr(0, rowNum, "Date: ");
	sprintf(buff, "%04d.%02d.%02d", gps.date.year(), gps.date.month(), gps.date.day());
	lcd.drawStr(20, rowNum, buff);

	rowNum += 6;
	lcd.drawStr(0, rowNum, "Time: ");
	sprintf(buff, "%02d:%02d:%02d UTC", gps.time.hour(), gps.time.minute(), gps.time.second());
	lcd.drawStr(20, rowNum, buff);

	//------- 2. oszlop

	rowNum = STATUS_BEGIN_ROW;
	lcd.drawStr(43, rowNum, "hDop: ");
	lcd.setCursor(65, rowNum);
	lcd.print(gps.hdop.hdop(), 2);

	rowNum += 6 * 2;
	lcd.drawStr(52, rowNum, "GBT: ");
	lcd.setCursor(69, rowNum);
	lcd.print(gpsBootTime);

	rowNum += 6;
	lcd.drawStr(52, rowNum, "Vcc: ");
	lcd.setCursor(69, rowNum);
	lcd.print(readVccInVolts(), 2);
}

#define MAX_DISP_SATELLITES 3*6 /* Max ennyit jelenítünk meg */
#define SAT_DATA_FRESH_MSEC (3 * 1000) /* ennyi ideig még friss a sat adata */

#include "Satellites.h"
Satellites satellites;

TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1);  // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);       // $GPGSV sentence, second element
TinyGPSCustom satsInView(gps, "GPGSV", 3);          // $GPGSV sentence, third element
TinyGPSCustom satNumber[4];                         // to be initialized later
TinyGPSCustom snr[4];

/**
 *
 */
bool processGPGSVMessages() {

	lcd.setFont(FONT_SMALL);

	if (!totalGPGSVMessages.isValid()) {
		lcd.drawStr(0, 10, "Not all $GPGSV messages processed yet");
		return false;
	}

	for (byte i = 0; i < 4; ++i) {

		//sat PRN number and SNR value
		uint8_t prnNo = atoi(satNumber[i].value());
		uint8_t snrVal = atoi(snr[i].value());

		//ez valami szemét adat?
		if (prnNo == 0) {
			continue;
		}

		//Fizikai limit
		if (satellites.countSats() >= MAX_DISP_SATELLITES) {
			break;
		}

		satellites.insertSatellite(prnNo, snrVal);
	}

	//totalMessages ==  currentMessage ?
	if (atoi(totalGPGSVMessages.value()) != atoi(messageNumber.value())) {
		//lcd.drawStr(0, 10, "Not complete");
		return false;
	}

	return true;
}

/**
 *
 */
void displaySatellites() {

	//karbantartjuk a listát
	satellites.freeSatellites();

	//rendezzük a listát
	satellites.sortSatellites();

	lcd.setFont(FONT_TINY);
	lcd.drawStr(0, 5, satellites.getSortType() == BY_PRN ? "PRN" : "SNR");

	//Nums
#define SAT_BEGIN_ROW 13
	uint8_t col = 0;
	uint8_t row = SAT_BEGIN_ROW;
	uint8_t displayedCnt = 0;
	uint8_t changedCnt = 0;
	struct sat *p = satellites.getStatsHead();
	while (p != NULL) {

		//változott az SNR értéke az elmúlt SAT_DATA_FRESH_MSEC másodpercben?
		if ((millis() - p->tStamp) < SAT_DATA_FRESH_MSEC) {
			lcd.setColorIndex(0);
			changedCnt++;
		}
		sprintf(buff, "%2d:%2d", p->prn, p->snr);
		lcd.drawStr(col, row, buff);
		lcd.setColorIndex(1);

		displayedCnt++;

		row += 7;

		if (displayedCnt == 6 || displayedCnt == 12) {
			col += 30;
			row = SAT_BEGIN_ROW;
		}

		p = p->next;
	}

	sprintf(buff, "Sat:%-2d/%-2d/%-2d", (uint8_t)gps.satellites.value(), displayedCnt, changedCnt);
	lcd.drawStr(30, 5, buff);
}

/**
 *
 */
void display() {

	switch (currentState) {
	case STATE_NORMAL:
	default:
		normalDisplay();
		break;

	case STATE_INTERNAL:
		displayStatus();
		break;

	case STATE_SATELITES:
		processGPGSVMessages();
		displaySatellites();
		break;
	}
}

/**
 *
 */
void handleButtonClick() {
	switch (currentState) {

	case STATE_NORMAL:
		currentState = STATE_INTERNAL;
		break;

	case STATE_INTERNAL:
		currentState = STATE_SATELITES;
		break;

	case STATE_SATELITES:
	default:
		currentState = STATE_NORMAL;
		break;
	}
}

/**
 *
 */
void handleButtonLongClick() {
	if (lcdBackLightAdjuster.blState) {
		lcdBackLightAdjuster.off();
	} else {
		lcdBackLightAdjuster.on();
	}
}

/**
 *
 */
void handleButtonDblClick() {

	switch (currentState) {

	case STATE_NORMAL:
	case STATE_INTERNAL:
	default:
		break;

	case STATE_SATELITES:
		if (satellites.getSortType() == BY_PRN) {
			satellites.setSortType(BY_SNR);
		} else {
			satellites.setSortType(BY_PRN);
		}
		break;
	}

}

/**
 *
 */

void setup() {

	//GPS start
	gpsSerial.begin(9600);

	//--- LCD
	lcd.begin();
	lcd.setContrast(130);
	lcd.setDrawColor(1);
	lcd.setFontMode(0);

	//--- LCD háttérvilágítás
	lcdBackLightAdjuster.init();
	lcdBackLightAdjuster.off(); //kikapcsoljuk a háttérvilágítást

	//--- Alarm LED
	pinMode(PIN_VEDA_ALARM_LED, OUTPUT);
	digitalWrite(PIN_VEDA_ALARM_LED, HIGH); //Veda Alarm LED on

	button.attachClick(handleButtonClick);
	button.attachLongPressStart(handleButtonLongClick);
	button.attachDoubleClick(handleButtonDblClick);

	//TinyGPSCustom objektumok felhúzása
	for (byte i = 0; i < 4; ++i) {
		satNumber[i].begin(gps, "GPGSV", 4 + 4 * i);  // offsets 4, 8, 12, 16
		snr[i].begin(gps, "GPGSV", 7 + 4 * i);        // offsets 7, 11, 15, 19
	}

	//Splash képernyő
	lcd.clear();
	lcd.firstPage();
	do {
		lcd.drawXBMP(8, 0, 71, 13, LogoXBmp);
		lcd.setFont(FONT_SMALL);
		lcd.drawStr(23, 25, "V0.0.5");
		lcd.setFont(FONT_TINY);
		lcd.drawStr(0, 45, __DATE__);
		lcd.drawStr(50, 45, __TIME__);
	} while (lcd.nextPage());
	delay(3000);

	//Veda Alarm LED off
	digitalWrite(PIN_VEDA_ALARM_LED, LOW);
}

/**
 *
 */
void vedaAlarm() {
	static bool vedaAlarmLedState = false;
	static long lastTime = millis();

	//Ha nem közeledünk vagy épp nincs VEDA a közelben ÉS épp aktív a LED
	if (!isVedaApproaching || vedaMinDistance > VEDA_WARN_DISTANCE) {
		if (vedaAlarmLedState) {
			digitalWrite(PIN_VEDA_ALARM_LED, LOW);
			vedaAlarmLedState = false;
		}
		return;
	}

	//Warning/Alarm frekvencia beállítása
	if (millis() - lastTime < (vedaMinDistance > VEDA_ALARM_DISTANCE ? 500 : 100)) {
		return;
	}

	//LED villogtatása
	digitalWrite(PIN_VEDA_ALARM_LED, (vedaAlarmLedState = !vedaAlarmLedState));
	lastTime = millis();

}

/**
 *
 */
void tick() {
	lcdBackLightAdjuster.adjust();
	button.tick();
	vedaAlarm();
}

/**
 *
 */
void loop() {

	static long startTime = millis();

	bool newData = false;
	for (unsigned long start = millis(); millis() - start < 1000;) {

		while (gpsSerial.available()) {
			if (gps.encode(gpsSerial.read())) {
				newData = true;

				//Mikor bootolt be a GPS?
				if(gpsBootTime == 0 && gps.speed.isValid()){
					gpsBootTime = (millis() - startTime) / 1000;
				}

			}
			tick();
		}

		tick();
	}

	if (newData) {
		lcd.setColorIndex(1);
		lcd.clearBuffer();
		lcd.firstPage();
		do {
			display();
		} while (lcd.nextPage());

		//VÉDA keresése
		findVeda();

		return;
	}

	lcd.clear();
	lcd.firstPage();
	lcd.setColorIndex(lcd.getColorIndex() == 0 ? 1 : 0);
	lcd.setFont(FONT_SMALL);
	do {
		lcd.drawStr(5, 25, "No GPS Data!");
	} while (lcd.nextPage());
	delay(1000);
}
