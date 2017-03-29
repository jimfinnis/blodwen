/**
 * \file
 * A data structure containing master-only data, such as temperatures.
 */

#ifndef __MASTER_H
#define __MASTER_H

#include "slave.h"
#include "regsauto.h"

/// the master data block (i.e. data on the arduino)

struct MasterData {
    /// a "fake" device that looks like an I2C device, but in
    /// fact is used for talking to the master directly.
    SlaveDevice *slave;
    
    
public:
    /// the temperature data as an array of 10 floats, one for the
    /// ambient temperature and one for each slave:
    /// - 0 is ambient
    /// - 1 is slave at I2C address 1: drive and steer for wheel 1
    /// - 2 is slave at I2C address 2: drive and steer for wheel 2
    /// - 3 is slave at I2C address 3: lift for wheels 1 and 2
    /// - 4 is slave at I2C address 1: drive and steer for wheel 3
    /// - 5 is slave at I2C address 2: drive and steer for wheel 4
    /// - 6 is slave at I2C address 3: lift for wheels 3 and 4
    /// - 7 is slave at I2C address 1: drive and steer for wheel 5
    /// - 8 is slave at I2C address 2: drive and steer for wheel 6
    /// - 9 is slave at I2C address 3: lift for wheels 5 and 6
    
    float temps[10];
    
    /// the type of the first exception reported to the master
    int exceptionType;
    /// the slave ID of the first exception reported to the master
    int exceptionSlave;
    /// the motor ID (0/1) of the first exception reported to the master
    int exceptionMotor;
    
    MasterData(SlaveDevice *s){
        slave = s;
    }
    
    void init(){
        slave->setReadSet(READSET_MASTER,
                          REGMASTER_TEMPAMBIENT,
                          REGMASTER_TEMP1,
                          REGMASTER_TEMP2,
                          REGMASTER_TEMP3,
                          REGMASTER_TEMP4,
                          REGMASTER_TEMP5,
                          REGMASTER_TEMP6,
                          REGMASTER_TEMP7,
                          REGMASTER_TEMP8,
                          REGMASTER_TEMP9,
                          REGMASTER_EXCEPTIONDATA,
                          -1);
    }
    
    void update(){
        slave->readRegs(READSET_MASTER);
        
        // here, we read the registers in the order in which they are
        // given in the read set
        
        int rct=0;
        for(int i=0;i<10;i++){
            temps[i] = slave->getRegFloat(rct++);
        }
        int e = slave->getRegInt(rct++);
                                 
        exceptionType = e & 0xff;
        exceptionSlave = (e >> 8)&0xf;
        exceptionMotor = (e >> 12);
    }
};




#endif /* __MASTER_H */
