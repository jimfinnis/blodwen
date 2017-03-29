/**
 * \file
   Communications with a drive motor via a slave device.
 *
 */


#ifndef __DRIVE_H
#define __DRIVE_H

#include "slave.h"
#include "motor.h"

/// drive motors are accessed via the REGDS_DRIVE_ registers,
/// and there is only one drive motor on a slave device, which
/// must be a DS board (drive/steer).
class DriveMotor : public Motor{
    /// basic parameter block for drive motors
    MotorParams params;
public:
    /// constructor, specifying the slave we're talking to
    DriveMotor(SlaveDevice *s):Motor(s){}
    
    virtual void sendParams(){
        slave->startWrites();
        slave->writeFloat(REGDS_DRIVE_PGAIN,params.pGain);
        slave->writeFloat(REGDS_DRIVE_IGAIN,params.iGain);
        slave->writeFloat(REGDS_DRIVE_DGAIN,params.dGain);
        slave->writeFloat(REGDS_DRIVE_INTEGRALCAP,params.iCap);
        slave->writeFloat(REGDS_DRIVE_INTEGRALDECAY,params.iDecay);
        slave->writeFloat(REGDS_DRIVE_OVERCURRENTTHRESH,params.overCurrentThresh);
        slave->writeFloat(REGDS_DRIVE_STALLCHECK,params.stallCheck);
        slave->writeFloat(REGDS_DRIVE_DEADZONE,params.deadZone);
        slave->endWrites();
    }
    
    /// reset the odometer - just make sure you haven't sent *another*
    /// kind of reset recently, or this'll overwrite it.
    virtual void resetOdometer(){
        slave->startWrites();
        slave->writeInt(REG_RESET,RESET_ODO);
        slave->endWrites();
    }
        
    /// return the current parameter block for modification
    /// or examination
    virtual MotorParams *getParams(){
        return &params;
    }
    
    /// send a new speed request
    virtual void setRequired(float speed){
        slave->startWrites();
        slave->writeFloat(REGDS_DRIVE_REQSPEED,speed);
        slave->endWrites();
        required = speed;
    }
};
    



#endif /* __DRIVE_H */
