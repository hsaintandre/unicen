/* 
 * File:   24lc512.c
 * Author: Horacio
 *
 * Created on 16 de agosto de 2017, 20:28
 */

#include "24lc512.h"
#include "i2clib.h"
#include <stdio.h>
#include <pic16f886.h>

void mem_write(unsigned int add, unsigned char data){
    unsigned char add_h,add_l;
    add_h = (add >> 8) & 0x00FF;
    add_l = add & 0x00FF;
    i2c_start();
    i2c_write(0b10100000,0);        //mem address
    i2c_write(add_h,0);              //adress_HIGH
    i2c_write(add_l,0);              //adress_LOW
    i2c_write(data,0);              //data
    i2c_stop();
    
    i2c_start();
    i2c_write(0b10100000,0);
    
    while(SSPCON2bits.ACKSTAT){
        i2c_start();
        i2c_write(0b10100000,0);
    }
    i2c_stop();
}

unsigned char mem_read(unsigned int add){
    unsigned char rta;
    unsigned char add_h,add_l;
    add_h = (add >> 8) & 0x00FF;
    add_l = add & 0x00FF;
    i2c_start();
    i2c_write(0b10100000,0);
    i2c_write(add_h,0);
    i2c_write(add_l,0);
    i2c_start();
    i2c_write(0b10100001,0);
    rta = i2c_read(0);
    i2c_stop();
    return rta;
}