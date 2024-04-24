#include <avr/io.h>
#include "lcd-routines.h"
#include "main.h"
#include "menu.h"

#define DEBOUNCE	512L	// debounce clock (256Hz = 4msec)

#define PHASE_A		(PIND & 1<<PD3)
#define PHASE_B		(PIND & 1<<PD4)

uint16_t prescaler;
int8_t volatile second = 0;
int8_t iPercentAv = 0;
uint16_t iPercent = 0;		// count seconds
volatile float Tsoll=8.0, fBatVolt = 0, fanalogValue = 0;
volatile uint8_t sleep = 1, iTempMes = 1;
volatile uint8_t encoder_state=0;
volatile uint8_t encoder_buffer[]={'N','N','N','N'};
volatile uint8_t iInactiveHalfSec = 0;
volatile int8_t enc_delta = 0, iTimerOn = 0;
volatile uint16_t iTimerWait = 0;
volatile uint8_t iTimerSpeed = 0, iTimerCount = 0, uiTimerCountSet = 0;
int8_t iHour = 0, iMinute = 0, iBeepRepeat = 0;
uint8_t uiMeassurementCount = 0, uiBeepAgain = 0, uiRodaryPush = 0, uiTimerIsK = 0, uiUpdateBrewMonitor = 0, uiTempError = 0;
uint8_t uiRodaryPressActive = 0, uiBrightness = 100, uiBrightnessCount = 0, uiBuzzerOnOff = 1, uiLongPressTime = 2, uiDisplayTimeOut = 255;

void encode_init();
void init_timer();
int8_t encode_read4();
void resetSelection();
void adc_init();
void adc_start_meas();

char *ftoa(char *a, double f, int precision)
{
	long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
	
	char *ret = a;
	long heiltal = (long)f;
	itoa(heiltal, a, 10);
	while (*a != '\0') a++;
	*a++ = '.';
	long desimal = abs((long)((f - heiltal) * p[precision]));
	itoa(desimal, a, 10);
	return ret;
}

void startBeep(uint8_t uiBeepTimes, uint8_t uiRepeatTimes)
{
	if(uiBuzzerOnOff < 1)
		return;

	iBeepRepeat = uiRepeatTimes;
	uiTimerCountSet = uiBeepTimes - 1;
	PORTD |= (1<<PD6);
	TIMSK0 |= 1<<TOIE0; 
	iTimerSpeed = 0;
	iTimerCount = 0;
}

