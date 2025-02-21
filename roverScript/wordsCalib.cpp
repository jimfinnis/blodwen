/**
 * @file
 * Words for modifying
 * rover settings and calibrations
 * 
 */
#include "angort.h"
#include "../pc/rover.h"
#include "roverexceptions.h"

extern void setsigs(bool allowInterrupt);

extern Rover *r;

void setpgain(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->pGain = v;
        m->sendParams();
    }else throw RoverInvalidException();
}

void setigain(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->iGain = v;
        m->sendParams();
    }else throw RoverInvalidException();
}

void setdgain(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->dGain = v;
        m->sendParams();
    }else throw RoverInvalidException();
}
void seticap(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->iCap = v;
        m->sendParams();
    }else throw RoverInvalidException();
}
void setidecay(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->iDecay = v;
        m->sendParams();
    }else throw RoverInvalidException();
}
void setocthresh(Runtime *a,int t){
    int w = a->popval()->toInt();
    float v = a->popval()->toFloat();
    if(r->isValid()){
        Motor *m = r->getMotor(w,t);
        MotorParams *p = m->getParams();
        p->overCurrentThresh = v;
        m->sendParams();
    }else throw RoverInvalidException();
}
void showparams(Runtime *a,int t){
    int w = a->popval()->toInt();
    if(r->isValid()){
        Motor *d = r->getMotor(w,t);
        MotorParams *p = d->getParams();
        printf("  P,I,D : %f %f %f\n",p->pGain,p->iGain,p->dGain);
        printf("  icap=%f, idecay=%f\n",p->iCap,p->iDecay);
        printf("  overcurrent: %f Stalllevel: %f\n",p->overCurrentThresh,p->stallCheck);
        if(t == LIFT || t == STEER){
            PosMotorParams *pp = (PosMotorParams *)p;
            printf("  calibration range: %f to %f\n",pp->calibMin,pp->calibMax);
        }
    }else throw RoverInvalidException();
}
void setallparams(Runtime *a,int t){
    int w = a->popval()->toInt();
    if(r->isValid()){
        setsigs(false);
        Motor *d = r->getMotor(w,t);
        MotorParams *p = d->getParams();
        p->deadZone = a->popval()->toFloat();
        p->stallCheck = a->popval()->toFloat();
        p->overCurrentThresh = a->popval()->toFloat();
        p->iDecay = a->popval()->toFloat();
        p->iCap = a->popval()->toFloat();
        p->dGain = a->popval()->toFloat();
        p->iGain = a->popval()->toFloat();
        p->pGain = a->popval()->toFloat();
        d->sendParams();
    }else throw RoverInvalidException();
}

%name calib


/*
 * 
 * Words for setting parameters of drive motors
 *
 */

%word dpgain (val wheel --) set drive p-gain
{
    setpgain(a,DRIVE);
}
%word ddgain (val wheel --) set drive d-gain
{
    setdgain(a,DRIVE);
}
%word digain (val wheel --) set drive i-gain
{
    setigain(a,DRIVE);
}
%word dicap (val wheel --) set drive i-cap
{
    seticap(a,DRIVE);
}
%word didecay (val wheel --) set drive i-decay
{
    setidecay(a,DRIVE);
}
%word doverth (val wheel --) set drive overcurrent threshold
{
    setocthresh(a,DRIVE);
}
%word dshow (wheel --) show drive parameters
{
    showparams(a,DRIVE);
}
%word setdriveparams (p i d cap decay octhresh stallcheck deadzone wheel --) set all parameters for a drive motor
{
    setallparams(a,DRIVE);
}


/*
 * 
 * Words for setting parameters of steer motors
 *
 */

%word spgain (val wheel --) set steer p-gain
{
    setpgain(a,STEER);
}
%word sdgain (val wheel --) set steer d-gain
{
    setdgain(a,STEER);
}
%word sigain (val wheel --) set steer i-gain
{
    setigain(a,STEER);
}
%word sicap (val wheel --) set steer i-cap
{
    seticap(a,STEER);
}
%word sidecay (val wheel --) set steer i-decay
{
    setidecay(a,STEER);
}
%word soverth (val wheel --) set steer overcurrent threshold
{
    setocthresh(a,STEER);
}
%word sshow (wheel --) show steer parameters
{
    showparams(a,STEER);
}
%word setsteerparams (p i d cap decay octhresh stallcheck deadzone wheel --) set all parameters for a steer motor
{
    setallparams(a,STEER);
}
%word calibsteer (min max wheel --) calibrate steer motor
{
    int w = a->popval()->toInt();
    float mx = a->popval()->toFloat();
    float mn = a->popval()->toFloat();
    if(r->isValid()){
        SteerMotor *m = r->getSteer(w);
        PosMotorParams *p = m->getPosParams();
        p->calibMin = mn;
        p->calibMax = mx;
        m->sendParams();
    } else throw RoverInvalidException();
    
}




/*
 * 
 * Words for setting parameters of lift motors
 *
 */

%word lpgain (val wheel --) set lift p-gain
{
    setpgain(a,LIFT);
}
%word ldgain (val wheel --) set lift d-gain
{
    setdgain(a,LIFT);
}
%word ligain (val wheel --) set lift i-gain
{
    setigain(a,LIFT);
}
%word licap (val wheel --) set lift i-cap
{
    seticap(a,LIFT);
}
%word lidecay (val wheel --) set lift i-decay
{
    setidecay(a,LIFT);
}
%word loverth (val wheel --) set lift overcurrent threshold
{
    setocthresh(a,LIFT);
}
%word lshow (wheel --) show lift parameters
{
    showparams(a,LIFT);
}
%word setliftparams (p i d cap decay octhresh stallcheck deadzone wheel --) set all parameters for a lift motor
{
    setallparams(a,LIFT);
}
%word caliblift (min max wheel --) calibrate lift motor
{
    int w = a->popval()->toInt();
    float mx = a->popval()->toFloat();
    float mn = a->popval()->toFloat();
    if(r->isValid()){
        LiftMotor *m = r->getLift(w);
        PosMotorParams *p = m->getPosParams();
        p->calibMin = mn;
        p->calibMax = mx;
        m->sendParams();
    } else throw RoverInvalidException();
    
}

