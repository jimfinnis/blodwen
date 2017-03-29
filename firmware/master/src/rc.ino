/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include "rc.h"

static volatile long count1,count2,count3;
static volatile long servo1,servo2,servo3;
static volatile unsigned long ticks=0;
static volatile uint8_t prevBits=0xff;

ISR(PCINT2_vect){
    uint8_t changed = PIND ^ prevBits; // which inputs changed?
    prevBits=PIND;
    
    ticks = micros();
    
    // look at the changed bits and calculate the servo readings.
    
    if(changed & 4){//pin2
        if(PIND & 4) //rising edge?
            count1=ticks;
        else
            servo1=ticks-count1;
    }
    if(changed & 8){//pin3
        if(PIND & 8) //rising edge?
            count2=ticks;
        else
            servo2=ticks-count2;
    }
    if(changed & 16){//pin4
        if(PIND & 16) //rising edge?
            count3=ticks;
        else
            servo3=ticks-count3;
    }
}

void RCReceiver::init(){
    pinMode(2,INPUT);
    pinMode(3,INPUT);
    pinMode(4,INPUT);
    rdy=false;
    cli();
    PCICR|=1<<PCIE2;
    // pins 2,3,4
    PCMSK2 |= (1<<PCINT18)|(1<<PCINT19)|(1<<PCINT20);
    sei();
    
    // feed the -1 to 1 ranges
    setCalib(0,1940,1000);
    setCalib(1,1738,940);
    setCalib(2,910,1800);
}

void RCReceiver::update(){
    cli();
    unsigned long now = micros();
    if(now<ticks)//wraparound, wait until ticks has also wrapped
        return;
    
    bool r = ticks && ((micros()-ticks)<1000000);
    
    if(!r && rdy)
        onDown();
    else if(r && !rdy)
        onUp();
    
    rdy=r;
    
    s[0] = servo1;
    s[1] = servo2;
    s[2] = servo3;
    sei();
}

    