int main()
{
	_delay_ms(250); // start up delay
	char clcd_Tist[9], clcd_Time[9], cTemp[5];
	int8_t enc_new_delta = 0;
	uint8_t uiToggle = 1;
	fTist = 8;
	timerHalf = NULL;
	RodaryPush = NULL;
	RodaryLongPush = NULL;
	RodaryTick = NULL;
	
	EICRA |= (1 << ISC11) | (1 << ISC10);   //Steigende Flanke von INT1 als ausloeser
	EIMSK |= (1 << INT1);                  //Global Interrupt Flag fuer INT1
	EICRA |= (1<<ISC00);   //jede Flanke von INT0 als ausloeser
	EIFR &= ~(1<<INTF0);
	EIMSK  |= (1<<INT0);		//Global Interrupt Flag fuer INT0
	sei();								//Interrupt aktivieren

	// Ext interrupts pullup on
	PORTD |= (1<<DDD2);					// Int Pull UP
	PORTD |= (1<<DDD3);
	
	// relais output
	DDRB |= (1<<PB0);
	PORTB &= ~(1<<PB0);
	DDRD |= (1<<PD1);	// light
	//PORTD |= (1<<PD1);
	
	// buzzer output
	DDRD |= (1<<PD6);
	PORTD &= ~(1<<PD6);
	// buzzer timer
	TCCR0B |= 1<<CS00;
		
	DDRD &= ~(1<<DDD4);	// Rodary A
	DDRD &= ~(1<<DDD3); // Rodary B
	DDRD &= ~(1<<DDD2); // Rodary Push
	PORTD |= (1<<PD4);	// int. pull up
	PORTD |= (1<<PD3);	// int. pull up
	PORTD |= (1<<PD2);  // int. pull up
	
	lcd_init();
	lcd_setcursor(2,2);
	lcd_string("Pflanzbeet Br\341u");
	lcd_setcursor(2,3);
	lcd_string("SudControl v1.00");

	// init EEPROM
	/*uint16_t uiOffset = 0, uiProgOffset = 0;
	char c[11] = {'n', 'e', 'u', '\0', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
	for(uint8_t i = 0; i < 14; i++)
	{
		uiOffset = PROG_OFFSET * i;

		sprintf(c, "Neu_%d", i + 1);
		eeprom_write_block(c, (uint8_t*)uiOffset + PROG_ADDR_NAME, 11);
		eeprom_write_byte((uint8_t*)uiOffset + PROG_ADDR_R_COUNT, (uint8_t)3);
		eeprom_write_byte((uint8_t*)uiOffset + PROG_ADDR_H_COUNT, (uint8_t)3);

		uiProgOffset = 0;
		for(uint8_t iProg = 0; iProg < PROG_R_COUNT; iProg++)
		{
			uiProgOffset = PROG_R_OFFSET * iProg;
			eeprom_write_byte((uint8_t*)uiOffset + uiProgOffset + PROG_ADDR_R_TEMP, (uint8_t)81);
			eeprom_write_byte((uint8_t*)uiOffset + uiProgOffset + PROG_ADDR_R_TIME, (uint8_t)15);
		}

		for(uint8_t iProg = 0; iProg < PROG_H_COUNT; iProg++)
		{
			uiProgOffset = PROG_H_OFFSET * iProg;
			eeprom_write_byte((uint8_t*)uiOffset + uiProgOffset + PROG_ADDR_H_TIME, (uint8_t)15);
		}

		eeprom_write_byte((uint8_t*)uiOffset + PROG_ADDR_K_TIME, (uint8_t)80);
	}*/

	w1_reset();
	init_timer();
	encode_init();
	adc_init();
	adc_start_meas();
	// restore program and step
	uiCurrentProgNr = eeprom_read_byte((const uint8_t*)ADDR_CURRENT_PROG);
	uiCurrentR = eeprom_read_byte((const uint8_t*)ADDR_CURRENT_STEP);

	// load settings
	uiBrightness = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_BRIGHTNESS);
	uiBuzzerOnOff = eeprom_read_byte((const uint8_t*)SETT_ADDR_BUZZER_ONOFF);
	uiLongPressTime = eeprom_read_byte((const uint8_t*)SETT_ADDR_LONG_PRESS_TIME);
	uiDisplayTimeOut = eeprom_read_byte((const uint8_t*)SETT_ADDR_DISPLAY_TIMEOUT);

	_delay_ms(1250);

	lcd_clear();
	wdt_enable(WDTO_1S);
	drawMainMenu();
	startBeep(1, 0);

	while(1)
	{	
		set_sleep_mode(SLEEP_MODE_IDLE );
		sleep_mode();
						
		if(sleep == 1)
		{
			sleep = 0;

			// 1 sec task
			if(iTempMes == 1)
			{
				iTempMes = 0;
				read_meas();
				start_meas();
				drawBatValue();

				if(iTimerOn)
				{
					if(second < 0)
					{
						iMinute--;
						second = 59;
					}
					if(iMinute < 0)
					{
						iHour--;
						iMinute = 59;
					}
				}

				if(uiBreMonitorActice == 1)
				{
					// temp
					if(uiTempError)
						sprintf(clcd_Tist, " --.-%cC", (char) 223);
					else
					{
						if(fTist < 100)
							sprintf(clcd_Tist, " %s%cC", ftoa(cTemp, fTist, 1), (char) 223);
						else
							sprintf(clcd_Tist, "%s%cC", ftoa(cTemp, fTist, 1), (char) 223);
					}

					lcd_setcursor(4, 1);
					lcd_string(clcd_Tist);

					if(iTimerOn)
					{
						sprintf(clcd_Time, "%02d:%02d:%02d", iHour, iMinute, second);
						lcd_setcursor(12, 2);
						lcd_string(clcd_Time);
					}

					if(uiUpdateBrewMonitor > 0)
					{
						uiUpdateBrewMonitor++;
						if((uiUpdateBrewMonitor > 4 && !uiTimerIsK) || (uiUpdateBrewMonitor > 8 && uiTimerIsK))
						{
							uiUpdateBrewMonitor = 0;
							updateBrewMonitor(uiCurrentR);
						}
					}
				}

				if(iTimerOn)
				{
					if(iHour == 0 && iMinute == 0 && second == 0)
					{
						stopCurrentTimer();
						++uiCurrentR;
						saveCurrentStep();
						if(uiBreMonitorActice == 1)
							uiUpdateBrewMonitor = 1;
						second = 0;
						iInactiveHalfSec = 0;
						if(uiTimerIsK)
							startBeep(4, 3);
						else
							startBeep(4, 1);
					}

					if(uiTimerIsK)
					{
						if(uiCurrentR - uiRCurrentCount < uiHCurrentCount && hTimes[uiCurrentR - uiRCurrentCount] / 60 == iHour && hTimes[uiCurrentR - uiRCurrentCount] % 60 == iMinute && second == 0)
						{
							++uiCurrentR;
							saveCurrentStep();
							if(uiBreMonitorActice == 1)
								uiUpdateBrewMonitor = 1;
							iInactiveHalfSec = 0;
							startBeep(4, 3);
						}
					}
				}

				if(uiBeepAgain > 0)
				{
					uiBeepAgain++;
					if(uiBeepAgain > 2)
					{
						startBeep(uiTimerCountSet + 1, iBeepRepeat);
						uiBeepAgain = 0;
					}
				}
			}

			// 500ms task
			uiToggle = !uiToggle;
			if(timerHalf && iInactiveHalfSec > 0)
				(*timerHalf)(uiToggle);

			if(uiUpdateBrewMonitor > 0 && uiBreMonitorActice == 1)
			{
				if(uiTimerIsK && uiCurrentR - uiRCurrentCount - 1 < uiHCurrentCount)
					lcd_setcursor(12, 3);
				else
					lcd_setcursor(12, 2);
				if(uiToggle)
				{
					if(uiTimerIsK && uiCurrentR - uiRCurrentCount - 1 < uiHCurrentCount)
					{
						sprintf(clcd_Time, "%02d:%02d:00", hTimes[uiCurrentR - uiRCurrentCount - 1] / 60, hTimes[uiCurrentR - uiRCurrentCount - 1] % 60);
						lcd_string(clcd_Time);
					}
					else
						lcd_string("00:00:00");
				}
				else
					lcd_string("        ");
			}

			if(iInactiveHalfSec == 8)
				resetSelection();
		}
		
		// check rodary must be in main! Otherwise issues with lcd when interupt driven since an lcd command can be interupted (cli() and sei() is no solution because of buzzer!)
		if(uiRodaryPush == 1)
		{
			uiRodaryPressActive = 0;
			uiRodaryPush = 0;
			iInactiveHalfSec = 0;
			(*RodaryPush)();
		}
		else if(RodaryLongPush && (uiRodaryPush == 2 || (uiRodaryPressActive && iInactiveHalfSec > (uiLongPressTime * 2) - 1)))
		{
			uiRodaryPressActive = 0;
			uiRodaryPush = 0;
			iInactiveHalfSec = 0;
			(*RodaryLongPush)();
		}

		enc_new_delta = encode_read4();
		if(enc_new_delta != 0)
		{
			iInactiveHalfSec = 0;
			if(RodaryTick)
				(*RodaryTick)(enc_new_delta);
		}
		wdt_reset();		
	}
	
	return 0;
}

