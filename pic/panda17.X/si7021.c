/* 
 * File:   si7021.c
 * Author: Horacio
 *
 * Created on 16 de agosto de 2017, 21:00
 */

#include <stdio.h>
#include <stdlib.h>
#include "i2clib.h"
#include "si7021.h"

void si_reset(){
    i2c_start();
    i2c_write(0x80,1);
    i2c_write(0xFE,1);
    i2c_stop();
}

unsigned int si_read_h (){
    unsigned int rta = 0;
    unsigned char rta_h,rta_l;
    i2c_start();
    i2c_write(0x80,1);
    i2c_write(0xE5,1);      //E5: independent, E0 of last hum measure
    i2c_rstart();
    i2c_write(0x81,1);
    rta_h = i2c_read(1);
    rta_l = i2c_read(3);
    i2c_stop();
    rta = rta_h;
    rta = rta << 8;
    rta += rta_l;
    return rta;
}

unsigned int si_read_t (){
    unsigned int rta;
    unsigned char rta_h,rta_l;
    i2c_start();
    i2c_write(0x80,1);
    i2c_write(0xE3,1);
    i2c_rstart();
    i2c_write(0x81,1);
    rta_h = i2c_read(1);
    rta_l = i2c_read(3);
    i2c_stop();
    rta = rta_h;
    rta = rta << 8;
    rta += rta_l;
    return rta;
}