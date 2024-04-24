/****************************************************************************
	menu.h Interface for the menu class
	Author Andreas Dorrer
	Last Change: 08.11.2018
*****************************************************************************/
#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

// Enums
typedef enum MENUTYPES {
	NAME = 1,
	RCOUNT = 2,
	R = 3,
	K = 4,
	HCOUNT = 5,
	H = 6,
	END = 7,
	PROG = 8,

	// SETTINGSMENU
	DISPLAYTIMEOUT = 9,
	BUZZERONOFF = 10,
	BRIGHTNESS = 11,
	LONGPRESS = 12

}MENUTYPES;

typedef struct MENUTYPE {
	MENUTYPES Type;
	uint8_t uiId;
}MENUTYPE;

// Global Variables
void (*RodaryTick)(uint8_t);
void (*RodaryPush)();
void (*RodaryLongPush)();
void (*timerHalf)(uint8_t);

uint8_t uiIsBrewMonitor, uiBreMonitorActice, uiRCurrentCount, uiKCurrentTime, uiCurrentR, uiHCurrentCount, uiCurrentProgNr;
uint8_t rTemps[7], rTimes[7];
uint8_t hTimes[5];
MENUTYPE scrollList[17];

// functions
void setSelectionPointer(uint8_t iXPos);
void initMenuChange();
void updateEditValue(uint8_t iDelta);
void clearMeanuSection();
void blinkEditValue(uint8_t uiBlink);

// Main Menu
void drawMainMenu();

// Brew Menu
void drawBrewMenu();

// Programs Menu
void drawProgrammsMenu();
void scrollProgrammsMenu(uint8_t uiPage);

// Programs Edit Menu
void drawProgrammsEditMenu();
void scrollProgrammsEditMenu(uint8_t uiPage);
void drawProgrammsEditLine(uint8_t uiIndex, uint8_t uiMenuLine);
void editPName();
void editPNameNextChar();
void editPNameFinished();
void editRCount();
void editRCountFinished();
void editRTemp();
void editRHour();
void editHour();
void editMinute();
void editTimeFinished();
void editHCount();
void editHCountFinished();

// Settings Menu
void drawSettingsMenu();
void scrollSettingsMenu(uint8_t uiPage);
void editBrightness();
void editBrightnessFinished();
void editBuzzer();
void editBuzzerFinished();
void editSecondValue();
void editSecondValueFinished();

// Brew Monitor
void loadProg();
void continueBrewMonitor();
void drawBrewMonitor(int8_t iProgNr);
void updateBrewMonitor(uint8_t uiStep);
void startCurrentTimer();
void stopCurrentTimer();
void blinkIndicator(uint8_t uiBlink);
void drawBrewMonitorMenu();
void skipCurrentStep();

#endif