void drawBatValue()
{
	char clcd_Bat[6];
	sprintf(clcd_Bat, "%d%%", iPercentAv);
	if(iPercentAv > 99)
		lcd_setcursor(16,1);
	else
		lcd_setcursor(17,1);
	lcd_string(clcd_Bat);
}

void resetSelection()
{
	//sleep = 1;
	TIMSK2 &= ~(1<<OCIE2A);	// stop encode timer as not needed right now
	EIFR |= (1<<INTF0);	 // reset interrupt flag bit otherwise ISR would be called right after enableing because this bit gets set everytime 
	EIMSK  |= (1<<INT1);  // start ext interrupt to start encode timer when rodary is turned
}


///////////////////////////////////////////////////////////
// rodary push
ISR(INT0_vect)
{
	if(!(PIND & (1<<PC2)))
	{
		iInactiveHalfSec = 0;
		uiRodaryPressActive = 1;
	}
	else if(uiRodaryPressActive)
	{
		if(RodaryLongPush && iInactiveHalfSec > (uiLongPressTime * 2) - 1)
			uiRodaryPush = 2;
		else if(RodaryPush)
			uiRodaryPush = 1;
	}
}

// rodary turn start encode timer
ISR(INT1_vect)
{
	// start encode!
	iInactiveHalfSec = 0;
	sleep = 1;
	TIMSK2 |= 1<<OCIE2A;  // start encode timer
	EIMSK  &= ~(1<<INT1); // stop ext interrup not needed right now
}

ISR (TIMER1_COMPA_vect)
{
	uchar tcnt1h = TCNT1H;

	OCR1A += F_CPU / DEBOUNCE;		// new compare value

	if( ++prescaler == (uint16_t)DEBOUNCE ){
		prescaler = 0;
		if(iTimerOn)
			second--; // exact one second over
		if(iInactiveHalfSec < 200)
			iInactiveHalfSec++;
		sleep = 1;	
		iTempMes = 1;
		#if F_CPU % DEBOUNCE			// handle remainder
		OCR1A += F_CPU % DEBOUNCE; 		// compare once per second
		#endif
	}
	else if(prescaler == ((uint16_t) DEBOUNCE) / 2)	// half a sec
	{
		sleep = 1;
		if(iInactiveHalfSec < 200)
			iInactiveHalfSec++;
	}

	if(iInactiveHalfSec > (uiDisplayTimeOut * 2))
		PORTD &= ~(1<<PD1); // turn off backlight
	else
	{
		if(uiBrightnessCount > 9)
			uiBrightnessCount = 0;

		if(uiBrightnessCount > uiBrightness / 10 - 1)
			PORTD &= ~(1<<PD1);
		else
			PORTD |= (1<<PD1);
		uiBrightnessCount++;
	}

	TCNT1H = tcnt1h;			// restore for delay() !
}

