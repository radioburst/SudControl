/****************************************************************************
	menu.h Definition for the menu class
	Author Andreas Dorrer
	Last Change: 08.11.2018
*****************************************************************************/

#include "menu.h"
#include "main.h"
#include "lcd-routines.h"

#define SELECTIONSTARTX 0
#define MENULINE1 2
#define MENULINE2 3
#define MENULINE3 4
#define MAXMENULINES 3
#define MENUSTARTX 2
#define HOMEPOSSELECTION 2
#define HEADERPOSX 0
#define HEADERPOSY 1

#define UE "\365" // ü
#define AE "\341" // ä
#define OE "\357" // ö
#define SZ "\342" //ß

// ext. Global Function pointers
extern void (*RodaryTick)(uint8_t);
extern void (*RodaryPush)();
extern void (*RodaryLongPush)();
extern void (*timerHalf)(uint8_t);
uint8_t uiIsBrewMonitor = 0, uiBreMonitorActice = 0, uiRCurrentCount = 0, uiKCurrentTime = 0, uiCurrentR = 0, uiHCurrentCount = 0;
uint8_t rTemps[7], rTimes[7], hTimes[5];
void (*ScrollToNextPage)(uint8_t);
void (*MenuSelections[MAXMENULINES])();

int8_t iCurrentSelectionPos = HOMEPOSSELECTION, iSavedSelectionPos = HOMEPOSSELECTION;
uint8_t uiCurrentMax = 1, uiCurrentPage = 0, uiSavedPage = 0, uiProgNr = 0, iEditValue = 0, uiCurrentProgNr = 0;
uint8_t uiEditValueX = 0, uiEditValueY = 0, uiEditValueMax = 0, uiEditValueMin = 0, uiEditValueIsNumeric = 0;
uint8_t uiEditHour = 0, uiEditMinute = 0;
uint8_t uiProgramsLastPage = 0, uiProgramsLastSelection = 0, uiProgramsEditLastPage = 0, uiProgramsEditLastSelection = 0;
char cPEditName[12] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
MENUTYPE currentMenuItem;

void setSelectionPointer(uint8_t iXPos)
{
	iCurrentSelectionPos -= iXPos;

	if(iCurrentSelectionPos > uiCurrentMax - (uiCurrentPage * MAXMENULINES) || iCurrentSelectionPos > MAXMENULINES + 1)
	{
		if(ScrollToNextPage && ((uiCurrentPage + 1) * MAXMENULINES) < uiCurrentMax - 1)
		{
			iCurrentSelectionPos = HOMEPOSSELECTION;
			uiCurrentPage++;
			ScrollToNextPage(uiCurrentPage);
		}
		else
			iCurrentSelectionPos = uiCurrentMax - (uiCurrentPage + 1) * MAXMENULINES + MAXMENULINES;
	}
	else if(iCurrentSelectionPos < MENULINE1)
	{
		if(ScrollToNextPage && uiCurrentPage > 0)
		{
			iCurrentSelectionPos = MAXMENULINES + 1;
			uiCurrentPage--;
			ScrollToNextPage(uiCurrentPage);
		}
		else
			iCurrentSelectionPos = HOMEPOSSELECTION;
	}

	currentMenuItem = scrollList[uiCurrentPage * MAXMENULINES + iCurrentSelectionPos - 2];
	RodaryPush = MenuSelections[iCurrentSelectionPos - 2];

	lcd_setcursor(SELECTIONSTARTX, MENULINE1);
	lcd_string(" ");
	lcd_setcursor(SELECTIONSTARTX, MENULINE2);
	lcd_string(" ");
	lcd_setcursor(SELECTIONSTARTX, MENULINE3);
	lcd_string(" ");

	lcd_setcursor(SELECTIONSTARTX, iCurrentSelectionPos);
	lcd_string(">");
}

void initMenuChange()
{
	lcd_clear();
	drawBatValue();
	uiCurrentPage = 0;
	iCurrentSelectionPos = HOMEPOSSELECTION;
	setSelectionPointer(0);
	RodaryLongPush = NULL;
}

