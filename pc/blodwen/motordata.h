/**
 * \file
 * This file contains classes dealing with reading motor boards: since
 * each board has two motors, we typically read data for both motors
 * in a single read. Classes from this file are used to do this.
 */


#ifndef __MOTORDATA_H
#define __MOTORDATA_H

#include "slave.h"
#include "regsauto.h"


/// general class which deals with reading data from both types of board - it's
/// subclassed by the specific board class. It also deals with firing
/// off the actual read command.

struct MotorDriverData {
    int timer; 			//!< the current time on this slave
    int interval;		//!< the control interval for this slave
    int status;			//!< the status of the slave
    int exceptionID;		//!< the ID field of the exception register
    int exceptionType;		//!< the type field of the exception register
    
protected:
    void readData(SlaveDevice *slave, int set){
        rct=0; // set the counter
        
        slave->readRegs(set); // do the read
        
        // here, we read the registers in the order in which they are
        // given in the read set
        
        timer = slave->getRegInt(rct++);
        interval = slave->getRegInt(rct++);
        status = slave->getRegInt(rct++);
        int exceptionData = slave->getRegInt(rct++);
        
        // these are the last exception to occur - not valid
        // if status bit not set.
        exceptionID = exceptionData>>8;
        exceptionType = exceptionData&0xff;
        
/*        printf("Driver: %d data: %x id: %x type: %x\n",
               slave->getAddr(),
               exceptionData,
               exceptionID,
               exceptionType);
*/               
        
        // there will be more reads in each subclass
    }
    
    /// used to handle updating indices
    /// automatically
    int rct;
};

/// stuff that's in all motors

struct MotorData {
    float error;
    float errorIntegral;
    float errorDeriv;
    float control;
    float intervalCtrl;
    float current;
    float actual; // either speed or position
    int exceptionType; // if the motor is in exception
};

/// the data for each steer motor (currently just a MotorData, really)
struct SteerMotorData : public MotorData {
};

/// the data for each drive motor
struct DriveMotorData : public MotorData {
    uint32_t odometer;
};

/// the data for each lift motor - it's the same as a steer motor, but
/// having a different type makes the calls typesafe.
struct LiftMotorData : public MotorData {
};

/// this class encapsulates reading data from a drive/steer motor
/// board
class DriveSteerMotorDriverData : public MotorDriverData {
    SlaveDevice *slave; //!< the slave device we're communicating with
public:
    
    SteerMotorData steer; //!< data about the steer motor
    DriveMotorData drive; //!< data about the drive motor
    float chassis;     //!< chassis inclinometer reading (may be invalid)
    
    /// initialise the system, saying which slave we're on
    DriveSteerMotorDriverData(SlaveDevice *s){
        slave = s;
    }
    
    /// initialise - sends the IDs of registers we want to read to the 
    /// board
    
    void init(){
//        printf("   setting readset on device %d\n",slave->getAddr());
        slave->setReadSet(READSET_DRIVESTEER, // this block must agree with MotorDriverData
                           REG_TIMER,
                           REG_INTERVALI2C,
                           REG_STATUS,
                           REG_EXCEPTIONDATA,
                           // now our stuff
                           REGDS_CHASSIS,
                           
                           REGDS_DRIVE_ACTUALSPEED,
                           REGDS_DRIVE_ERROR,
                           REGDS_DRIVE_ERRORINTEGRAL,
                           REGDS_DRIVE_ERRORDERIV,
                           REGDS_DRIVE_CONTROL,
                           REGDS_DRIVE_INTERVALCTRL,
                           REGDS_DRIVE_CURRENT,
                           REGDS_DRIVE_ODO,
                           
                           REGDS_STEER_ACTUALPOS,
                           REGDS_STEER_ERROR,
                           REGDS_STEER_ERRORINTEGRAL,
                           REGDS_STEER_ERRORDERIV,
                           REGDS_STEER_CONTROL,
                           REGDS_STEER_INTERVALCTRL,
                           REGDS_STEER_CURRENT,
                           -1);
    }
    
    /// update - first calls readData in the superclass, which will
    /// actually do the read, and then copies the values into our structure.
    
