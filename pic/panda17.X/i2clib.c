/* 
 * File:   i2clib.c
 * Author: Horacio
 *
 * Created on 11 de agosto de 2017, 02:08
 */

#include <pic16f886.h>
#include "panda17.h"
#include "i2clib.h"
#include <stdio.h>

void i2c_init(){
    
    SSPCONbits.SSPM = 0b1000;
    SSPCONbits.SSPEN = 1;   //enables i2c in master mode
    //SSPADD = 0x19;      //Fclk = Fosc /(4*(SSPADD + 1)) = 100KHz
    SSPADD = 0x04;      //Fclk = Fosc /(4*(SSPADD + 1)) = 100KHz
    SSPSTATbits.SMP = 1;    
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;  
}

void i2c_start(){
    while(SSPSTATbits.R_nW | SSPCON2bits.PEN | SSPCON2bits.ACKEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN | SSPCON2bits.SEN);
    SSPCON2bits.SEN = 1;    //START condition
}

void i2c_rstart(){
    while(SSPSTATbits.R_nW | SSPCON2bits.PEN | SSPCON2bits.ACKEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN | SSPCON2bits.SEN);
    SSPCON2bits.RSEN = 1;
}

void i2c_stop(){
    while(SSPSTATbits.R_nW | SSPCON2bits.PEN | SSPCON2bits.ACKEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN | SSPCON2bits.SEN);
    SSPCON2bits.PEN = 1;        //generates STOP condition
}

void i2c_write(unsigned char datum, unsigned char ack){
    
    while(SSPSTATbits.R_nW | SSPCON2bits.PEN | SSPCON2bits.ACKEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN | SSPCON2bits.SEN);
    SSPBUF = datum;        //loads one byte of data
    while(SSPSTATbits.BF);
    if(ack){
        while (SSPCON2bits.ACKSTAT){  //check the ACK polarity
        }
    }
    while (!PIR1bits.SSPIF);    //waits for the interrupt indicating data transmited
}

unsigned char i2c_read(unsigned char ack){
    unsigned char resp;
    static bit flag, not_flag;
    flag = ack & 0x01;
    if (ack == 1){
        not_flag = 0;
    }
    if (ack == 3){
        not_flag = 1;
    }
    while(SSPSTATbits.R_nW | SSPCON2bits.PEN | SSPCON2bits.ACKEN | SSPCON2bits.RCEN | SSPCON2bits.RSEN | SSPCON2bits.SEN);
    SSPCON2bits.RCEN = 1;   //enables receive mode for i2c
    while(!SSPSTATbits.BF);
    while(!PIR1bits.SSPIF);
    SSPCON2bits.ACKDT = not_flag;
    SSPCON2bits.ACKEN = flag;
    resp = SSPBUF;
    while(SSPSTATbits.BF);
    return resp;
}