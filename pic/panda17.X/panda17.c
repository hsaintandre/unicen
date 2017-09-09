/* 
 * File:   panda17.c
 * Author: Horacio SA
 *
 * Created on 10 de agosto de 2017, 17:11
 */

#include <stdio.h>
#include <stdlib.h>
#include "panda17.h"
#include "i2clib.h"
#include "24lc512.h"
#include "si7021.h"
#include "ds1307.h"

//global vars
unsigned char milis = 0,led = 5, condition = 0;
unsigned char time[7] = {0,0,0,0,0,0,0};
unsigned int secs = 0;
static bit fyh;

void uart_init(){       //Mandar esto a otra librería
    PIE1bits.RCIE = 1;  //enables EUSART reception interrupt
    PIE1bits.TXIE = 0;  //disables uart transmision interrupt
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 0;
    SPBRGH = 0x00;
    //SPBRG = 0x33;     //9600bps @ Fosc=8MHz
    SPBRG = 0x0C;       //9600bps @ Fosc=2MHz
    TXSTAbits.SYNC = 0;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1; //enables receiver
}

void putch(char data){      //Mandar esto a otra librería
    while(!PIR1bits.TXIF)
        continue;
    TXREG = data;
}

void mem_init(){
    unsigned char aux;
    aux = mem_read(0x0002);
    if (aux != 0xAA){
        mem_write(0x0002,0xAA); //0xAA means memory initialized
        mem_write(0x0003,0x00); //enables date saving
        mem_write(0x000A,0x00); //sets measures pointers to zero
        mem_write(0x000B,0x00);
        mem_write(0x000C,0x00);
        mem_write(0x000D,0x00);
    }
}

void save_the_date(){
    unsigned char ax;    //supports upto 10 events start/stop
    ax = ds_get(0x01);
    mem_write(0xFFFA,ax); //hour
    ax = ds_get(0x02);
    mem_write(0xFFF9,ax); //min
    ax = ds_get(0x04);
    mem_write(0xFFFB,ax); //day
    ax = ds_get(0x05);
    mem_write(0xFFFC,ax); //month
    ax = ds_get(0x06);
    mem_write(0xFFFD,ax); //year
    ax = mem_read(0x000A);
    mem_write(0xFFFE,ax);   //saves also the amount data
    ax = mem_read(0x000B);
    mem_write(0xFFFF,ax);
    mem_write(0x0003,0xAA); //date already saved
}

