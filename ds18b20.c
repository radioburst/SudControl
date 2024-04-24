#include "main.h"


void start_meas( void ){
  if( W1_IN & 1<< W1_PIN ){
    w1_command( CONVERT_T, NULL );
    W1_OUT |= 1<< W1_PIN;
    W1_DDR |= 1<< W1_PIN;			// parasite power on

  }
}


void read_meas( void )
{
  uchar id[8], diff;
  int16_t temp = 0;

  for( diff = SEARCH_FIRST; diff != LAST_DEVICE; ){
    diff = w1_rom_search( diff, id );

    if( diff == PRESENCE_ERR ){
    	uiTempError = 1;
    	break;
    }
    if( diff == DATA_ERR ){
    	uiTempError = 1;
    	break;
    }
    if( id[0] == 0x28 || id[0] == 0x10 )
	{	// temperature sensor
		uiTempError = 0;
		w1_byte_wr( READ );	// read command
		temp = w1_byte_rd();	// low byte
		temp |= w1_byte_rd() << 8;	// high byte

		fTist = (float) temp / 16.0;
    }
    else{
    	uiTempError = 1;
    	break;
    }
  }
}
