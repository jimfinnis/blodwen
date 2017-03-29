/**
 * @file
 * Abstraction of a device, which is typically an I2C device but may be local. It's made
 * up of registers.
 * 
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "../../common/regs.h"


#define E_NOSUCHREG 1
#define E_READONLY 2

class Device {
protected:
    /// static copy of register definition
    static Register reg;
    /// register table in program memory
    MAYBEPROGMEM Register *table;
    
    /// internal method to check register correct,
    /// will load it into SRAM.
    int checkAndLoadRegister(int r,bool writeRequested){
        if(r>=numRegs)
            return E_NOSUCHREG;
        else {
            loadReg(r);
            if(writeRequested && !reg.writable())
                return E_READONLY;
            else 
                return 0;
        }
    }
    
    /// load a register definition from the table
    void loadReg(int n){
        memcpy_P(&reg,table+n,sizeof(Register));
    }
        
    /// read set used by protocol - these are
    /// SHARED BY ALL DEVICES
    static uint8_t readSet[READSETS][READSETSIZE];
    static uint8_t readSetCt[READSETS];
    
public:
    uint8_t numRegs; //!< highest register number
    
    Device(MAYBEPROGMEM Register *r){
        table = r;
        // count registers - the terminator is a silly register size
        numRegs=0;
        for(;;){
            loadReg(numRegs); // load in each register
            if(reg.getSize()>30)break;
            numRegs++;
        }
    }
    
    /// initialise the read set
    static void clearReadSet(uint8_t s){
        readSetCt[s]=0;
    }
    
    /// add a register to the read set
    static void addReadSet(uint8_t s,uint8_t r){
        readSet[s][readSetCt[s]++]=r;
    }
    
    /// get s read set and its count
    static const uint8_t *getReadSet(uint8_t s,uint8_t *ct){
        *ct = readSetCt[s];
        return readSet[s];
    }
    
    /// get the size of a register from the register
    /// table
    int getRegisterSize(int r){
        loadReg(r);
        return reg.getSize();
    }
    
    /// override this method to provide register write semantics
    virtual int writeRegister(byte reg,uint16_t val)=0;
    /// override this method to provide register read semantics
    virtual int readRegister(byte reg,uint16_t *readValue)=0;

    
};




#endif /* __DEVICE_H */
