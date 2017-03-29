/**
 * @file This is the code for the Arduino I2C master for the rover,
 * controlling up to nine serial driver boards.
 */

#include <stdint.h>
#include <avr/wdt.h>
#include <util/twi.h>

#include <Wire.h>

#include "hwconfig.h"

#include "i2c.h"
#include "master.h"
#include "rcRover.h"

#ifndef cbi
/// handy macro for clearing register bits (turning off pullups)
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#define BLUELED 7

/// this is a table of 9 bytes, giving the 2nd byte of the addresses
/// of the temperature sensors in sensor order.

const byte sensorAddrs[] PROGMEM ={
    0x9c, // ambient sensor
    // slave sensors
    0x20,0x51,0xa8,0x89,
    0x2f,0xb1,0xb5,0x6f, 0x9e};




/// the master device representing registers actually on the arduino
/// master itself
MasterDevice master;
Register Device::reg;
State state;
RoverRCReceiver rc;


uint8_t Device::readSet[READSETS][READSETSIZE];
uint8_t Device::readSetCt[READSETS];


/// an array of slave devices, one for each I2C device. Some are DS devices
/// and some are LL devices!

#define NUMSLAVES 9
I2CDevice slaves[] = {
    I2CDevice(1,registerTable_DS),
    I2CDevice(2,registerTable_DS),
    I2CDevice(3,registerTable_LL),
    I2CDevice(4,registerTable_DS),
    I2CDevice(5,registerTable_DS),
    I2CDevice(6,registerTable_LL),
    I2CDevice(7,registerTable_DS),
    I2CDevice(8,registerTable_DS),
    I2CDevice(9,registerTable_LL),
};

class Device *getDeviceByAddr(int addr){
    if(addr==0)
        return &master;
    else if(addr<=NUMSLAVES)
        return slaves+(addr-1);
    else halt(10);
    
    return NULL; //never happens
    
}


/// when we last got a USB message
unsigned long lastmsgtime=0;

/// debugging light counter
int debledct=0;

/// handy debug method  - displays a sequence of N flashes with a pause
/// between them, m times. If m is -100, does it forever.

void flashdeb(int n,int m){
    while(m==-100 || m--){
        for(int i=0;i<n;i++){
            delay(200);
            digitalWrite(13,LOW);
            delay(200);
            digitalWrite(13,HIGH);
        }
        delay(1000);
    }
}

/// panic method - flashes n times, forever
void halt(int n){
    wdt_disable();
    flashdeb(n,-100);
}

/// a binary reader which listens for serial messages whose first byte is their length,
/// and then passes them into a process() method.

class BinarySerialReader {
    int ct;
    uint8_t buf[256];
    
protected:
    /// takes command block sans first (count) character and parses it.
    /// Also takes the payload length (length of message AFTER the 
    /// command byte)
    virtual void process(int ct,uint8_t *p) = 0;
    
public:
    /// initialise, zeroing the byte count - meaning we're waiting
    /// for length byte
    BinarySerialReader(){
        ct=0;
    }
    /// build up a buffer - once the length of the buffer is equal to
    /// the first byte read, process it.
    void poll(){
        if(Serial.available()){
            buf[ct++]=Serial.read();
            if(buf[0]==ct){
                // send the buffer without the size byte,
                // and the payload length (message size minus
                // size and command bytes)
                // ONLY DO THIS if the RC isn't running
                if(!rc.ready())
                    process(ct-2,buf+1);
                ct=0;
            }
        }
    }
};

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

/// an implementation of BinarySerialReader which processes read and write messages
/// and reads and writes SlaveDevice registers appropriately

class MySerialReader : public BinarySerialReader {
    /// process a set of register writes
    void dowrite(Device *s, uint8_t *p){
        int writes = *p++; // get the number of writes
        for(int i=0;i<writes;i++){ // for each write
            int r = *p++; // get the register number
            uint16_t v = *p++; // get the value (low byte)
            if(s->getRegisterSize(r)==2){ // if the value is 16-bit
                v|= *p++<<8; // get the value (high byte)
            }
            // and write the value over I2C
            wdt_reset();
            s->writeRegister(r,v);
        }
        wdt_reset();
        Serial.write((uint8_t)0); // just to say it's done
    }
    
    /// process a set of register reads
    void doread(Device *s,uint8_t *p){
        uint8_t buf[128];
        int ct=0;
        
        uint8_t set = *p++; // get the read set index
        
        // get the read set itself
        uint8_t readSetCt;
        const uint8_t *readSet = Device::getReadSet(set,&readSetCt);
        
        for(int i=0;i<readSetCt;i++){ // for each read
            uint16_t v;
            uint8_t r = readSet[i];
            wdt_reset();
            s->readRegister(r,&v); // get the register value from I2C
            buf[ct++]=v & 0xff; // store the bottom byte in the buffer
            if(s->getRegisterSize(r)==2) // if the value is 16-bit
                buf[ct++]=v>>8; // store the top byte
        }
        wdt_reset();
        Serial.write(buf,ct); // write the buffer
    }
    