void updateEditValue(uint8_t iDelta)
{
	if(currentMenuItem.Type == BRIGHTNESS || currentMenuItem.Type == DISPLAYTIMEOUT)
		iEditValue -= (int8_t)(iDelta * 10);
	else
		iEditValue -= (int8_t)iDelta;

	if(iEditValue > uiEditValueMax)
		iEditValue = uiEditValueMin;
	else if(iEditValue < uiEditValueMin)
		iEditValue = uiEditValueMax;

	lcd_setcursor(uiEditValueX, uiEditValueY);
	if(uiEditValueIsNumeric == 1)
	{
		char c[3];
		lcd_string(itoa(iEditValue, c, 10));
	}
	else if(uiEditValueIsNumeric == 2)
	{
		char c[6];
		uint8_t uiEditValue = iEditValue;
		sprintf(c, "%s", ftoa(c, (float)uiEditValue / 2, 1));
		lcd_string(c);
	}
	else if(uiEditValueIsNumeric == 3)
	{
		char c[5];
		if(currentMenuItem.Type == BRIGHTNESS && iEditValue < 99)
			sprintf(c, " %02d", iEditValue);
		else if(currentMenuItem.Type == DISPLAYTIMEOUT && iEditValue > 60)
			sprintf(c, "Ein");
		else if(currentMenuItem.Type == DISPLAYTIMEOUT)
			sprintf(c, "%02ds", iEditValue);
		else
			sprintf(c, "%02d", iEditValue);
		lcd_string(c);
	}
	else if(uiEditValueIsNumeric == 4)
	{
		if(iEditValue > 0)
			lcd_string("Ein");
		else
			lcd_string("Aus");
	}
	else
		lcd_data(iEditValue);

	if(currentMenuItem.Type == BRIGHTNESS)
		uiBrightness = iEditValue;
}

void clearMeanuSection()
{
	for(uint8_t i = 2; i < MENULINE3 +1; i++)
	{
		lcd_setcursor(2, i);
		lcd_string("                  ");
	}
}

void blinkEditValue(uint8_t uiBlink)
{
	if(uiBlink > 0)
	{
		lcd_setcursor(uiEditValueX, uiEditValueY);
		if(uiEditValueIsNumeric == 2)
			lcd_string("    ");
		else if(uiEditValueIsNumeric == 4 || currentMenuItem.Type == BRIGHTNESS || (currentMenuItem.Type == DISPLAYTIMEOUT && iEditValue > 60))
			lcd_string("   ");
		else if(uiEditValueIsNumeric == 3)
			lcd_string("  ");
		else
			lcd_string(" ");
	}
	else
		updateEditValue(0);
}

// Main Menu

void drawMainMenu()
{
	uiCurrentMax = MENULINE3;
	RodaryTick = &setSelectionPointer;
	timerHalf = NULL;
	blinkIndicator(0);
	ScrollToNextPage = NULL;
	uiProgramsEditLastPage = 0;
	uiProgramsEditLastSelection = 0;
	MenuSelections[0] = &drawBrewMenu;
	MenuSelections[1] = &drawProgrammsMenu;
	MenuSelections[2] = &drawSettingsMenu;
	initMenuChange();
	uiIsBrewMonitor = 0;
	uiBreMonitorActice = 0;

	uiProgramsLastPage = 0;
	uiProgramsLastSelection = 0;

	lcd_setcursor(MENUSTARTX, MENULINE1);
	lcd_string("Brauen");

	lcd_setcursor(MENUSTARTX, MENULINE2);
	lcd_string("Programme");

	lcd_setcursor(MENUSTARTX, MENULINE3);
	lcd_string("Einstellungen");
}

// Brew Menu

void drawBrewMenu()
{
	char cProg[21];
	uiCurrentMax = MENULINE3;
	RodaryTick = &setSelectionPointer;
	ScrollToNextPage = NULL;
	MenuSelections[0] = &continueBrewMonitor;
	MenuSelections[1] = &drawProgrammsMenu;
	MenuSelections[2] = &drawMainMenu;
	initMenuChange();
	uiIsBrewMonitor = 1;

	lcd_setcursor(HEADERPOSX, HEADERPOSY);
	lcd_string("Brauen");

	lcd_setcursor(MENUSTARTX, MENULINE1);
	sprintf(cProg, "Fortsetzen (P%02d)", uiCurrentProgNr);
	lcd_string(cProg);

	lcd_setcursor(MENUSTARTX, MENULINE2);
	lcd_string("Programm laden");

	lcd_setcursor(MENUSTARTX, MENULINE3);
	lcd_string("Zur\365ck..");
}