void init_timer( void )
{
	TCCR1B = 1<<CS10;			// divide by 1
	OCR1A = 0;
	TCNT1 = -1;
	second = 0;
	prescaler = 0;
	TIMSK1 |= 1<<OCIE1A;
}

void encode_init()
{
	TCCR2A = 1<<WGM21;
	TCCR2B = 1<<CS22;		// CTC, XTAL / 64
	OCR2A = (uint8_t)(F_CPU / 64 * 1e-3 - 0.5);	// 2ms
}

int8_t encode_read4()			// read four step encoders
{
	int8_t val;

	cli();
	val = enc_delta;
	enc_delta &= 3;
	sei();
	return val >> 2;
}


ISR(TIMER2_COMPA_vect)				// 2ms for manual movement
{
	static int8_t last;
	int8_t new, diff;

	new = 0;
	if( PHASE_A )
	new = 3;
	if( PHASE_B )
	new ^= 1;					// convert gray to binary
	diff = last - new;				// difference last - new
	if( diff & 1 ){				// bit 0 = value (1)
		last = new;					// store new as next last
		enc_delta += (diff & 2) - 1;		// bit 1 = direction (+/-)
	}
}

ISR(TIMER0_OVF_vect)
{
	if(iTimerSpeed > 10)
	{
		if(iTimerWait < 500)
			DDRD ^= (1<<DDD6);
		else
			PORTD &= ~(1<<PD6);
		
		if(iTimerWait > 1000)
		{
			iTimerWait = 0;
			iTimerCount++;
			PORTD |= (1<<PD6);
		}

		if(iTimerCount > uiTimerCountSet)
		{
			TIMSK0 &= ~(1<<TOIE0);
			PORTD &= ~(1<<PD6);
			iTimerSpeed = 0;
			iTimerCount = 0;

			if(iBeepRepeat > 0)
			{
				iBeepRepeat--;
				uiBeepAgain = 1;
			}
			return;
		}
		
		iTimerWait++;
		iTimerSpeed = 0;
	}
	iTimerSpeed++;
} 

void adc_init()
{
	//Enable ADC, set 128 prescale and enable the interrupt
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);
	ADMUX |= (1 << MUX2) | (1 << MUX1) | (1 << REFS0) | (1 << REFS1); // adc channel 6 AVcc as Vref	
	//sei();  //Enable interrupts
}

void adc_start_meas()
{
	ADCSRA |= (1<<ADSC);
}

ISR(ADC_vect,ISR_NOBLOCK)
{
	fBatVolt = (float) (ADC * 4.2 * 1.1) / 1024.0;
	
	if(fBatVolt > 4.29)
		fBatVolt = 4.2;
	
	int8_t iPercentTemp = ((fBatVolt-3.3) /  0.88) * 100;	// 4.18V is 100% 3.3 is 0% 
	if(iPercentTemp > 100)
		iPercentTemp = 100;
	else if(iPercentTemp < 0)
		iPercentTemp = 0;
	iPercent += iPercentTemp;
	uiMeassurementCount++;
	
	if(uiMeassurementCount > 80)
	{
		iPercentAv = iPercent / uiMeassurementCount;
		uiMeassurementCount = 0;
		iPercent = 0;
	}
}

void startTimer(uint8_t iR)
{
	if(!iTimerOn && iHour == 0 && iMinute == 0 && second == 0)
	{
		uiCurrentR = iR;

		if(iR + 1 > uiRCurrentCount)
		{
			iHour = uiKCurrentTime / 60;
			iMinute = uiKCurrentTime % 60;
			uiTimerIsK = 1;
		}
		else
		{
			iHour = rTimes[iR] / 60;
			iMinute = rTimes[iR] % 60;
			uiTimerIsK = 0;
		}
		second = 0;
		iTimerOn = 1;
	}
	else if(!iTimerOn) // resume
		iTimerOn = 1;
}

void stopTimer()
{
	iTimerOn = 0;
}

void resetTimer()
{
	iHour = 0;
	iMinute = 0;
	second = 0;
	uiTimerIsK = 0;
}

void saveCurrentStep()
{
	eeprom_write_byte((uint8_t*) ADDR_CURRENT_STEP, uiCurrentR);
}
