/**
 * \file
 * This file contains the top-level definitions for the rover system.
 * To use the rover, instantiate a rover object and then call its
 * init() method giving the /dev/tty device you're using.
 * 
 * \page mappings Wheel Mappings
 * Front (connector end): 5 RIGHT 6 LEFT Device 1
 * Middle:                3 RIGHT 4 LEFT Device 2
 * Back:                  1 RIGHT 2 LEFT Device 3
 */


#ifndef __ROVER_H
#define __ROVER_H

#include "drive.h"
#include "steer.h"
#include "lift.h"
#include "motordata.h"
#include "master.h"

#include "sim.h"

    
static const int DRIVE = 0; //!< value for getMotor() etc.
static const int STEER = 1; //!< value for getMotor() etc.
static const int LIFT = 2; //!< value for getMotor() etc.
    
    


/// this class contains all the classes for reading data
/// from and sending data to the motor controller boards
/// which control a single wheel pair.
class WheelPair {
private:
    
    /// each board has a I2C slave device
    SlaveDevice devs[3];
    
    /// boards 1 and 2 are drive/steer boards
    DriveSteerMotorDriverData *dsData[2];
    
    /// board 3 is a lift/lift board
    LiftMotorDriverData *llData;
    
    /// the two drive motors on devices 1 and 2
    DriveMotor *driveMotors[2];
    /// the two steer motors on devices 1 and 2
    SteerMotor *steerMotors[2];
    /// the two lift motors on device 3
    LiftMotor *liftMotors[2];
    
public:
    
    /// reset the exception state on all slaves for this
    /// pair
    void resetExceptions(){
        devs[0].resetExceptions();
        devs[1].resetExceptions();
        devs[2].resetExceptions();
    }

          
    
    /// get the chassis pot value for this wheel pair -
    /// this is undefined until update()
    float getChassisValue(){
        // it's always on the first DS device
        return dsData[0]->chassis;
    }
    
    WheelPair(){
        dsData[0] = new DriveSteerMotorDriverData(devs+0);
        dsData[1] = new DriveSteerMotorDriverData(devs+1);
        llData = new LiftMotorDriverData(&devs[2]);
        
        driveMotors[0] = new DriveMotor(devs+0);
        driveMotors[1] = new DriveMotor(devs+1);
        steerMotors[0] = new SteerMotor(devs+0);
        steerMotors[1] = new SteerMotor(devs+1);
        liftMotors[0] = new LiftMotor(devs+2,0);
        liftMotors[1] = new LiftMotor(devs+2,1);
    }
        
    
    /// initialise all the systems and connect, using
    /// the given protocol, and given the pair number
    /// (devices 1-3 are on pair 0, device 4-6 on pair 1 etc.)
    bool init(SlaveProtocol *protocol, int pair){
        printf("Initialising wheelpair %d...\n",pair);
        
        pair *= 3;
        
        
        // give the slave devices the protocol to talk
        // to and tell them which register set they have
        // and which I2C address they're using.
        
        devs[0].init(protocol,registerTable_DS,1+pair);
        devs[1].init(protocol,registerTable_DS,2+pair);
        devs[2].init(protocol,registerTable_LL,3+pair);
        
        // send commands to set up the register
        // reads for each board
        setReadSets();
        
        return true;
    }
    
    /// send commands to set up the register reads for
    /// each board - if you set a new read set directly
    /// you MUST call this again before calling update().
    
    void setReadSets(){
        dsData[0]->init();
        dsData[1]->init();
        llData->init();
    }
    
    /// update all device data
    
    void update(){
        dsData[0]->update();
        dsData[1]->update();
        llData->update();
    }
    
    /// access to the individual boards may be required;
    /// if you use setReadSet() on the device, be sure
    /// to call setReadSets() afterwards.
    /// @param n The device index, 0-2.
   
    SlaveDevice *getDevice(int n){
        return devs+n;
    }
    
    /// get a pointer to a given motor, given the type (0,1,2) and the motor number (0,1)
    /// used to get common parameters etc. Alternatively use the full getDrive(), getSteer(), getLift()
    /// calls.
    