// Programms Menu

void drawProgrammsMenu()
{
	uiCurrentMax = 16;
	RodaryTick = &setSelectionPointer;
	ScrollToNextPage = &scrollProgrammsMenu;
	initMenuChange();
	uiProgramsEditLastPage = 0;
	uiProgramsEditLastSelection = 0;

	lcd_setcursor(HEADERPOSX, HEADERPOSY);
	if(uiIsBrewMonitor == 1)
		lcd_string("Programm laden");
	else
		lcd_string("Programme");

	MENUTYPE temp;
	for(uint8_t i = 0; i < PROG_COUNT; i++)
	{
		temp.Type = PROG;
		temp.uiId = i;
		scrollList[i] = temp;
	}
	temp.Type = END;
	temp.uiId = 0;
	scrollList[PROG_COUNT] = temp;

	iCurrentSelectionPos = uiProgramsLastSelection;
	uiCurrentPage = uiProgramsLastPage;
	scrollProgrammsMenu(uiProgramsLastPage);
	setSelectionPointer(0);
}

void scrollProgrammsMenu(uint8_t uiPage)
{
	char cProg[21], cProgName[12];
	clearMeanuSection();
	uiProgramsLastPage = uiPage;
	uint8_t uiMenuLine = MENULINE1;

	for(uint8_t uiIndex = uiPage * MAXMENULINES; uiIndex < uiPage * MAXMENULINES + MAXMENULINES; uiIndex++)
	{
		if(scrollList[uiIndex].Type == PROG)
		{
			eeprom_read_block((void*) &cProgName, (const void*)(PROG_OFFSET * scrollList[uiIndex].uiId) + PROG_ADDR_NAME, 11);
			cProgName[11] = '\0';

			if(uiIsBrewMonitor == 1)
				MenuSelections[uiMenuLine - 2] = &loadProg;
			else
				MenuSelections[uiMenuLine - 2] = &drawProgrammsEditMenu;

			sprintf(cProg, "P%02d (%s) ", scrollList[uiIndex].uiId + 1, cProgName);
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			lcd_string(cProg);
		}
		else if(scrollList[uiIndex].Type == END)
		{
			MenuSelections[uiMenuLine - 2] = &drawMainMenu;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			lcd_string("Zur\365ck..         ");
		}
		uiMenuLine++;
	}
}

// Programms Edit Menu

void drawProgrammsEditMenu()
{
	char cLcd[21];
	uiProgramsLastSelection = iSavedSelectionPos = iCurrentSelectionPos;
	uiSavedPage = uiCurrentPage;
	uiProgNr = (uiCurrentPage * MAXMENULINES + iCurrentSelectionPos - 1);
	initMenuChange();
	RodaryTick = &setSelectionPointer;
	ScrollToNextPage = scrollProgrammsEditMenu;

	uint8_t uiRCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_R_COUNT));
	uint8_t uiHCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_H_COUNT));
	uiCurrentMax = MENULINE3 + uiRCount + uiHCount + 2;

	sprintf(cLcd, "P%d", uiProgNr);
	lcd_setcursor(HEADERPOSX, HEADERPOSY);
	lcd_string(cLcd);

	uint8_t uiIndex = 0;
	MENUTYPE temp;

	temp.Type = NAME;
	temp.uiId = 0;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = RCOUNT;
	scrollList[uiIndex] = temp;
	uiIndex++;

	for(uint8_t i = 0; i < uiRCount; i++)
	{
		temp.Type = R;
		temp.uiId = i;
		scrollList[uiIndex] = temp;
		uiIndex++;
	}

	temp.Type = K;
	temp.uiId = 0;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = HCOUNT;
	scrollList[uiIndex] = temp;
	uiIndex++;

	for(uint8_t i = 0; i < uiHCount; i++)
	{
		temp.Type = H;
		temp.uiId = i;
		scrollList[uiIndex] = temp;
		uiIndex++;
	}

	temp.Type = END;
	temp.uiId = 0;
	scrollList[uiIndex] = temp;

	iCurrentSelectionPos = uiProgramsEditLastSelection;
	uiCurrentPage = uiProgramsEditLastPage;
	scrollProgrammsEditMenu(uiProgramsEditLastPage);
	setSelectionPointer(0);
}

