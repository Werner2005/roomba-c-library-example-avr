/*
    Copyright (C) 2013 Andreas Werner <webmaster@andy89.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <roomba.h>
#include <roomba_command.h>
#include "uart.h"

#define UART_BAUD_RATE 115200
//#define UART_BAUD_RATE 57600

void error(){
	//static int i = 0;
	/*
	 * gehe in Error Mode Deaktivie alle Interrupts
	 */
	//cli();
	for(;;){
		//uart_putc(ROOMBA_SENSORS);
		//uart_putc(ROOMBA_SENSOR_OI_MODE);
		uart_putc(0x5a);
		PORTD ^= _BV(PD6);
		_delay_ms(100);
	}
}

int writer(unsigned char c){
    	uart_putc(c);
	return 0;
}

void reader(){
        int c = uart_getc();
        if ( c & UART_NO_DATA ){

	}else{
            /*
             * new data available from UART
             * check for Frame or Overrun error
             */
            if ( c & UART_FRAME_ERROR || c & UART_OVERRUN_ERROR || c & UART_BUFFER_OVERFLOW){
	    	return;
	    }else{
		roomba_read((uint8_t) c);
	    }

	}
	
}

int waitms(unsigned int ms){
	int i;
	for(i = ms; i > 0; i-=10){
		_delay_ms(10);
	}
	return 0;
}

void initRoomba(){
	int8_t ret;
	roombaSensorData_t s;

	roomba_command_init(&roomba_sendCommand, waitms);

	ret = roomba_start();

	if(ret < 0){
		error();
	}

	ret = roomba_send(ROOMBA_SENSORS, 1, ROOMBA_SENSOR_OI_MODE);

	if(ret < 0){
		error();
	}
	/*
	 * Stelle dich auf entfanngen ein
	 */
	for(ret = 0; ret < 100; ret++){
		reader();
		_delay_ms(100);
	}
	if(roomba_hasSensorData()){
		ret = roomba_getSensorData(&s);
		if(ret < 0){
			error();
		}

		if(s.data != ROOMBA_SENSOR_OI_MODE_SAFE){
			error();
		}
	}else{
		error();
	}
}

void testCommands(){
	roomba_drive(20, 0);
	_delay_ms(3000);	
	roomba_drive(-20, 0);
	_delay_ms(3000);	
	roomba_drive(0, 0);
}

int main(int argc, char** argv){
	DDRD = (1 << DDD6);
	PORTD = 0;

    	uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(UART_BAUD_RATE,F_CPU)); 
    	sei();
	
	roomba_init(&writer);	

	initRoomba();

	testCommands();

	do{
		PORTD ^= _BV(PD5);
		_delay_ms(1000);
		reader();
	}while(1);
	return 0;
}
