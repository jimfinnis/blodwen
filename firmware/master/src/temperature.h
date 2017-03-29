/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#include <avr/pgmspace.h>
#include <OneWire.h>
#include "../../common/state.h"

#define MAXSENSORS 10
extern const byte sensorAddrs[] PROGMEM;
    
class Temperature {
private:
    OneWire ds;
    byte devct;
    byte addrs[MAXSENSORS][8];
    
    
public:
    Temperature() : ds(6) {
        devct=0;
    }
    
    void praddr(byte *p){
        for(int j=0;j<8;j++){
            if(p[j]<16)
                Serial.print('0');
            Serial.print(p[j],HEX);
            if(j<7)
                Serial.print(' ');
        }
    }
    
    void init(){
        for(int i=0;i<MAXSENSORS;i++){
            if(!ds.search(addrs[devct])){
                break;
            }
//            Serial.print("device found: ");
//            praddr(addrs[devct]);
//            Serial.println("");
            
              devct++;
        }
        
        ds.reset_search();
    }
    
    /// get the 2nd byte of the address we're looking for
    /// from the sensor ID table, then scan the address table for it
    /// and return its index in the that table
    int getAddrIdx(uint8_t sensorID){
        byte v = pgm_read_byte_near(sensorAddrs+sensorID);
        for(int i=0;i<devct;i++){
            if(addrs[i][1]==v)
                return i;
        }
        // oh noes!
        state.raiseException(0,0,10);
    }
    
    
    void requestTemp(int i){
        ds.reset();
        ds.select(addrs[getAddrIdx(i)]);
        ds.write(0x44,1); // start convo, with parasite
    }
    
    int getDevCount(){
        return devct;
    }
    
    float getTemp(int i){
        ds.reset();
        ds.select(addrs[getAddrIdx(i)]);
        ds.write(0xbe); // read scratch
        
        byte data[9];
        for(int i=0;i<9;i++)
            data[i] = ds.read();
        
        int lo = data[0];
        int hi = data[1];
        int t = (hi<<8)+lo;
        int sign = t & 0x8000;
        if(sign)
            t = (t ^ 0xffff)+1; // 2's complement
        // now this is in 0.5 degree increments
        float temp = t/2;
        if(lo&1)temp+=0.5f;
        if(sign)temp=-temp;
        return temp;
    }
    
    
};


#endif /* __TEMPERATURE_H */