void scrollProgrammsEditMenu(uint8_t uiPage)
{
	clearMeanuSection();
	uiProgramsEditLastPage = uiPage;
	uint8_t uiMenuLine = MENULINE1;

	for(uint8_t uiIndex = uiPage * MAXMENULINES; uiIndex < uiPage * MAXMENULINES + MAXMENULINES; uiIndex++)
	{
		drawProgrammsEditLine(uiIndex, uiMenuLine);
		uiMenuLine++;

		if(scrollList[uiIndex].Type == END)
			break;
	}

	for(; uiMenuLine < MAXMENULINES + 2; uiMenuLine++)
		lcd_clearLine(uiMenuLine);
}

void drawProgrammsEditLine(uint8_t uiIndex, uint8_t uiMenuLine)
{
	char cLcd[21], cAtof[5];
	uint8_t uiTime, uiTemp, uiRCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_R_COUNT));

	if(scrollList[uiIndex].Type == NAME)
	{
		char cProgName[12];
		eeprom_read_block((void*) &cProgName, (const void*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_NAME), 11);
		cProgName[11] = '\0';
		MenuSelections[uiMenuLine - 2] = &editPName;
		sprintf(cLcd, "Name: %s", cProgName);
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == RCOUNT)
	{
		MenuSelections[uiMenuLine - 2] = &editRCount;
		sprintf(cLcd, "Anz. R: %d         ", uiRCount);
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == R)
	{
		uiTemp = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * scrollList[uiIndex].uiId) + PROG_ADDR_R_TEMP));
		uiTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * scrollList[uiIndex].uiId) + PROG_ADDR_R_TIME));

		MenuSelections[uiMenuLine - 2] = &editRTemp;
		sprintf(cLcd, "R%d: %s%cC %02d:%02d", scrollList[uiIndex].uiId + 1, ftoa(cAtof, (float) uiTemp / 2, 1), (char) 223, uiTime / 60, uiTime % 60);
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == K)
	{
		uint8_t uiKTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_K_TIME));
		MenuSelections[uiMenuLine - 2] = &editHour;
		sprintf(cLcd, "K:         %02d:%02d", uiKTime / 60, uiKTime % 60);
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == HCOUNT)
	{
		uint8_t uiHCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_H_COUNT));
		MenuSelections[uiMenuLine - 2] = &editHCount;
		sprintf(cLcd, "Anz. H: %d         ", uiHCount);
		lcd_setcursor(MENUSTARTX, uiMenuLine );
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == H)
	{
		uiTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_H_OFFSET * scrollList[uiIndex].uiId) + PROG_ADDR_H_TIME));
		MenuSelections[uiMenuLine - 2] = &editHour;
		sprintf(cLcd, "H%d:        %02d:%02d", scrollList[uiIndex].uiId + 1, uiTime / 60, uiTime % 60);
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string(cLcd);
	}
	else if(scrollList[uiIndex].Type == END)
	{
		MenuSelections[uiMenuLine - 2] = &drawProgrammsMenu;
		lcd_setcursor(MENUSTARTX, uiMenuLine);
		lcd_string("Zur\365ck..");
	}
}

void editPName()
{
	uiEditValueIsNumeric = 0;
	uiEditValueMax = 'z';
	uiEditValueMin = ' ';
	uiEditValueX = 8;
	uiEditValueY = iCurrentSelectionPos;
	uiProgramsEditLastSelection = iCurrentSelectionPos;

	eeprom_read_block((void*) &cPEditName, (const void*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_NAME), 11);
	iEditValue = cPEditName[0];

	RodaryTick = &updateEditValue;
	RodaryPush = &editPNameNextChar;
	RodaryLongPush = &editPNameFinished;
	timerHalf = &blinkEditValue;
}

void editPNameNextChar()
{
	timerHalf = NULL;
	updateEditValue(0);
	cPEditName[uiEditValueX - 8] = iEditValue;
	uiEditValueX++;

	if(uiEditValueX > 18)
		editPNameFinished();

	iEditValue = cPEditName[uiEditValueX - 8];
	timerHalf = &blinkEditValue;
}

