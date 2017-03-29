/**
 * \file I2C abstraction (for master, currently).
 * \author $Author$
 * \date $Date$
 */

#ifndef __I2C_H
#define __I2C_H

#include "device.h"
#include "../../common/state.h"
#include "../../common/regsauto.h"

/// class for an object encapsulating a register interface
/// for I2C master comms. Does NOT initialise the Wire
/// library. Represents a link to a single device.

class I2CDevice : public Device {
    /// the device address
    byte addr;
public:
    
    /// reset the required I2C device.
    
    void reset(){
        writeRegister(REG_RESET,RESET_HARD);
    }
    
    /// update - check the device and check its exception status,
    /// may not return if there is a comms failure
    void update(){
        uint16_t r;
        writeRegister(REG_PING,0); // ping
        readRegister(REG_STATUS,&r);
        
        if(r & ST_EXCEPTION){
            // best find out what it is
            readRegister(REG_EXCEPTIONDATA,&r);
            int t = r & 0xff;
            int id = r>>8;
            // raise it into our exception status, which
            // causes the listener in sketch.ino to run.
            if(t!=EX_REMOTESLAVE && t!=EX_BOOT)
                state.raiseException(addr,id,t);
        }
    }
    
    int getAddr(){
        return addr;
    }
    
    /// initialise - does not set up Wire, but initialise the address
    /// and register table
    
    I2CDevice(byte a, MAYBEPROGMEM Register *r) : Device(r) {
        addr = a;
    }
    
        
    
    /// write a register on a device - sends the
    /// byte register number, followed by the bytes
    /// of the value in little-endian (LSB first) format.
    virtual int writeRegister(byte r,uint16_t val){
        byte buf[8]; // buffer of data to send
        
        int rv;
        if(rv=checkAndLoadRegister(r,true))return rv;
        
        Wire.beginTransmission(addr); // start transmit
        buf[0]=r; // register number
        
        // add the actual data
        memcpy(buf+1,(byte*)&val,reg.getSize());
                   
        // send the contents as a byte array (it's little
        // endian, so that works out quite well.
        Wire.write(buf,1+reg.getSize());
        
        // and end - this actually does the sending,
        // the stuff before this just buffers.
        Wire.endTransmission();
        return 0;
    }
    
    /// used by remote control code
    int writeRegFloat(byte r, float v){
        int rv;
        if(rv=checkAndLoadRegister(r,true))return rv;
        writeRegister(r,reg.map(v));
    }
    
    /// read the value of a register. Returns 0 or an error code,
    /// stores the read value in the pointer.
    virtual int readRegister(byte r,uint16_t *readValue){
        int rv;
        
        if(rv=checkAndLoadRegister(r,false))return rv;
        
        // send a message indicating that we should
        // switch the current register on that device
        
        Wire.beginTransmission(addr); // start transmit
        Wire.write(r); // send reg number
        Wire.endTransmission();
        *readValue = 0;
        
        // request a read
        Wire.requestFrom((int)addr,(int)reg.getSize());
        
        // await reply and build value - slave *may* send
        // less than requested
        int v=0; // value, which arrives as LSB first
        int s=0;
        
        // stall while no data there yet
        while(!Wire.available()){
            Wire.requestFrom((int)addr,(int)reg.getSize());
        }
            
        // once data's there, read it
        while(Wire.available()){
            int c = (int)Wire.read();
            v |= c<<s;
            s+=8;
        }
        *readValue = v;
        return 0;
    }
};


#endif /* __I2C_H */