    Motor *getMotor(int n,int type){
        switch(type){
        case DRIVE:
            return driveMotors[n];
        case STEER:
            return steerMotors[n];
        case LIFT:
            return liftMotors[n];
        default:return NULL;
        }
    }
    
    /// get a pointer to a given motor monitoring data block, given the type and the motor number - 
    /// alternatively use getDriveData(), getSteerData() or getLiftData()
    
    MotorData *getMotorData(int n,int type){
        switch(type){
        case DRIVE:
            return getDriveData(n);
        case STEER:
            return getSteerData(n);
        case LIFT:
            return getLiftData(n);
        default:return NULL;
        }
    }
    
    
    /// get a pointer to the monitoring data for a given drive motor
    /// @param n drive motor number 0-1
    
    DriveMotorData *getDriveData(int n){
        return &(dsData[n]->drive);
    }
    
    /// get a pointer to the monitoring data for a given steer motor
    /// @param n steer motor number 0-1
    
    SteerMotorData *getSteerData(int n){
        return &(dsData[n]->steer);
    }
    
    /// get a pointer to the monitoring data for a given lift motor
    /// @param n lift motor number 0-1
    
    LiftMotorData *getLiftData(int n){
        return &(llData->data[n]);
    }
    
    /// get a pointer to a given drive motor
    /// @param n drive motor number 0-1
    
    DriveMotor *getDrive(int n){
        return driveMotors[n];
    }
    
    /// get a pointer to a given steer motor
    /// @param n steer motor number 0-1
    
    SteerMotor *getSteer(int n){
        return steerMotors[n];
    }
    
    /// get a pointer to a given lift motor
    /// @param n lift motor number 0-1

    LiftMotor *getLift(int n){
        return liftMotors[n];
    }
    
};



/// This is the top level rover class! To create one, call Rover *r =
/// Rover::getInstance() which will create a rover if one doesn't exist, and
/// then return the pointer. Subsequent calls will return the same pointer. 

class Rover {
    /// it's a singleton, so the ctor should be private
    Rover(){
        legCollisionChecksEnabled=false;
        valid = false;
    }
    
    /// the single instance
    static Rover *instance;
    
    /// each of these is a pair of wheels
    WheelPair pair[3];
    /// this is the protocol wrapper around the comms
    SlaveProtocol protocol;
    /// bitmask indicating which wheel pair boards
    /// are present
    int pairsPresent;
    
    
    /// true if the rover has valid comms
    bool valid;
    
    /// this looks like a slave device but is
    /// in fact the arduino master (with address zero)
    SlaveDevice masterDev;
    
    /// pointer to the master's data block
    MasterData *masterData;
    
public:
    /// are leg collision/interference checks enabled?
    bool legCollisionChecksEnabled; 
    
    /// set leg collision check enabled state, return previous value
    bool setLegCollisionChecks(bool f){
        bool ret=legCollisionChecksEnabled;
	legCollisionChecksEnabled=f;
        return ret;
    }
    
    /// return the singleton instance, creating if required.
    static Rover *getInstance(){
        if(!instance)
            instance = new Rover();
        return instance;
    }
    
    /// given a wheel number 0-5, return the pair number
    int getPairIdx(int wheelNumber){
        static int pair[]={0,0,1,1,2,2};
        return pair[wheelNumber];
    }
    /// given a wheel number 0-5, return the wheel number
    /// within the pair
    int getWheelIdx(int wheelNumber){
        static int wheel[]={0,1,0,1,0,1};
        return wheel[wheelNumber];
    }
    
    /// will be true if the comms initialised OK
    bool isValid(){
        return valid;
    }
        
    /// this is the comms device, public so we can set the notifier if
    /// we want
    SerialComms comms;
    
    /// initialise the entire rover.
    /// @param port the serial device to connect to - or null to collect to a standard simulator
    /// @param pp   bitmask of which wheel pair boards are present
    