void editPNameFinished()
{
	timerHalf = NULL;

	for(uint8_t i = 0; i < 11; i++)
	{
		if(cPEditName[i] == ' ')
		{
			cPEditName[i] = '\0';
		    break;
		}
	}

	eeprom_write_block(cPEditName, (uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_NAME), 11);
	iCurrentSelectionPos = iSavedSelectionPos;
	uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawProgrammsEditMenu();
}

void editRCount()
{
	uiEditValueIsNumeric = 1;
	uiEditValueMax = PROG_R_COUNT;
	uiEditValueMin = 1;
	iEditValue = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_R_COUNT));
	uiEditValueX = 10;
	uiEditValueY = iCurrentSelectionPos;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editRCountFinished;
	timerHalf = &blinkEditValue;
}

void editRCountFinished()
{
	timerHalf = NULL;
	updateEditValue(0);
	eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_R_COUNT), iEditValue);
	iCurrentSelectionPos = iSavedSelectionPos;
	uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawProgrammsEditMenu();
}

void editRTemp()
{
	uiEditValueIsNumeric = 2;
	uiEditValueMax = 199;
	uiEditValueMin = 10;
	uiEditValueX = 6;
	uiEditValueY = iCurrentSelectionPos;
	uiProgramsEditLastSelection = iCurrentSelectionPos;

	iEditValue = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * currentMenuItem.uiId) + PROG_ADDR_R_TEMP));

	RodaryTick = &updateEditValue;
	RodaryPush = &editRHour;
	timerHalf = &blinkEditValue;
}

void editRHour()
{
	timerHalf = NULL;
	updateEditValue(0);
	eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * currentMenuItem.uiId) + PROG_ADDR_R_TEMP), iEditValue);

	editHour();
}

void editHour()
{
	uiEditValueIsNumeric = 3;
	uiEditValueMax = 4;
	uiEditValueMin = 0;
	uint8_t uiTime;

	if(currentMenuItem.Type == K)
		uiTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_K_TIME));
	else if(currentMenuItem.Type == H)
		uiTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_H_OFFSET * currentMenuItem.uiId) + PROG_ADDR_H_TIME));
	else if(currentMenuItem.Type == R)
		uiTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * currentMenuItem.uiId) + PROG_ADDR_R_TIME));
	else
		return;

	uiEditHour = uiTime / 60;
	uiEditMinute = uiTime % 60;

	iEditValue = uiEditHour;
	uiEditValueX = 13;
	uiEditValueY = iCurrentSelectionPos;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editMinute;
	timerHalf = &blinkEditValue;
}

void editMinute()
{
	timerHalf = NULL;
	updateEditValue(0);
	uiEditValueMax = 59;
	uiEditValueMin = 0;

	uiEditHour = iEditValue;
	iEditValue = uiEditMinute;
	uiEditValueX = 16;
	RodaryPush = &editTimeFinished;
	timerHalf = &blinkEditValue;
}

void editTimeFinished()
{
	timerHalf = NULL;
	updateEditValue(0);

	if(currentMenuItem.Type == K)
		eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_K_TIME), uiEditHour * 60 + iEditValue);
	else if(currentMenuItem.Type == H)
		eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_H_OFFSET * currentMenuItem.uiId) + PROG_ADDR_H_TIME), uiEditHour * 60 + iEditValue);
	else if(currentMenuItem.Type == R)
		eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + (PROG_R_OFFSET * currentMenuItem.uiId) + PROG_ADDR_R_TIME), uiEditHour * 60 + iEditValue);

	iCurrentSelectionPos = iSavedSelectionPos;
	uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawProgrammsEditMenu();
}

void editHCount()
{
	uiEditValueIsNumeric = 1;
	uiEditValueMax = PROG_H_COUNT;
	uiEditValueMin = 1;
	iEditValue = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_H_COUNT));
	uiEditValueX = 10;
	uiEditValueY = iCurrentSelectionPos;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editHCountFinished;
	timerHalf = &blinkEditValue;
}