    void update(){
        readData(slave,READSET_DRIVESTEER); // does read, and updates the first few things
        
        // must agree with block above
        
        chassis = slave->getRegFloat(rct++);
        
        drive.actual = slave->getRegFloat(rct++);
        drive.error = slave->getRegFloat(rct++);
        drive.errorIntegral = slave->getRegFloat(rct++);
        drive.errorDeriv = slave->getRegFloat(rct++);
        drive.control = slave->getRegFloat(rct++);
        drive.intervalCtrl = slave->getRegFloat(rct++);
        drive.current = slave->getRegFloat(rct++);
        drive.odometer = slave->getRegInt(rct++);
        
        steer.actual = slave->getRegFloat(rct++);
        steer.error = slave->getRegFloat(rct++);
        steer.errorIntegral = slave->getRegFloat(rct++);
        steer.errorDeriv = slave->getRegFloat(rct++);
        steer.control = slave->getRegFloat(rct++);
        steer.intervalCtrl = slave->getRegFloat(rct++);
        steer.current = slave->getRegFloat(rct++);
        
        // route the exception type, if any, to the appropriate motor
        if(status & ST_EXCEPTION){
            switch(exceptionID){
            case 0:
                drive.exceptionType = exceptionType;break;
            case 1:
                steer.exceptionType = exceptionType;break;
            default:
                drive.exceptionType = steer.exceptionType = exceptionType;
            }
        }
        else drive.exceptionType = steer.exceptionType = 0;
    }
};

/// this class encapsulates reading data from a drive/steer motor
/// board
class LiftMotorDriverData : public MotorDriverData {
    SlaveDevice *slave; //!< the slave device we're communicating with
public:
    
    /// our lift motor data structures
    LiftMotorData data[2];
    
    /// initialise the system, saying which slave we're on
    LiftMotorDriverData(SlaveDevice *s){
        slave = s;
    }
    
    /// initialise - sends the IDs of registers we want to read to the 
    /// board
    
    void init(){
//        printf("   setting readset on device %d\n",slave->getAddr());
        slave->setReadSet(READSET_LIFT, // this block must agree with MotorDriverData
                           REG_TIMER,
                           REG_INTERVALI2C,
                           REG_STATUS,
                           REG_EXCEPTIONDATA,
                           // now our stuff
                           REGLL_ONE_ACTUALPOS,
                           REGLL_ONE_ERROR,
                           REGLL_ONE_ERRORINTEGRAL,
                           REGLL_ONE_ERRORDERIV,
                           REGLL_ONE_CONTROL,
                           REGLL_ONE_INTERVALCTRL,
                           REGLL_ONE_CURRENT,
                           
                           REGLL_TWO_ACTUALPOS,
                           REGLL_TWO_ERROR,
                           REGLL_TWO_ERRORINTEGRAL,
                           REGLL_TWO_ERRORDERIV,
                           REGLL_TWO_CONTROL,
                           REGLL_TWO_INTERVALCTRL,
                           REGLL_TWO_CURRENT,
                           -1);
    }
public:

    /// update - first calls readData in the superclass, which will
    /// actually do the read, and then copies the values into our structure.
    
    void update(){
        readData(slave,READSET_LIFT); // does read, and updates the first few things
        for(int i=0;i<2;i++){
            data[i].actual = slave->getRegFloat(rct++);
            data[i].error = slave->getRegFloat(rct++);
            data[i].errorIntegral = slave->getRegFloat(rct++);
            data[i].errorDeriv = slave->getRegFloat(rct++);
            data[i].control = slave->getRegFloat(rct++);
            data[i].intervalCtrl = slave->getRegFloat(rct++);
            data[i].current = slave->getRegFloat(rct++);
        }
        // route the exception type, if any, to the appropriate motor
        if(status & ST_EXCEPTION){
            switch(exceptionID){
            case 0:
            case 1:
                data[exceptionID].exceptionType = exceptionType;
                break;
            default:
                data[0].exceptionType = exceptionType;
                data[1].exceptionType = exceptionType;
            }
            
        }else {
            data[0].exceptionType = 0;
            data[1].exceptionType = 0;
        }
            
    }
};



#endif /* __MOTORDATA_H */
