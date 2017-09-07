/* 
 * File:   ds1307.c
 * Author: Horacio
 *
 * Created on 17 de agosto de 2017, 16:42
 */

#include <stdio.h>
#include <stdlib.h>
#include "i2clib.h"
#include "ds1307.h"

void ds_set(unsigned char *data){ /*y, unsigned char m, unsigned char d, unsigned char h, unsigned char min*/
    unsigned char j;
    for (j=0;j<7;j++){
        i2c_start();
        i2c_write(0b11010000,1);
        i2c_write(j,1);
        i2c_write(*(data+j),1);
        i2c_stop();
//        __delay_ms(100);
    }
}

unsigned char ds_get(unsigned char data){
    unsigned char resp;
    i2c_start();
    i2c_write(0b11010000,1);
    i2c_write(data,1);
    i2c_rstart();
    i2c_write(0b11010001,1);
    resp = i2c_read(0);
    i2c_stop();
    return resp;
}

/* 
 * 0x00 = 0b0sss ssss
 * 0x01 = 0b0mmm mmmm
 * 0x02 = 0b00hh hhhh
 * 0x03 = 0b0000 0ddd -> day of the week
 * 0x04 = 0b00DD DDDD
 * 0x05 = 0b000M MMMM
 * 0x06 = 0bYYYY YYYY
 */