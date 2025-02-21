/**
 * @file
 * Words for running the rover
 * 
 */
#include "angort.h"
#include "../pc/rover.h"
#include "roverexceptions.h"

extern Rover *r;
extern void emergencyStop();

%name control

%word s (--) emergency stop
{
    emergencyStop();
}

%word reset (--) reset exception states, overrides standard angort word!
{
    r->resetExceptions();
}

%word setlegchecks (bool --) enable/disable leg collision checks
{
    r->setLegCollisionChecks(a->popInt()?true:false);
}

%word exceptions (--) list all exceptions
{
    static const char * const names[]={"",
        "overcurrent","boot","remote","shortNC","stall","encoderfault","drivefault","shortOC"
    };
    
    r->update();
    for(int i=1;i<=6;i++){
        MotorData *d = r->getMotorData(i,DRIVE);
        int e = d->exceptionType;
        const char *n = names[e];
        
        printf("Drive %d : %d (%s)\n",i,e,n);
        d = r->getMotorData(i,STEER);
        printf("Steer %d : %d (%s)\n",i,e,n);
        d = r->getMotorData(i,LIFT);
        printf("Lift  %d : %d (%s)\n",i,e,n);
    }
    MasterData *m = r->getMasterData();
    printf("Master: type %d(%s), slave %d, motor %d\n",
           m->exceptionType,names[m->exceptionType],
           m->exceptionSlave,
           m->exceptionMotor);
}

%word calib (--) perform preset calibration
{
    r->calibrate();
}

%word temps (--) show all temperatures
{
    r->update();
    printf("Amb: %f\n",r->getMasterData()->temps[0]);
    for(int i=1;i<10;i++){
        printf("%d %f\n",i,r->getMasterData()->temps[i]);
    }
}
%word temp (index --) get a given sensor's temperature reading
{
    int i = a->popval()->toInt();
    Types::tFloat->set(a->pushval(),
            r->getMasterData()->temps[i]);
}
%word ambtemp (--) get the ambient temperature
{
    Types::tFloat->set(a->pushval(),
            r->getMasterData()->temps[0]);
}
%word rtemp (--) get a given (1-9) sensor's temperature above ambient
{
    int i = a->popval()->toInt();
    Types::tFloat->set(a->pushval(),
            r->getMasterData()->temps[i]-r->getMasterData()->temps[0]);
}    

%word dcurrent (wheel -- cur) get drive motor current
{
    MotorData *d = r->getMotorData(a->popval()->toInt(),DRIVE);
    Types::tFloat->set(a->pushval(),d->current);
}
%word lcurrent (wheel -- cur) get lift motor current
{
    MotorData *d = r->getMotorData(a->popval()->toInt(),LIFT);
    Types::tFloat->set(a->pushval(),d->current);
}
%word scurrent (wheel -- cur) get steer motor current
{
    MotorData *d = r->getMotorData(a->popval()->toInt(),STEER);
    Types::tFloat->set(a->pushval(),d->current);
}

%word update (--) update the rover sensor data (takes time)
{
    r->update();
}

void getactual(Runtime *a,int wheel,int type){
    MotorData *p = r->getMotorData(wheel,type);
    Types::tFloat->set(a->pushval(),p->actual);
}

%word dactual (wheel --) get actual drive speed
{
    getactual(a,a->popval()->toInt(),DRIVE);
}


%word lactual (wheel --) get actual lift position
{
    getactual(a,a->popval()->toInt(),LIFT);
}

%word sactual (wheel --) get actual steer position
{
    getactual(a,a->popval()->toInt(),STEER);
}

%word resetodo (wheel --) reset odometry for a wheel
{
    int wheel = a->popval()->toInt();
    DriveMotor *m = r->getDrive(wheel);
    m->resetOdometer();
}

%word odo (wheel --) get odometry for a wheel
{
    int wheel = a->popval()->toInt();
    DriveMotorData *p = r->getDriveData(wheel);
    Types::tFloat->set(a->pushval(),p->odometer);
}

