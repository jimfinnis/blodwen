/**
 * \file
 * Code for the rover simulator, which simulates at the serial comms
 * level for development without a rover.
 *
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "rover.h"

#include "motorsim.h"

float RoverSimulator::simCurrentFactor=0.07;

// motor simulation smoothing factors, which describe how required becomes
// actual. They're different for each motor to ensure that motors take
// different times to perform different activities.

float driveSmoothing[] = {0.11f,0.09f,0.10f,
                        0.12f,0.112f,0.08f};
float liftSmoothing[] = {0.09f,0.11f,0.12f,
                        0.08f,0.12f,0.12f};
float steerSmoothing[] = {0.11f,0.09f,0.10f,
                        0.12f,0.112f,0.08f};
    



/// a cyclic buffer of bytes - we use this class to simulate
/// incoming and outgoing data.

class CyclicBuf {
    uint8_t *buf;
    int writect;
    int readct;
    int capacity;
public:
    CyclicBuf(int cap){
        capacity=cap;
        buf = (uint8_t *)malloc(cap);
        readct=writect=0;
    }
    ~CyclicBuf(){
        free(buf);
    }
    
    bool hasData(){
        return readct!=writect;
    }
    
    char read(){
        char c;
        if(readct!=writect){
            c = buf[readct++];
            readct %= capacity;
        }else{
            c=0;
        }
        return c;
    }
    
    void write(char c){
        buf[writect++]=c;
        writect%=capacity;
        if(writect==readct){
            printf("out of space in simulator buffer write\n");
            exit(1);
        }
    }
    void write(char *d,int ct){
        for(int i=0;i<ct;i++)
            write(*d++);
    }
};

static CyclicBuf in(1024); // system -> simulator
static CyclicBuf out(1024); // simulator -> system


int RoverSimulator::read(char *buf,int ct){
    // we're reading from the fake rover - this involves
    // copying out the out buffer
    
    int read=0;
    while(out.hasData() && read<ct){
        buf[read++] = out.read();
    }
    return read;
}

int RoverSimulator::write(const char *s,int ct){
    for(int i=0;i<ct;i++)
        in.write(s[i]);
    
    // process the simulator
    //    update();
    poll();
    return 0; // 0=OK
}




/////////// the actual simulator - much of this code is copied and modified
/////////// from code in the master firmware.


static int readSets[16][32]; // should be loads of space
static int readSetCts[16];
static uint16_t regs[16][64]; // register values! Eek!

static timespec lastTime;

static inline double time_diff(timespec start, timespec end)
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

const Register *getReg(int dev,int regN){
    const Register *r;
    switch(dev){
    case 0:
        r = registerTable_MASTER;
        break;
    case 3:
    case 6:
    case 9:
        r = registerTable_LL;
        break;
    default:
        r = registerTable_DS;
        break;
    }
    return r+regN;
}

inline void setr(int d,int n,float v){
    const Register *r = getReg(d,n);
    regs[d][n] = r->map(v);
}

inline float getr(int d,int n){
    const Register *r = getReg(d,n);
    return r->unmap(regs[d][n]);
}



static double odo[10];//fake odometry readings; 10 of them even though it wastes space, it simplifies the code.

RoverSimulator::RoverSimulator(){
    timeSoFar = 0;
    
    clock_gettime(CLOCK_MONOTONIC,&lastTime);
    // default is zero for everything
    memset(regs,0,16*64*sizeof(uint8_t));
    
    /// initialise the motors
    for(int i=0;i<6;i++){
        drive[i] = new MotorSim(driveSmoothing[i]);
        lift[i] = new MotorSim(liftSmoothing[i]);
        steer[i] = new MotorSim(steerSmoothing[i]);
    }
    
    // set the defaults
    int i;
    for(int d=0;d<10;d++){
        odo[d]=0; // reset odometry
        switch(d){
        case 0://master
            setr(d,REGMASTER_TEMPAMBIENT,13);
            for(i=0;i<=8;i++)
                setr(d,REGMASTER_TEMP1+i,15); // default temps
            break;
        case 1://drive steer
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:
            setr(d,REGDS_DRIVE_REQSPEED,0);
            setr(d,REGDS_DRIVE_ERROR,0);
            setr(d,REGDS_DRIVE_DGAIN,0);
            setr(d,REGDS_DRIVE_ACTUALSPEED,0);
            setr(d,REGDS_DRIVE_ERRORDERIV,0);
            setr(d,REGDS_DRIVE_ERROR,0);
            setr(d,REGDS_DRIVE_CONTROL,0);
            setr(d,REGDS_STEER_REQPOS,0);
            setr(d,REGDS_STEER_ERROR,0);
            setr(d,REGDS_STEER_DGAIN,0);
            setr(d,REGDS_STEER_ACTUALPOS,0);
            setr(d,REGDS_STEER_ERRORDERIV,0);
            setr(d,REGDS_STEER_ERROR,0);
            setr(d,REGDS_STEER_CONTROL,0);
            setr(d,REGDS_STEER_CALIBMIN,-60);
            setr(d,REGDS_STEER_CALIBMAX,60);
            break;
        case 3:
        case 6:
        case 9:
            setr(d,REGLL_ONE_REQPOS,0);
            setr(d,REGLL_ONE_ERROR,0);
            setr(d,REGLL_ONE_DGAIN,0);
            setr(d,REGLL_ONE_ACTUALPOS,0);
            setr(d,REGLL_ONE_ERRORDERIV,0);
            setr(d,REGLL_ONE_ERROR,0);
            setr(d,REGLL_ONE_CONTROL,0);
            setr(d,REGLL_ONE_CALIBMIN,-60);
            setr(d,REGLL_ONE_CALIBMAX,60);
            setr(d,REGLL_TWO_REQPOS,0);
            setr(d,REGLL_TWO_ERROR,0);
            setr(d,REGLL_ONE_DGAIN,0);
            setr(d,REGLL_TWO_ACTUALPOS,0);
            setr(d,REGLL_TWO_ERRORDERIV,0);
            setr(d,REGLL_TWO_ERROR,0);
            setr(d,REGLL_TWO_CONTROL,0);
            setr(d,REGLL_TWO_CALIBMIN,-60);
            setr(d,REGLL_TWO_CALIBMAX,60);
        }
    }
}

RoverSimulator::~RoverSimulator(){
    for(int i=0;i<6;i++){
        delete drive[i];
        delete steer[i];
        delete lift[i];
    }
}


static void doread(int id,uint8_t *p){
    uint8_t buf[128];
    int ct=0;
    
    int set = *p++; // get the read set index
        
    for(int i=0;i<readSetCts[set];i++){ // for each read
        uint16_t v;
        int r = readSets[set][i];
        const Register *reg = getReg(id,r);
        v = regs[id][r]; // get value
        buf[ct++]=v & 0xff; // store the bottom byte in the buffer
        if(reg->getSize()==2) // if the value is 16-bit
            buf[ct++]=v>>8; // store the top byte
    }
    out.write((char *)buf,ct); // write the buffer
    
}
static void dowrite(int id,uint8_t *p){
    
    int writes = *p++;
    for(int i=0;i<writes;i++){
        int r = *p++;
        uint16_t v = *p++;
        if(getReg(id,r)->getSize() == 2){
            v |= *p++ << 8;
        }
        regs[id][r]=v;
        
        // put special cases down here
        if(r == REG_RESET){
            if(v & RESET_ODO)
                odo[id]=0;
        }
        
    }
    char qqq=0;
    out.write(&qqq,1);
}
static void doreadset(uint8_t *p,int ct){
    int set = *p++;
    ct--;
    readSetCts[set]=ct;
    for(int i=0;i<ct;i++){
        readSets[set][i]=*p++;
    }
    out.write(ct);
}


static void processCmd(int ct,uint8_t *p){
    int id = *p>>4; // address/id: 0 for master, 1-9 for slaves
    switch(*p++&0xf){// get command and increment ptr
    case 2: // write command
        dowrite(id,p);
        break;
    case 3: // read command
        doread(id,p);
        break;
    case 4:// readset command
        doreadset(p,ct);
        break;
    default:
        printf("Unknown command in simulator\n");
        exit(1);
        
    }
}

static const char wheelToDevice_ds[]={1,2,4,5,7,8};
static const char wheelToDevice_ll[]={3,3,6,6,9,9};

void RoverSimulator::simMotors(double t){
    
    for(int i=0;i<6;i++){
        drive[i]->required = getr(wheelToDevice_ds[i],REGDS_DRIVE_REQSPEED);
        steer[i]->required = getr(wheelToDevice_ds[i],REGDS_STEER_REQPOS);
        
        lift[i]->required = getr(wheelToDevice_ll[i],
                                 (i%2)?REGLL_TWO_REQPOS:REGLL_ONE_REQPOS);
        
        drive[i]->update();
        lift[i]->update();
        steer[i]->update();
        
        setr(wheelToDevice_ds[i],REGDS_DRIVE_ACTUALSPEED,drive[i]->getActual());
        setr(wheelToDevice_ds[i],REGDS_DRIVE_CURRENT,drive[i]->getSimCurrent());
        setr(wheelToDevice_ds[i],REGDS_STEER_ACTUALPOS,steer[i]->getActual());
        
        setr(wheelToDevice_ll[i],(i%2)?REGLL_TWO_ACTUALPOS:REGLL_ONE_ACTUALPOS,
             lift[i]->getActual());
        
        setr(wheelToDevice_ll[i],(i%2)?REGLL_TWO_CURRENT:REGLL_ONE_CURRENT,0);
        
    }
}


void RoverSimulator::simulate(double t){
    
    // add the time to the time accumulator, and if it's over,
    // run a simulator tick.
    
    timeSoFar += t;
    if(timeSoFar>SIMTICKLENGTH){
        timeSoFar-=SIMTICKLENGTH;
        simMotors(t);
    
        // get values of temperature - we're not using it right now.
        setr(0,REGMASTER_TEMP1,15);
        setr(0,REGMASTER_TEMP2,15);
        setr(0,REGMASTER_TEMP4,15);
        setr(0,REGMASTER_TEMP5,15);
        setr(0,REGMASTER_TEMP7,15);
        setr(0,REGMASTER_TEMP8,15);
    
        // odometry
        for(int i=0;i<6;i++){
            odo[i]+=drive[i]->getActual();
            setr(wheelToDevice_ds[i],REGDS_DRIVE_ODO,odo[i]);
        }
    }
}

                       
                       
static uint8_t buf[256]; // command buffer
static int ct; // command buffer count

void RoverSimulator::update(){
    struct timespec a;
        
    clock_gettime(CLOCK_MONOTONIC,&a);
    double t = time_diff(lastTime,a);
    lastTime=a;
    simulate(t);
    usleep(5000);
}

void RoverSimulator::poll(){
    while(in.hasData()){
        buf[ct++] = in.read();
        if(buf[0]==ct){
            processCmd(ct-2,buf+1);
            ct=0;
        }
    }
        
}
