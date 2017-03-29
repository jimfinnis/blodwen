/**
 * \file
 * Most of the rover code is in the .h files; this file contains those few
 * methods which would introduce cyclic dependencies were they in
 * the includes. It also includes the statics.
 * 
 */

#include "rover.h"

Rover *Rover::instance = NULL;

uint8_t SlaveDevice::readSet[READSETS][READSETSIZE];
uint8_t SlaveDevice::readSetCt[READSETS];


inline bool isPositive(float angle){
    return angle>5;
}

inline bool isNegative(float angle){
    return angle<-5;
}

inline bool isLiftRequestedPositive(int wheel){
    if(wheel<0)return false;
    Rover *r = Rover::getInstance();
    LiftMotor *m = r->getLift(wheel);
    return isPositive(m->getRequired());
}
inline bool isLiftRequestedNegative(int wheel){
    if(wheel<0)return false;
    Rover *r = Rover::getInstance();
    LiftMotor *m = r->getLift(wheel);
    return isNegative(m->getRequired());
}


bool LiftMotor::isAdjacencyViolated(float req){
    if(!Rover::getInstance()->legCollisionChecksEnabled)
	return false;

    // first, find the two adjacent wheels (or perhaps just one)
    int forw=-1,back=-1;
    switch(wheelNumber){
        case 1:forw=3;break;
        case 2:forw=4;break;
        case 3:forw=5;back=1;break;
        case 4:forw=6;back=2;break;
        case 5:back=3;break;
        case 6:back=4;break;
    }
    
    //positive angles tilt the wheel towards the front
    
    if(isPositive(req) && isLiftRequestedNegative(forw))
        return true;
    else if(isNegative(req) && isLiftRequestedPositive(back))
        return true;
    else
        return false;
}


void Rover::calibrate(){
    
    float liftc[][2]={
        {-109,109},
        {-102,112},
        {-110,105},
        {-101,110},
        {-113,113}, // was -107,113
        {-109,103}
    };
    float steerc[][2]={
        {-56,66},
        {-62,59},
        {-59,64},
        {-60,65},
        {-66,57},
        {-62,60},
    };
    
    // calibrate the wheels and set parameters
    for(int i=1;i<=6;i++){
        
        // drive wheels
        
        DriveMotor *d = getDrive(i);
        MotorParams *dp = d->getParams();
        dp->pGain = 0.01f;
        dp->iGain = 0;
        dp->dGain = 0;
        dp->iCap = 0;
        dp->iDecay = 0;
        dp->overCurrentThresh = 500;
        d->sendParams();
        
        // steer wheels
    
        // send calibration changes first, THEN parameters
        SteerMotor *s = getSteer(i);
        PosMotorParams *p = s->getPosParams();
        p->calibMin = steerc[i-1][0];
        p->calibMax = steerc[i-1][1];
        p->iCap=0;
        p->iDecay=0;
        s->sendParams();
        usleep(50000); // delay to set things settle
        p->pGain = 0;
        p->iGain = 2;
        p->dGain = 0;
        p->iCap = 300;
        p->iDecay = 0.95f;
        p->overCurrentThresh = 500;
        p->calibMin = steerc[i-1][0];
        p->calibMax = steerc[i-1][1];
	p->deadZone = 0.5f;
        s->sendParams();
        
        // lift wheels
        
        LiftMotor *l = getLift(i);
        p = l->getPosParams();
        p->calibMin = liftc[i-1][0];
        p->calibMax = liftc[i-1][1];
        p->iCap=0;
        p->iDecay=0;
        l->sendParams();
        usleep(50000); // delay to set things settle
        p->pGain = 0;
        p->iGain = 5;
        p->dGain = 0;
        p->iCap = 50;
        p->iDecay = 0.9f;
        p->overCurrentThresh = 500;
        l->sendParams();
    }
}