    bool init(const char *port,int pp=7){
        
        if(!port){
            comms.simConnect(new RoverSimulator());
        } else {
            comms.connect(port,115200);
        }
        if(!comms.isReady())
            return false;
        pairsPresent = pp;
        
        // set up the protocol
        protocol.init(&comms);
        
        for(int i=0;i<3;i++){
            if(pairsPresent & (1<<i)){
                pair[i].init(&protocol,i);
            }
        }
        masterData = new MasterData(&masterDev);
        masterDev.init(&protocol,registerTable_MASTER,0);
        masterData->init(); // sets the read set
        
        // this load of code ensures each lift motor knows its
        // own wheel number, so we can check lift constraints; it
        // also allows the lift motors to reference the rover.
        
        for(int i=1;i<=6;i++){
            LiftMotor *lift = getLift(i);
            lift->wheelNumber = i;
        }
        
        valid = true;
        return true;
    }
    
    /// get the name of a wheel type used by getMotor() or getMotorData()
    static const char *getMotorTypeName(int t){
        static const char *typeNames[]={"drive","steer","lift"};
        return typeNames[t];
    }
    
    /// reset all exceptions - including the boot exception, so
    /// this must be run after initialisation.
    void resetExceptions(){
        if(pairsPresent & 1)pair[0].resetExceptions();
        if(pairsPresent & 2)pair[1].resetExceptions();
        if(pairsPresent & 4)pair[2].resetExceptions();
        masterDev.resetExceptions();
    }
    
    /// update all wheel pairs
    
    void update(){
        if(valid){
            if(pairsPresent & 1)pair[0].update();
            if(pairsPresent & 2)pair[1].update();
            if(pairsPresent & 4)pair[2].update();
            masterData->update();
            comms.pollSim();
            comms.tickSim();
        }
    }
        
    
    /// it may be necessary to get direct access to a
    /// wheel pair - this is sometimes done for chassis value
    /// access. 
    /// @param n wheel pair - for chassis, 0=rear 1=left 2=right
    WheelPair *getPair(int n){
        return pair+n;
    }
    
    /// get a pointer to a motor by wheel and type
    /// @param n wheel number 1-6
    /// @param t type 0,1,2 (drive,steer,lift)
    Motor *getMotor(int w,int t){
        w--;
        return pair[getPairIdx(w)].getMotor(getWheelIdx(w),t);
    }
    
    /// get a pointer to a motor's data block by wheel and type
    /// @param n wheel number 1-6
    /// @param t type 0,1,2 (drive,steer,lift)
    MotorData *getMotorData(int w,int t){
        w--;
        return pair[getPairIdx(w)].getMotorData(getWheelIdx(w),t);
    }
    
        
    
    /// get a pointer to the monitoring data for a given drive motor
    /// @param n wheel number 1-6
    
    DriveMotorData *getDriveData(int n){
        n--;
        return pair[getPairIdx(n)].getDriveData(getWheelIdx(n));
    }
    
    /// get a pointer to the monitoring data for a given steer motor
    /// @param n wheel number 1-6
    
    SteerMotorData *getSteerData(int n){
        n--;
        return pair[getPairIdx(n)].getSteerData(getWheelIdx(n));
    }
    
    /// get a pointer to the monitoring data for a given lift motor
    /// @param n wheel number 1-6
    
    LiftMotorData *getLiftData(int n){
        n--;
        return pair[getPairIdx(n)].getLiftData(getWheelIdx(n));
    }
    /// get a pointer to a given drive motor
    /// @param n wheel number 1-6
    
    DriveMotor *getDrive(int n){
        n--;
        return pair[getPairIdx(n)].getDrive(getWheelIdx(n));
    }
    
    /// get a pointer to a given steer motor
    /// @param n wheel number 1-6
    
    SteerMotor *getSteer(int n){
        n--;
        return pair[getPairIdx(n)].getSteer(getWheelIdx(n));
    }
    
    /// get a pointer to a given lift motor
    /// @param n wheel number 1-6

    LiftMotor *getLift(int n){
        n--;
        return pair[getPairIdx(n)].getLift(getWheelIdx(n));
    }
    
    /// return a pointer to the master device's
    /// data, temperature monitoring etc.
    MasterData *getMasterData(){
        return masterData;
    }
    
    /// after initialisation, will send some default
    /// calibration data.
    void calibrate();
};

#endif /* __ROVER_H */