void editHCountFinished()
{
	timerHalf = NULL;
	updateEditValue(0);
	eeprom_write_byte((uint8_t*)((PROG_OFFSET * (uiProgNr - 1)) + PROG_ADDR_H_COUNT), iEditValue);
	iCurrentSelectionPos = iSavedSelectionPos;
	uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawProgrammsEditMenu();
}

void drawSettingsMenu()
{
	uiCurrentMax = MENULINE3;
	RodaryTick = &setSelectionPointer;
	ScrollToNextPage = scrollSettingsMenu;
	initMenuChange();
	uiCurrentMax = 6;

	uint8_t uiIndex = 0;
	MENUTYPE temp;

	temp.Type = DISPLAYTIMEOUT;
	temp.uiId = 0;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = BRIGHTNESS;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = BUZZERONOFF;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = LONGPRESS;
	scrollList[uiIndex] = temp;
	uiIndex++;

	temp.Type = END;
	scrollList[uiIndex] = temp;

	lcd_setcursor(HEADERPOSX, HEADERPOSY);
	lcd_string("Einstellungen");

	iCurrentSelectionPos = uiProgramsEditLastSelection;
	uiCurrentPage = uiProgramsEditLastPage;
	scrollSettingsMenu(uiProgramsEditLastPage);
	setSelectionPointer(0);
}

void scrollSettingsMenu(uint8_t uiPage)
{
	char cLcd[21];
	clearMeanuSection();
	uint8_t uiMenuLine = MENULINE1;
	uiProgramsEditLastPage = uiPage;

	for(uint8_t uiIndex = uiPage * MAXMENULINES; uiIndex < uiPage * MAXMENULINES + MAXMENULINES; uiIndex++)
	{
		if(scrollList[uiIndex].Type == DISPLAYTIMEOUT)
		{
			MenuSelections[uiMenuLine - 2] = &editSecondValue;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			uint8_t uiValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_TIMEOUT);
			if(uiValue > 60)
				sprintf(cLcd, "Dis. Timeout:  Ein");
			else
				sprintf(cLcd, "Dis. Timeout:  %02ds", uiValue);
			lcd_string(cLcd);
		}
		else if(scrollList[uiIndex].Type == BRIGHTNESS)
		{
			MenuSelections[uiMenuLine - 2] = &editBrightness;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			uint8_t uiValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_BRIGHTNESS);
			if(uiValue < 99)
				sprintf(cLcd, "Helligkeit:    %d%%", uiValue);
			else
				sprintf(cLcd, "Helligkeit:   %d%%", uiValue);
			lcd_string(cLcd);
		}
		else if(scrollList[uiIndex].Type == BUZZERONOFF)
		{
			MenuSelections[uiMenuLine - 2] = &editBuzzer;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			if(eeprom_read_byte((const uint8_t*)SETT_ADDR_BUZZER_ONOFF) > 0)
				lcd_string("Buzzer:        Ein");
			else
				lcd_string("Buzzer:        Aus");
		}
		else if(scrollList[uiIndex].Type == LONGPRESS)
		{
			MenuSelections[uiMenuLine - 2] = &editSecondValue;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			sprintf(cLcd, "Long press:     %ds", eeprom_read_byte((const uint8_t*)SETT_ADDR_LONG_PRESS_TIME));
			lcd_string(cLcd);
		}
		else if(scrollList[uiIndex].Type == END)
		{
			MenuSelections[uiMenuLine - 2] = &drawMainMenu;
			lcd_setcursor(MENUSTARTX, uiMenuLine);
			lcd_string("Zur\365ck..");
		}
		uiMenuLine++;
	}
}

void editBrightness()
{
	uiEditValueIsNumeric = 3;
	uiEditValueMax = 100;
	uiEditValueMin = 10;
	iEditValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_BRIGHTNESS);
	uiEditValueX = 16;
	uiEditValueY = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editBrightnessFinished;
	timerHalf = &blinkEditValue;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
}

void editBrightnessFinished()
{
	timerHalf = NULL;
	updateEditValue(0);
	eeprom_write_byte((uint8_t*)SETT_ADDR_DISPLAY_BRIGHTNESS, iEditValue);
	uiBrightness = iEditValue;
	//iCurrentSelectionPos = iSavedSelectionPos;
	//uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawSettingsMenu();
}

