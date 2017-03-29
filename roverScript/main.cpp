/**
 * @file 
 *
 * 
 * @author $Author$
 * @date $Date$
 */

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

#include <signal.h>
#include <stdlib.h>

#include "../pc/rover.h"

#include "angort.h"
#include "udpclient.h"
#include "udpserver.h"
#include "roverexceptions.h"

extern LibraryDef LIBNAME(calib);
extern LibraryDef LIBNAME(control);
extern LibraryDef LIBNAME(util);
extern LibraryDef LIBNAME(udp);
extern LibraryDef LIBNAME(stdmath);

/// set to the current time when the program
/// starts

/// an instance of the Angort language
Angort ang;


/// mutex controlling access to Angort and the rover
pthread_mutex_t mutex;

/// a background thread, which does periodic rover updates,
/// sends monitoring data, and runs some optional Angort code
pthread_t thread;

/// delay time for monitoring, and also the update tick.
unsigned long updateTickLength = 10000L;


/// if I make these a subclasses Exception and throw them, C++
/// seems to only catch them as "Exception" - not their actual classes.
/// Odd.

class SigIntException{};
class SigQuitException{};

/// program start time
timespec progstart;

double time_diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    
    double t = temp.tv_sec;
    double ns = temp.tv_nsec;
    t += ns*1e-9;
    return t;
}

// these define which wheels are actually wired into the
// system.

#define MINWHEEL 1
#define MAXWHEEL 6

/// an instance of the rover control object
Rover *r;


/// this is the port the monitor sends
/// messages to
#define UDPSERVER_PORT 33333
/// this handles messages coming from the monitor program,
/// and processes them to change variables or perform actions.

class MyUDPServerListener: public UDPServerListener {
    /// messages arrive as key=value pairs.
    virtual void onKeyValuePair(const char *s,float v){
        extern void setUDPProperty(const char *name,float val);
        setUDPProperty(s,v);
    }
};

/// the instance of a UDP server, to handle
/// incoming UDP messages
UDPServer udpServer;


/// the update string is run periodically by Angort
/// inside the thread.

struct UpdateStringProperty : public Property{
    const char *val;
    char buf[256];
    
    UpdateStringProperty(){
        val = NULL;
    }
    virtual void postSet(){
        strncpy(buf,v.toString().get(),256);
        val = buf;
    }
    virtual void preGet(){
        Types::tString->set(&v,val?val:"?");
    }
    void clear(){
        val=NULL;
    }
};

/// single instance of the update string property
UpdateStringProperty updateString;


/// when true, update and send UDP packets regularly - activated
/// once the rover is fully up; and also poll
/// for incoming data on the UDP server.
bool autoUDP = false;

/// emergency routine for zeroing parameters (so all the gains will
/// zero) and forcing Angort to stop (by making all calls return until the
/// interpreter completes).

void emergencyStop(){
    ang.stop();
    if(!r->isValid())return;
    autoUDP=false;
    printf("EMERGENCY STOP\n");
    
    updateString.clear();
    usleep(180000L); // wait for any data
    
    
    int i,t;
    try {
        // these must run as much as possible, so I'll
        // catch the exceptions individually
        for(i=MINWHEEL;i<=MAXWHEEL;i++){
            for(t=0;t<3;t++){// each motor type
                // get and reset parameter block
                MotorParams *p = r->getMotor(i,t)->getParams();
                p->reset();
                // so we don't immediately overcurrent if the motor
                // is moving
                p->overCurrentThresh = 200;
                // send the modified block
                r->getMotor(i,t)->sendParams();
                r->getMotor(i,t)->setRequired(0);
            }
        } 
    }catch(SlaveException e){
        printf("error in resetting %s wheel: %s\n",
               r->getMotorTypeName(t), e.what());
    }
    autoUDP=true;
}

/// this checks signal states

int sigcheck(){
    sigset_t set;
    if(!sigpending(&set)){
        if(sigismember(&set,SIGINT) || sigismember(&set,SIGQUIT)){
            int n;
            if(!sigwait(&set,&n)){
                printf("Pending signal received: %d\n",n);
                return n;
            }
        }
    }
    return 0;
}

/// check signal states, and if an exception is pending, throw an exception.

void exceptonsig(){
    if(int sig = sigcheck()){
        if(sig == SIGINT)throw SigIntException();
        else if(sig == SIGQUIT)throw SigQuitException();
    }
}    

