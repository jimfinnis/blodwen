/**
 * @file
 * Master device - address zero, fakes being an I2C device but is actually
 * local.
 * 
 */

#ifndef __MASTER_H
#define __MASTER_H

#include "device.h"
#include "../../common/regsauto.h"
#include <OneWire.h>
#include "temperature.h"

class MasterDevice : public Device {
    Temperature temp;
    float temperatures[10];
    int curtempdev;
public:    
    MasterDevice() : Device(registerTable_MASTER){
        // start temperature
        curtempdev=0;
        temp.requestTemp(curtempdev);
    }
    
    /// initialise onewire and search for devices
    void init(){
        temp.init();
    }
          
        
    
    /// reset will reset the master's exception state
    
    virtual int writeRegister(byte r,uint16_t val){
        int rv;
        if(rv=checkAndLoadRegister(r,true))return rv;
        
        switch(r){
        case REG_DISABLEDEXCEPTIONS:
            state.setDisabledExceptions(val);
            break;
        case REGMASTER_RESET:
            state.reset();
            break;
        }
        
        return 0;
    }
    
    virtual int readRegister(byte r,uint16_t *readValue){
        int rv;
        float t;
        int i;
        
        if(rv=checkAndLoadRegister(r,false)){
            // not ideal, putting the bad return value into
            // the data
            *readValue=rv;
            return rv;
        }
        switch(r){
        case REGMASTER_EXCEPTIONDATA:
            if(state.exception){
                i = state.exType;
                i |= state.exSlave<<8;
                i |= state.exMotor<<12;
            }
            else i=0;
            *readValue = i;
            break;
        case REGMASTER_TEMPAMBIENT:
        case REGMASTER_TEMP1:
        case REGMASTER_TEMP2:
        case REGMASTER_TEMP3:
        case REGMASTER_TEMP4:
        case REGMASTER_TEMP5:
        case REGMASTER_TEMP6:
        case REGMASTER_TEMP7:
        case REGMASTER_TEMP8:
        case REGMASTER_TEMP9:
            t = temperatures[r-REGMASTER_TEMPAMBIENT];
            *readValue = reg.map(t);
            break;
        default:
            *readValue = reg.map(-1);
        }
        return 0;
    }
    
    /// called every few seconds to start reading the next temp
   
    void tick(){
        temperatures[curtempdev]=temp.getTemp(curtempdev++);
        if(curtempdev==temp.getDevCount()){
            curtempdev=0;
        }
        temp.requestTemp(curtempdev);
    }
          
    
};


#endif /* __MASTER_H */