void editBuzzer()
{
	uiEditValueIsNumeric = 4;
	uiEditValueMax = 1;
	uiEditValueMin = 0;
	iEditValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_BUZZER_ONOFF);
	uiEditValueX = 17;
	uiEditValueY = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editBuzzerFinished;
	timerHalf = &blinkEditValue;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
}

void editBuzzerFinished()
{
	timerHalf = NULL;
	updateEditValue(0);
	eeprom_write_byte((uint8_t*)SETT_ADDR_BUZZER_ONOFF, iEditValue);
	uiBuzzerOnOff = iEditValue;
	//iCurrentSelectionPos = iSavedSelectionPos;
	//uiCurrentPage = uiSavedPage;
	startBeep(1,0);
	drawSettingsMenu();
}

void editSecondValue()
{
	if(currentMenuItem.Type == DISPLAYTIMEOUT)
	{
		uiEditValueX = 17;
		uiEditValueMax = 70;
		uiEditValueMin = 10;
		uiEditValueIsNumeric = 3;
		iEditValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_TIMEOUT);
		if(iEditValue > 60)
			iEditValue = 70;
	}
	else
	{
		uiEditValueX = 18;
		uiEditValueMax = 5;
		uiEditValueMin = 1;
		uiEditValueIsNumeric = 1;
		iEditValue = eeprom_read_byte((const uint8_t*)SETT_ADDR_LONG_PRESS_TIME);
	}

	uiEditValueY = iCurrentSelectionPos;
	RodaryTick = &updateEditValue;
	RodaryPush = &editSecondValueFinished;
	timerHalf = &blinkEditValue;
	uiProgramsEditLastSelection = iCurrentSelectionPos;
}

void editSecondValueFinished()
{
	timerHalf = NULL;
	updateEditValue(0);

	if(currentMenuItem.Type == DISPLAYTIMEOUT)
	{
		if(iEditValue > 60)
			uiDisplayTimeOut = 250;
		else
			uiDisplayTimeOut = iEditValue;
		eeprom_write_byte((uint8_t*)SETT_ADDR_DISPLAY_TIMEOUT, uiDisplayTimeOut);
	}
	else
	{
		uiLongPressTime = iEditValue;
		eeprom_write_byte((uint8_t*)SETT_ADDR_LONG_PRESS_TIME, iEditValue);
	}

	startBeep(1,0);
	drawSettingsMenu();
}

// Brew Monitor

void loadProg()
{
	uiCurrentProgNr = (uiCurrentPage * MAXMENULINES + iCurrentSelectionPos - 1);
	eeprom_write_byte((uint8_t*) ADDR_CURRENT_PROG, uiCurrentProgNr); // save current prog
	uiCurrentR = 0;
	resetTimer();
	stopTimer(); // only when programm loaded
	drawBrewMonitor(uiCurrentProgNr);
}

void continueBrewMonitor()
{
	if(uiCurrentProgNr > 0)
	{
		drawBrewMonitor(uiCurrentProgNr);
		if(iTimerOn)
		{
			timerHalf = &blinkIndicator;
			RodaryPush = &stopCurrentTimer;
		}
	}
}

void drawBrewMonitor(int8_t iProgNr)
{
	char cLcd[19];
	lcd_clear();
	initMenuChange();
	uiBreMonitorActice = 1;
	RodaryLongPush = &drawBrewMonitorMenu;
	RodaryTick = NULL;
	RodaryPush = &startCurrentTimer;

	uiRCurrentCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + PROG_ADDR_R_COUNT));
	uiHCurrentCount = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + PROG_ADDR_H_COUNT));

	for(uint8_t iR = 0; iR < uiRCurrentCount && iR < PROG_R_COUNT - 1; iR++)
	{
		rTemps[iR] = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + (PROG_R_OFFSET * iR) + PROG_ADDR_R_TEMP));
		rTimes[iR] = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + (PROG_R_OFFSET * iR) + PROG_ADDR_R_TIME));
	}

	for(uint8_t iH = 0; iH < uiHCurrentCount && iH < PROG_H_COUNT - 1; iH++)
		hTimes[iH] = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + (PROG_H_OFFSET * iH) + PROG_ADDR_H_TIME));

	uiKCurrentTime = eeprom_read_byte((const uint8_t*)((PROG_OFFSET * (iProgNr - 1)) + PROG_ADDR_K_TIME));

	// draw
	sprintf(cLcd, "ist: --.-%cC  P%d", (char) 223, iProgNr);
	lcd_setcursor(0, HEADERPOSY);
	lcd_string(cLcd);

	updateBrewMonitor(uiCurrentR);
	blinkIndicator(1);
}