/// the signal handler used when we might be inside a loop
void roversighandler(int signum){
    printf("Signal %d received\n",signum);
    
    emergencyStop();
    updateString.clear();
}

/// when we are about to enter a loop or sleep - any operation which blocks
/// user input - we call this with true to set signal handlers and enable
/// those signals. Sometimes - for certain loops and user signals - we call
/// it with false to disable those signals. In this latter case, signals are
/// checked by exceptonsig() by hand, giving better control. 

void setsigs(bool allowInterrupt){
    signal(SIGINT,roversighandler);
    signal(SIGQUIT,roversighandler);
    
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    
    if(allowInterrupt)
        sigprocmask(SIG_UNBLOCK,&set,NULL);
    else
        sigprocmask(SIG_BLOCK,&set,NULL);
}



/// property encapsulating the required value of a motor

struct MotorReqProperty : public Property {
    Angort *a;
    int type;
    
    int wheel; // set in preSet()
    
    MotorReqProperty(Angort *_a,int t){
        a=_a;
        type =t;
    }
    virtual void postSet(){
        setsigs(false);
        r->getMotor(wheel,type)->setRequired(v.toFloat());
    }
    virtual void preSet(){
        wheel = a->popval()->toInt();
    }
    virtual void preGet(){
        wheel = a->popval()->toInt();
        Types::tFloat->set(&v,
                           r->getMotor(wheel,type)->getRequired());
    }
};

/// a property encapsulating the simulator's current factor

struct SimCurrentFactorProperty : public Property {
    Angort *a;
    SimCurrentFactorProperty(Angort *_a){
        a=_a;
    }
    virtual void postSet(){
        RoverSimulator::setSimCurrentFactor(Types::tFloat->get(&v));
    }
    virtual void preGet(){
        Types::tFloat->set(&v,RoverSimulator::getSimCurrentFactor());
    }
};


class MyStatusListener : public StatusListener {
public:
    /// called when a status update message occurs.
    virtual void onMessage(const char *str){
        printf("----%s\n",str);fflush(stdout);
    }
    
    
    /// called when the system's status changes; takes a new set
    /// of status flags (which are system dependent)
    virtual void onStatusChange(StatusFlag flags){}
};


double gettime(){
    timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    
    double t = ts.tv_nsec;
    t *= 1e-9;
    t += ts.tv_sec;
    return t;
}
void udpwrite(const char *s,...){
    va_list args;
    va_start(args,s);
    char buf[400];
    time_t t;
    
    
    sprintf(buf,"time=%f ",gettime());
    vsnprintf(buf+strlen(buf),400-strlen(buf),s,args);
    udpSend(buf);
    va_end(args);
}

/// send a standard block of UDP data, and
/// process any incoming messages
void handleUDP() {
    udpServer.poll(); // check for incoming
    
    /// send the special properties, whose
    /// values came from the monitor in
    /// the first place, for confirmation.
    extern void sendUDPProperties();
    sendUDPProperties();
    
    // get elapsed time
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,&t);
    double diff=time_diff(progstart,t);
    
    udpwrite("ptime=%f",diff);
    
    for(int w=1;w<=6;w++){
        DriveMotorData *d = r->getDriveData(w);
        DriveMotor *dm = r->getDrive(w);
        SteerMotorData *s = r->getSteerData(w);
        LiftMotorData *l = r->getLiftData(w);
        
        udpwrite("actual%d=%f req%d=%f current%d=%f lift%d=%f steer%d=%f liftcurrent%d=%f odo%d=%d",
                 w,d->actual,
                 w,dm->getRequired(),
                 w,d->current,
                 w,l->actual,
                 w,s->actual,
                 w,l->current,
                 w,d->odometer
                 );
    }
    
    MasterData *m = r->getMasterData();
    
    for(int i=1;i<10;i++){
        udpwrite("temp%d=%f",i,m->temps[i] - m->temps[0]);
    }
}

char threadRunning=1;
char threadDead=0;
// set up a thread used for periodic updates - this is 
// locked out during processing of user input. Also,
// UDP data is sent here and regular updates done if a given
// flag is set

void *updateThreadFunc(void *d){
    
    while(threadRunning){
        usleep(updateTickLength);
        if(updateString.val){
            pthread_mutex_lock(&mutex);
            ang.feed(updateString.val);
            fflush(stdout);
            pthread_mutex_unlock(&mutex);
        }
        if(autoUDP){
            pthread_mutex_lock(&mutex);
            r->update();
            handleUDP();
            pthread_mutex_unlock(&mutex);
        }
    }
    threadDead=1;
}
void initThreads(){
    pthread_mutex_init(&mutex,NULL);
    pthread_create(&thread,NULL,updateThreadFunc,NULL);
}

