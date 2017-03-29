/**
 * \file Slave I2C implementation
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <Wire.h>
#include "slave_i2c.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

using namespace I2CSlave;

/// maximum register number
static int numRegs;
/// register info table
static const Register *regs;

extern volatile unsigned long lastmsgtime;

/// cached register values
static volatile uint16_t *regVals;
/// has a register changed? Set in receiveEvent, cleared
/// after notifications sent in poll().
static volatile byte *regChanged;

/// the "current register" we have been set to read
static volatile int currentreg;

/// the head of the listener list
static I2CSlaveListener *headListener=NULL;

/// called on receipt of data from the master (the master is writing to us)
static void receiveEvent(int ct){
    byte buf[5];
    
    lastmsgtime = millis();
    
    // copy msg into temp buffer
    for(int i=0;i<ct;i++){
        byte c = Wire.read();
        if(i<5)
            buf[i]=c;
    }
    
    // byte 0 is the current register number,
    // which may be the only thing we receive (we may later get
    // a request, in which case we'll send this value)
    
    currentreg=buf[0];
    
    // handle out of range by always using var 0.
    if(currentreg<0 || currentreg>=numRegs)
        currentreg=0;
    
    ct--; // ct now number of data bytes
    if(ct){
        // ah, there's more - we must be setting the value too.
        // copy the value over
        memcpy((byte *)(regVals+currentreg),buf+1,regs[currentreg].getSize());
#if defined(IMMEDIATE_PERMITTED)
        for(I2CSlaveListener *s=headListener;s;s=s->next){
            s->changeEventImmediate(currentreg,regVals[currentreg]);
        }
#endif
        regChanged[currentreg]=1; // notify change
    }
}


void I2CSlave::poll(){
    // we look at one register each time we call this, covering all
    // of them in NUMREGS polls
    static int i=0;
    
    // inform any listeners of changes since last time.
    if(regChanged[i]){
        for(I2CSlaveListener *s=headListener;s;s=s->next){
            s->changeEvent(i,regVals[i]);
        }
        regChanged[i]=0;
    }
    i++;
    i %= numRegs;
}
    

static void requestEvent(){
    int sz = regs[currentreg].getSize();
    if(sz){
        Wire.write((byte *)(regVals+currentreg),sz);
    } else
        Wire.write((byte)0xcd); // just write a dummy if unknown
}



void I2CSlave::init(int addr,const Register *r){
    headListener = NULL;
    
    regs=r;
    numRegs=0;
    while((r++)->getSize()<30)
        numRegs++;
    
    // allocate data
    regVals = (uint16_t *)malloc(sizeof(uint16_t)*numRegs);
    regChanged = (byte *)malloc(numRegs);
    
    // clear registers
    for(int i=0;i<numRegs;i++){
        regChanged[i]=0;
        regVals[i]=0;
    }
    
    Wire.begin(addr);
/*    
    cbi(PORTC,4); // disable internal pullups
    cbi(PORTC,5);
    
    TWSR = 3; // slow down
    TWBR = 0x20;
*/    
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
}

void I2CSlave::addListener(I2CSlaveListener *l){
    l->next = headListener;
    headListener = l;
}


void I2CSlave::setRegisterInt(int n,uint16_t v){
    regVals[n]=v;
}

void I2CSlave::setRegisterFloat(int n,float v){
    regVals[n]=regs[n].map(v);
}