void updateBrewMonitor(uint8_t uiStep)
{
	char cLcd[21], cTemp[5];
	uint8_t uiLineCount = 0;

	clearMeanuSection();
	if(uiStep > uiRCurrentCount + uiHCurrentCount)
	{
		lcd_setcursor(2, MENULINE1);
		lcd_string("Fertig!");
	}
	else
	{
		for(; uiStep < uiRCurrentCount && uiLineCount < 3 && uiStep < PROG_R_COUNT - 1; uiStep++)
		{
			sprintf(cLcd, "R%d %s%cC %02d:%02d:00", uiStep + 1, ftoa(cTemp, (float) rTemps[uiStep] / 2, 1), (char) 223, rTimes[uiStep] / 60, rTimes[uiStep] % 60);
			lcd_setcursor(2, uiLineCount + 2);
			lcd_string(cLcd);
			uiLineCount++;
		}

		if(uiLineCount < 3)
		{
			uiStep++;
			sprintf(cLcd, "K         %02d:%02d:00", uiKCurrentTime / 60, uiKCurrentTime % 60);
			lcd_setcursor(2, uiLineCount + 2);
			lcd_string(cLcd);
			uiLineCount++;
		}

		if(uiLineCount < 3)
		{
			for(; uiStep - uiRCurrentCount - 1 < uiHCurrentCount && uiLineCount < 3 && uiStep - uiRCurrentCount - 1 < PROG_H_COUNT - 1; uiStep++)
			{
				sprintf(cLcd, "H%d        %02d:%02d:00", uiStep - uiRCurrentCount, hTimes[uiStep - uiRCurrentCount - 1] / 60, hTimes[uiStep - uiRCurrentCount - 1] % 60);
				lcd_setcursor(2, uiLineCount + 2);
				lcd_string(cLcd);
				uiLineCount++;
			}
		}
	}
}

void startCurrentTimer()
{
	startTimer(uiCurrentR);
	RodaryPush = &stopCurrentTimer;
	timerHalf = &blinkIndicator;
	startBeep(1,0);
}

void stopCurrentTimer()
{
	stopTimer();
	if(uiBreMonitorActice)
	{
		RodaryPush = &startCurrentTimer;
		timerHalf = NULL;
		blinkIndicator(1);
		startBeep(1,0);
	}
}

void blinkIndicator(uint8_t uiBlink)
{
	lcd_setcursor(0, 2);

	if(uiBlink)
		lcd_data('>');
	else
		lcd_data(' ');
}

void drawBrewMonitorMenu()
{
	char cProg[20];
	uiCurrentMax = MENULINE3;
	RodaryTick = &setSelectionPointer;
	ScrollToNextPage = NULL;
	timerHalf = NULL;
	blinkIndicator(0);
	uiIsBrewMonitor = 0;
	uiBreMonitorActice = 0;
	MenuSelections[0] = &skipCurrentStep;
	MenuSelections[1] = &drawMainMenu;
	MenuSelections[2] = &continueBrewMonitor;
	initMenuChange();

	lcd_setcursor(HEADERPOSX, HEADERPOSY);
	lcd_string("Brau Monitor");

	lcd_setcursor(MENUSTARTX, MENULINE1);
	if(uiCurrentR < uiRCurrentCount)
		sprintf(cProg, "R%d \365berspringen", uiCurrentR + 1);
	else
		sprintf(cProg, "K \365berspringen");
	lcd_string(cProg);

	lcd_setcursor(MENUSTARTX, MENULINE2);
	lcd_string("Hauptmenu");

	lcd_setcursor(MENUSTARTX, MENULINE3);
	lcd_string("Zur\365ck..");
}

void skipCurrentStep()
{
	stopCurrentTimer();
	resetTimer();
	uiCurrentR++;
	continueBrewMonitor();
}
