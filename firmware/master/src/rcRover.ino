/**
 * @file
 * This file deals with the details
 * of doing remote control on the rover,
 * as opposed to rc.h/ino which just read the RC
 * servo data and massage it into -1 to 1 ranges.
 * It's going to be messy in here.
 * 
 */

#include <stdint.h>
#include <avr/wdt.h>
#include <util/twi.h>

#include <Wire.h>
#define PI 3.1415927f
#define DEGSTORADS (PI/180.0f)
#define RADSTODEGS (180.0f/PI)

#include <math.h>


#include "hwconfig.h"

#include "i2c.h"
#include "master.h"
#include "rcRover.h"

extern class Device *getDeviceByAddr(int addr);

#define MAXSPEED 1300.0f
#define MAXANGLE 30.0f
#define STEERDEADZONE 0.3f
#define DRIVEDEADZONE 0.2f

static int steerMode=0;

static int getDriveSteerDevice(int wh){
    switch(wh){
    case 1:return 1;
    case 2:return 2;
    case 3:return 4;
    case 4:return 5;
    case 5:return 7;
    case 6:return 8;
    }
}

static int getSteerCalib(int i){
    switch(i){
        // wheel 1
    case 0:return -56;
    case 1:return 66;
        
        // wheel 2
    case 2:return -62;
    case 3:return 59;
        
        // wheel 3
    case 4:return -59;
    case 5:return 64;
        
        
        // wheel 4
    case 6:return -60;
    case 7:return 65;
        
        // wheel 5
    case 8:return -66;
    case 9:return 57;
        
        // wheel 6
    case 10:return -62;
    case 11:return 60;
    }
}

void RoverRCReceiver::calibrate(){
    
    for(int i=0;i<10;i++){
        Device *dev= getDeviceByAddr(i);
        dev->writeRegister(REG_RESET,RESET_EXCEPTIONS);
    }
    
    
    for(int i=1;i<=6;i++){
        // drive
        I2CDevice *dev= (I2CDevice *)getDeviceByAddr(getDriveSteerDevice(i));
        dev->writeRegFloat(REGDS_DRIVE_PGAIN,0.01f);
        dev->writeRegFloat(REGDS_DRIVE_IGAIN,0);
        dev->writeRegFloat(REGDS_DRIVE_DGAIN,0);
        dev->writeRegFloat(REGDS_DRIVE_INTEGRALCAP,0);
        dev->writeRegFloat(REGDS_DRIVE_INTEGRALDECAY,0);
        dev->writeRegFloat(REGDS_DRIVE_OVERCURRENTTHRESH,500);
        dev->writeRegFloat(REGDS_DRIVE_STALLCHECK,255);
        
        dev->writeRegFloat(REGDS_STEER_PGAIN,0);
        dev->writeRegFloat(REGDS_STEER_IGAIN,6);
        dev->writeRegFloat(REGDS_STEER_DGAIN,0);
        dev->writeRegFloat(REGDS_STEER_INTEGRALCAP,300);
        dev->writeRegFloat(REGDS_STEER_INTEGRALDECAY,0.9f);
        dev->writeRegFloat(REGDS_STEER_OVERCURRENTTHRESH,500);
        dev->writeRegFloat(REGDS_STEER_STALLCHECK,255);
        dev->writeRegFloat(REGDS_STEER_DEADZONE,1);
        
        int j=(i-1)*2;
        dev->writeRegFloat(REGDS_STEER_CALIBMIN,getSteerCalib(j));
        dev->writeRegFloat(REGDS_STEER_CALIBMAX,getSteerCalib(j+1));
        
        // don't bother calibrating lift
    }
}



static void setWheelPos(int wheel, float pos){
    I2CDevice *dev= (I2CDevice *)getDeviceByAddr(getDriveSteerDevice(wheel));
    dev->writeRegFloat(REGDS_STEER_REQPOS,pos);
}


inline float acotf(float f){
    return atan(1.0f/f);
}
inline float cotf(float f){
    return 1.0f/tan(f);
}

// wheel separation
#define SEPARATION 42.0f
// HALF the true wheelbase, because our turning circle is based on the midline.
#define WHEELBASE (53.0f*0.5f)


static void setSteer(float outerWheelAngle){
    float innerWheelAngle;
    
    bool neg;
    neg = outerWheelAngle<0;
    if(neg)
        outerWheelAngle= -outerWheelAngle;
    
    if(outerWheelAngle<5.0f)
        innerWheelAngle = outerWheelAngle; // deal with the singularity
    else {
        // calculate innerWheelAngle angle, ackerman steering
        innerWheelAngle = acotf((WHEELBASE * cotf(outerWheelAngle*DEGSTORADS) +
                      SEPARATION)/WHEELBASE);
        innerWheelAngle *= RADSTODEGS;
    }
    
//////////////////////////// SHOULDNT BE HERE //////////////
//innerWheelAngle=outerWheelAngle;
//////////////////////////// DEBUGGING CODE ////////////////
    
    if(neg){
        innerWheelAngle= -innerWheelAngle;
        outerWheelAngle= -outerWheelAngle;
    }
    
    switch(steerMode){
    case 0:// normal
        if(outerWheelAngle<0){
            // left turn
            // odd is right side, low numbers are back wheels
            setWheelPos(1,-outerWheelAngle);
            setWheelPos(2,-innerWheelAngle);
            setWheelPos(3,0);
            setWheelPos(4,0);
            setWheelPos(5,outerWheelAngle);
            setWheelPos(6,innerWheelAngle);
        }else{
            // right turn
            // odd is right side, low numbers are back wheels
            setWheelPos(1,-innerWheelAngle);
            setWheelPos(2,-outerWheelAngle);
            setWheelPos(3,0);
            setWheelPos(4,0);
            setWheelPos(5,innerWheelAngle);
            setWheelPos(6,outerWheelAngle);
        }
        break;
    default: //crab
        setWheelPos(1,outerWheelAngle);
        setWheelPos(2,outerWheelAngle);
        setWheelPos(3,outerWheelAngle);
        setWheelPos(4,outerWheelAngle);
        setWheelPos(5,outerWheelAngle);
        setWheelPos(6,outerWheelAngle);
    }
}

static void setDrive(float speed){
    for(int i=1;i<=6;i++){
        I2CDevice *dev= (I2CDevice *)getDeviceByAddr(getDriveSteerDevice(i));
        dev->writeRegFloat(REGDS_DRIVE_REQSPEED,speed*MAXSPEED);
        
    }
}


void RoverRCReceiver::update(){
    RCReceiver::update();
    
    if(!ready())return;
    
    if(!calibrated){
        calibrate();
        calibrated=true;
    }
    
    float steer = get(1);
    float drive = get(0);
    
    if(steer<STEERDEADZONE && steer>-STEERDEADZONE)
        steer=0;
    if(drive<DRIVEDEADZONE && drive>-DRIVEDEADZONE)
        drive=0;
    
    steerMode = get(2) > 0.75f;
    steerPos -= steer*0.03f; // negative because I got it the wrong way round
    
        
    if(steerPos<-MAXANGLE)steerPos=-MAXANGLE;
    if(steerPos>MAXANGLE)steerPos=MAXANGLE;
    
    setSteer(steerPos);
    setDrive(drive);
}


void RoverRCReceiver::onUp(){
}
    
    
void RoverRCReceiver::onDown(){
}