void showAngortError(){
    const Instruction *ip = ang.getIPException();
    if(ip){
        char buf[1024];
        ip->getDetails(buf,1024);
        printf("At: %s\n",buf);
    }
}

int main(int argc,char *argv[]){
    char buf[256];
    
    MyUDPServerListener udpServerListener;
    udpServer.setListener(&udpServerListener);
    
    ang.registerLibrary(&LIBNAME(calib),true);
    ang.registerLibrary(&LIBNAME(control),true);
    ang.registerLibrary(&LIBNAME(util),true);
    ang.registerLibrary(&LIBNAME(udp),true);
    ang.registerLibrary(&LIBNAME(stdmath),true);
    
    r = Rover::getInstance();
    
    r->comms.setStatusListener(new MyStatusListener);
    
    ang.registerProperty("drive",new MotorReqProperty(&ang,DRIVE));
    ang.registerProperty("steer",new MotorReqProperty(&ang,STEER));
    ang.registerProperty("lift",new MotorReqProperty(&ang,LIFT));
    ang.registerProperty("updatestring",&updateString);
    ang.registerProperty("simcurrentfactor",new SimCurrentFactorProperty(&ang));
    
    
    //    emergencyStop();
    
    exceptonsig();
    //    ang.printLines=true;
    try{
        ang.fileFeed("script.ang");
    }catch(Exception e) {
        emergencyStop();
        printf("Error in script: %s\n",e.what());
        showAngortError();
        exit(1);
    }
    exceptonsig();
    
    setsigs(false);
    
    bool sim = false;
    for(int ii=1;ii<argc;ii++){
        // should put proper opt parsing here..
        if(*argv[ii]=='-'){
            switch(argv[ii][1]){
            case 's':
                sim = true;
                break;
            case 'v':
                ang.printLines= true;
                break;
            case 'h':
                hostName = argv[ii]+2;
                break;
                
            default:break;
            }
        }else{
            try{
                ang.fileFeed(argv[ii]);
            }catch(Exception e) {
                emergencyStop();
                printf("Error in script: %s\n",e.what());
                showAngortError();
                exit(1);
            }
        }
    }
    
    try {
        // the integer is a bit mask for which boards are
        // present. 7 means all 3.
        int mask=0;
        for(int i=1;i<=6;i++){
            if(i>=MINWHEEL && i<=MAXWHEEL)
                mask |= (1<<((i-1)/2));
        }
        if(sim)
            r->init(NULL,mask); // NO PORT to run in simulation mode
        else
            r->init("/dev/ttyACM0",mask);
    } catch(SlaveException &e){
        printf("Error in init: %s\n",e.what());
    }
    
    initThreads();
    r->calibrate(); 
    clock_gettime(CLOCK_MONOTONIC,&progstart);
    
    udpServer.start(UDPSERVER_PORT);
    
    autoUDP=true;
    
    for(;;){
        char buf[256];
        sprintf(buf,"%d|%d %c ",
                GarbageCollected::getGlobalCount(),
                ang.stack.ct,
                ang.isDefining()?':':'>');
        setsigs(true);
#if READLINE
        char *line = readline(buf);
        setsigs(false);
        if(!line)break;
        
        if(*line)
            add_history(line);
#else
        char *line=buf;
        fputs("*\n",stdout);fflush(stdout);
        fgets(buf,256,stdin);
#endif
        try {
            pthread_mutex_lock(&mutex);
            ang.feed(line);
            exceptonsig();
        } catch(SigIntException &e){
            printf("interrupt signal\n");
            showAngortError();
        } catch(SigQuitException &e){
            emergencyStop();
            printf("quit signal\n");
            showAngortError();
        } catch(SlaveException &e){
            emergencyStop();
            printf("slave exception: %s\n",e.what());
            showAngortError();
        } catch(RoverException &e){
            emergencyStop();
            printf("rover exception: %s\n",e.what());
            showAngortError();
        } catch(Exception &e){
            emergencyStop();
            printf("exception: %s\n",e.what());
            showAngortError();
        }
        pthread_mutex_unlock(&mutex);
    }
    
    threadRunning=0;
    while(!threadDead)
        usleep(100);
    
    emergencyStop();
    rl_cleanup_after_signal();
    udpServer.stop();
}
