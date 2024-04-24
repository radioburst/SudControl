#ifndef _main_h_
#define _main_h_

#ifndef __AVR_ATmega168__
#define __AVR_ATmega168__
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>


#define uchar unsigned char
#define uint unsigned int
#define bit uchar
#define idata
#define code

#define W1_PIN	PD0
#define W1_IN	PIND
#define W1_OUT	PORTD
#define W1_DDR	DDRD

// EEPROM Addresses
// PROGRAM MEMORY
#define ADDR_CURRENT_PROG 9
#define ADDR_CURRENT_STEP 10

// SETTINGS
#define SETT_ADDR_BUZZER_ONOFF 15
#define SETT_ADDR_DISPLAY_TIMEOUT 16
#define SETT_ADDR_DISPLAY_BRIGHTNESS 17
#define SETT_ADDR_LONG_PRESS_TIME 18
#define SETTINGS_OFFSET 40

// BREW PROGRAMS
#define PROG_COUNT 14
#define PROG_OFFSET 33
#define PROG_R_COUNT 7
#define PROG_R_OFFSET 2
#define PROG_H_COUNT 5
#define PROG_H_OFFSET 1

#define PROG_ADDR_NAME 		SETTINGS_OFFSET + 1
#define PROG_ADDR_R_COUNT 	SETTINGS_OFFSET + 12
#define PROG_ADDR_R_TEMP 	SETTINGS_OFFSET + 13
#define PROG_ADDR_R_TIME 	SETTINGS_OFFSET + 14
#define PROG_ADDR_H_COUNT 	PROG_R_OFFSET * PROG_R_COUNT + PROG_ADDR_R_TEMP
#define PROG_ADDR_H_TIME 	PROG_ADDR_H_COUNT + 1
#define PROG_ADDR_K_TIME 	PROG_H_OFFSET * PROG_H_COUNT + PROG_ADDR_H_TIME

volatile char cIsttemp[20];
volatile float fTist;
volatile int8_t iTimerOn;
uint8_t uiBrightness, uiBuzzerOnOff, uiLongPressTime, uiDisplayTimeOut, uiTempError;

void drawBatValue();
char *ftoa(char *a, double f, int precision);

void startBeep(uint8_t uiBeepTimes, uint8_t uiRepeatTimes);
void stopBeep();

void startTimer(uint8_t iR);
void stopTimer();
void resetTimer();

void saveCurrentStep();

#include "1wire.h"
#include "ds18b20.h"

#endif