void interrupt ints_isr(void){
/***************************** uart_isr ***************************************/
    if (PIR1bits.RCIF){
        INTCONbits.GIE = 0;
        static bit ferr = 0, oerr = 0;
        unsigned char rcv = 0;    
        ferr = RCSTAbits.FERR;  //checks framing error
        oerr = RCSTAbits.OERR;  //checks overrun
        if (!ferr & !oerr){
            rcv = RCREG;        //reads char
        }
        switch (rcv){
            case 'a':   //Send data saved
                PORTBbits.RB4 = 1;
                unsigned int dt = 0,add,n,aux;
                unsigned int aux2;
                add = mem_read(0x0A);
                add = add << 8;
                add += mem_read(0x0B);  //gets the amount of saved data
                //printf("Give\n");
                printf("I%05d\r\n",add);
                for (n=0;n<add;n++){
                    aux = mem_read(2*n+0x0010);
                    aux = aux << 8;
                    aux += mem_read(2*n+0x0011);        
                    printf("%04x",aux);
                    aux = mem_read(2*n+0x7FF7);
                    aux = aux << 8;
                    aux += mem_read(2*n+0x7FF8);
                    printf("%04x",aux);
                }
                printf("Z");
                aux2 = mem_read(0xFFF9);
                printf("%02x:",aux2);
                aux2 = mem_read(0xFFFA);
                printf("%02x ",aux2);
                aux2 = mem_read(0xFFFB);
                printf("%02x/",aux2);
                aux2 = mem_read(0xFFFC);
                printf("%02x/",aux2);
                aux2 = mem_read(0xFFFD);
                printf("%02x\t",aux2);
                aux2 = mem_read(0xFFFE);
                printf("%02x",aux2);
                aux2 = mem_read(0xFFFF);
                printf("%02x",aux2);
                printf("\r\n");
                printf("X");    //this determines the end of the transmision
                PORTBbits.RB4 = 0;
                break;
            case 'b':   //Set RTC date and time
                for (unsigned char z = 0;z<7;z++){
                    while(!PIR1bits.RCIF);   //waits for the incoming byte
                    ferr = RCSTAbits.FERR;  //checks framing error
                    oerr = RCSTAbits.OERR;  //checks overrun
                    if (!ferr & !oerr){
                        time[z] = RCREG;        //reads char
                    }
                }
                ds_set(time);                
                break;
            case 'c':   //clears lower memory
                PORTBbits.RB4 = 1;
                mem_write(0x0002,0x00); //clears initialization
                mem_init(); //this allow the system to continue run again without need of a reset
                PORTBbits.RB4 = 0;
                if ((mem_read(0x0002) == 0xAA) && (mem_read(0x000D) == 0x00)){  //double check
                    printf("OK");
                }
                break;
            case 'd':
                for (unsigned char k=0;k<7;k++){
                    printf("%X ",ds_get(k));
                }
                break;
            case 'm':
                printf("I%04x;",si_read_h()); 
                printf("%04x",si_read_t());
                printf("X");
                break;
//            default: return 0;
        }
        INTCONbits.GIE = 1;
    }
/***************************** uart_isr ***************************************/
    
/*************************** timer2_isr ***************************************/    
    if (PIR1bits.TMR2IF){
        PIR1bits.TMR2IF = 0;
        INTCONbits.GIE = 0;
        if (!PORTBbits.RB2){
            led = 5;
            milis = 0;
            secs = 0;          
        }
        if (PORTBbits.RB2){
            if (mem_read(0x0003) != 0xAA){
                save_the_date();
            }
            milis++;
            if(milis > 6){        //24 gives 1 sec @40ms
                if (led > 0){ //led 5 secs turned on at the begining of the acquisition
                    PORTBbits.RB4 = 1;
                    led--;
                }
                else {
                    PORTBbits.RB4 = 0;
                }
                secs++;
                milis = 0;
                if (secs == 1){    //initialize in 1800 to start saving a set of data
                    condition = 2;  //this is to avoid if() large nesting
                }
            }
            if (condition == 2){
                condition = 0;
                secs = 0;
                /********************************** data saving ***************************/
                unsigned char arg;
                unsigned int temp_add,hum_add,up;
                unsigned int hum_val,temp_val;
                //this part carries out the measures
                hum_val = si_read_h();      
                temp_val = si_read_t();
                //this part gets the address pointers (it's actually the amount of data saved, it's enough with only one)
                hum_add = mem_read(0x000A);   //0x0A:0x0B hum address pointer
                hum_add = hum_add << 8;
                hum_add += mem_read(0x000B);
                up = hum_add + 1;
                arg = up >> 8;
                mem_write(0x000A,arg);
                arg = up & 0x00FF;
                mem_write(0x000B,arg);
                hum_add = (hum_add * 2) + 1 + 0x000F; //0x10: first hum value
                temp_add = mem_read(0x000C);  //0x0C:0x0D temp address pointer
                temp_add = temp_add << 8;
                temp_add += mem_read(0x000D);
                up = temp_add + 1;
                arg = up >> 8;
                mem_write(0x000C,arg);
                arg = up & 0x00FF;
                mem_write(0x000D,arg);
                temp_add = (temp_add * 2) + 1 + 0x7FF6; //0x7FF7 first temp value
                //this part saves the values
                arg = hum_val >> 8;     
                mem_write(hum_add,arg);
                arg = hum_val & 0x00FF;
                mem_write(hum_add + 1,arg);
                arg = (temp_val >> 8) & 0x00FF;
                mem_write(temp_add,arg);
                arg = temp_val & 0x00FF;
                mem_write(temp_add + 1,arg);
                /********************************** data saving ***************************/ 
            }
        }   
        INTCONbits.GIE = 1;
    }
/*************************** timer2_isr ***************************************/
}

void init_timer2(unsigned char pre, unsigned char post, unsigned char eoc){
    unsigned char sfr = 0x00;
    PR2 = eoc;  //sets the end of count
    sfr = ((post-1) << 3) & 0x78;   //places postscaler's bits in their position and applies a mask
    sfr += 4;   //enables timer2
    sfr += pre; //places prescaler's bits
    T2CON = sfr;
    PIE1bits.TMR2IE = 1;    //enables tmr2 interrupt
}



int main(void) {
    OSCCON = 0b01010001;    //internal osc @2MHz
    INTCON = 0xC0;  //enables global and peripheral interrupts
    
    TRISA = 0x00;
    TRISB = 0x00;    //all output
    ANSELHbits.ANS8 = 0;
    TRISBbits.TRISB2 = 1;   //input for initializations
    PORTBbits.RB4 = 0;
    uart_init();
    i2c_init();
    __delay_ms(10);
    mem_init();
    si_reset();
    fyh = 1;
    //printf("\n");
            
    __delay_ms(200);
    init_timer2(2,5,250);   //40ms @Fosc=2MHz
    while(1){     
    }
}