    void doreadset(uint8_t *p,int ct){
        uint8_t set = *p++;
        ct--; // subtracting 1 because of the readset index
        Device::clearReadSet(set);
        for(int i=0;i<ct;i++){
            Device::addReadSet(set,*p++);
        }
        wdt_reset();
        Serial.write(ct);
    }
    
protected:
    /// takes command block sans first (count) character and parses it, reading
    /// the command number and device ID, and passing the rest of the block into
    /// appropriate command methods.
    virtual void process(int ct,uint8_t *p){
        int id = *p>>4; // get the addr/id
        
        // now get the device from the addr
        Device *s = getDeviceByAddr(id);
        
        switch(*p++&0xf){ // get the command and increment the pointer
        case 2: //write command
            dowrite(s,p);
            break;
        case 3: //read command
            doread(s,p);
            break;
        case 4:// read set command
            doreadset(p,ct);
            break;
        default:break;
        }
        lastmsgtime = millis();
                
    }
};

int globalException = 0;

/// an exception handler which will send messages to all other slaves,
/// telling them to raise an exception. Note that this will KEEP HAPPENING
/// every time we poll that bad slave.

class MyExceptionListener : public ExceptionListener {
    virtual void onException(char slave,char motor,char type){
        // there should be no existing exception state. Boot and remote
        // exceptions are ignored by the exception reader.
/*        
        Serial.println("Got an exception:");
        Serial.print((int)type);
        Serial.print(" on ");
        Serial.println((int)slave);
*/        
        if(!state.exception) { 
            slave--; // decrement to give the slave number
            for(int i=0;i<NUMSLAVES;i++){
                if(i!=slave) { // don't send to originator!
                    // tell the other slave about the problem
//                    Serial.print("telling slave ");
//                    Serial.println(i);
                    slaves[i].writeRegister(REG_EXCEPTIONDATA,100);
                }
            }
        }
        globalException = type;
    }
};

MySerialReader serialReader;
MyExceptionListener exceptionListener;

/// start I2C and serial, and wait a little while for the
/// other units to start up before we can command them

void setup()
{
    Serial.begin(115200); // start the serial IO
    Serial.println("Starting");
    
    Wire.begin(); // join I2C bus as the master
    pinMode(13,OUTPUT);
    pinMode(BLUELED,OUTPUT);
    digitalWrite(13,HIGH);
    digitalWrite(13,LOW);
    
    
    cbi(PORTC,4); // disable internal pullups
    cbi(PORTC,5);
    
    master.init(); // start up one-wire
    state.addExceptionListener(&exceptionListener); // initialise exception listener
    
    rc.init(); // initialise and calibrate the radio receiver
    
    wdt_enable(WDTO_1S);
    for(int i=0;i<NUMSLAVES;i++){
        slaves[i].reset();
    }
    wdt_enable(WDTO_120MS);
    
    //    TWSR = 3; // slow down
    //    TWBR = 0x20;
    
    wdt_reset();
    Serial.print(__DATE__);Serial.print(" ");Serial.println(__TIME__);
    Serial.println(freeRam());
    Serial.println("Ready");
}

/// main loop - just handle characters coming in on
/// the serial port.

void loop()
{
    static unsigned long lastt1=0;
    static unsigned long lastt2=0;
    
    // update the RC receiver data
    rc.update();
        
    if(globalException){
        static uint32_t curexceptionflash=0;
        uint32_t x = curexceptionflash>>3;
        
        if(curexceptionflash++ == 80000)
            curexceptionflash=0;
        
        int modu = x%1000;
        
        int qqq = globalException*1000;
        digitalWrite(BLUELED,modu<300 && x<qqq);
    }
    else 
        digitalWrite(BLUELED,rc.ready()?HIGH:LOW);
    
    // every now and then, ping each device.
    // This will timeout if the device has died.
    // Only do this once we've waited long enough
    // for the devices to come up.
    
    unsigned long t = millis();
    if(t>5000){
        if(t>lastt1 && (t-lastt1)>1000){
            lastt1=t;
            digitalWrite(13,HIGH);
            debledct=100;
            for(int i=0;i<NUMSLAVES;i++){
                slaves[i].update();
            }
        }
        if(t>lastt2 && (t-lastt2)>2000){
            lastt2=t;
            master.tick();
        }
        
        // check that we had a USB message recently
//        if(t>lastmsgtime && (t-lastmsgtime)>400){
//            flashdeb(10,-100); // halt and reboot
//        }
    }
    
    if(debledct && !--debledct){
        digitalWrite(13,LOW);
    }
    
    
    wdt_reset();
    serialReader.poll();
}
