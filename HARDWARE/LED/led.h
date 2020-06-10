#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
					  
////////////////////////////////////////////////////////////////////////////////// 
#define LEDA_Tx   PAout(4) // LED
#define LEDA_Rx   PAout(5) // LED
#define LEDB_Tx   PAout(6) // LED
#define LEDB_Rx   PBout(8) // LED

#define Beep   PBout(9)


											
void LED_Init(void);



void Buzzer(uint8_t time);

		 				    
#endif
