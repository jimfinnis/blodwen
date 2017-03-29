/**
 * \file Serial port implementation of slave interface.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "../../common/serialbuffer.h"
#include "motor.h"

#include <avr/pgmspace.h>

// reference the motors, defined in sketch.ino

extern MotorController motors[2];

// strings holding usage information, kept in program space
const char PROGMEM hlp0[] ="0s   : stop motor 0 on -ve and set to max duty cycle";
const char PROGMEM hlp1[] ="0S   : stop motor 0 on +ve and set to max duty cycle";
const char PROGMEM hlp2[] ="0i   : stop motor on -ve and set to zero duty cycle";
const char PROGMEM hlp3[] ="0f10 : motor 0, forward speed 10";
const char PROGMEM hlp4[] ="0r10 : motor 0, reverse speed 10";
const char PROGMEM hlp7[] ="0o   : show motor 0 overcurrent threshold";
const char PROGMEM hlp8[] ="0O100: set motor 0 overcurrent threshold to 100";
const char PROGMEM hlp9[] ="0R  : reset motor 0 overcurrent";
const char PROGMEM hlp10[] ="0c  : show motor 0 current";
const char PROGMEM hlp11[] ="0Hu0.001: set motor 0 rising current monitor hysteresis";
const char PROGMEM hlp12[] ="0Hd0.001: set motor 0 falling current monitor hysteresis";
const char PROGMEM hlp13[] ="0Hb0.001: set motor 0 both current monitor hystereses";
const char PROGMEM hlp14[] ="0H? : print current hystereses for motor 0";

/// table of usage information string ptrs, kept in program space
const char PROGMEM * const PROGMEM usage_table[] ={
    hlp0,hlp1,hlp2,hlp3,    hlp4,hlp7,
    hlp8,hlp9,hlp10,hlp11,  hlp12,hlp13,hlp14
};

extern volatile int adc_isr_ct; 

/// show the usage
static void showUsage(){
    char buf[80];
    for(int i=0;i<=14;i++){
        delay(20);
        const char PROGMEM *p = (const char PROGMEM *) 
              pgm_read_word(&(usage_table[i]));
        strcpy_P(buf,p);
        Serial.println(buf);
        delay(20);
    }
}

/// an implementation of the serial listener, which responds to
/// commands coming in on the serial port.
class MySerialListener : public SerialListener {
    virtual void process(SerialBuffer *src,const char *s){
        
        int t;
        
        int motorid = *s - '0';
        if(motorid<0 || motorid>1)
            src->error(100,"motor ID out of range");
        else {
            MotorController *m = motors+motorid;
            s++;
            switch(*s){
            case 's': // stop LOW and set to max duty cycle
                m->stopLow();
                m->setSpeed(MotorController::MAXDUTY);
                src->success("motor stop low");
                break;
            case 'S': // stop HIGH and set to max duty cycle
                m->stopHigh();
                m->setSpeed(MotorController::MAXDUTY);
                src->success("motor stop high");
                break;
            case 'i': // stop LOW and set to zero duty cycle
                m->stopLow();
                m->setSpeed(0);
                src->success("motor idle");
                break;
            case 'f': // set forward with given speed (denary)
                s++;
                t=atoi(s);
                m->forward();
                if(m->setSpeed(t))
                    src->success("motor forward %d",t);
                else
                    src->error(101,"overcurrent");
                break;
            case 'r': // set forward with given speed (denary)
                s++;
                t=atoi(s);
                m->reverse();
                if(m->setSpeed(t))
                    src->success("motor reverse %d",t);
                else
                    src->error(101,"overcurrent");
                break;
            case 'R': // reset an overcurrent motor
                if(m->isOverCurrent()){
                    m->reset();
                    src->success();
                } else {
                    src->error(102,"motor does not need reset");
                }
                break;
            case 'o': // show the overcurrent threshold for this motor
                src->success("%d",m->overCurrentThreshold);
                break;
            case 'O': // set the overcurrent threshold for this motor
                s++;
                t=atoi(s);
                m->overCurrentThreshold = t;
                src->success("threshold %d",t);
                break;
            case 'c': // show the current on this motor
                src->success("%d",(int)(100.0f*m->getCurrent()));
                break;
            case 'h': // show commands
                showUsage();
                break;
            case 'H': // hysteresis up or down, e.g. Hu0.001 or Hd0.001 or Hb0.001 for both
                {
                    s++;
                    char c = *s++;
                    float v = atof(s);
                    switch(c){
                    case 'u':
                        m->hysterUp = v;
                        src->success();
                        break;
                    case 'd':
                        m->hysterDown = v;
                        src->success();
                        break;
                    case 'b':
                        m->hysterDown = v;
                        m->hysterUp = v;
                        src->success();
                        break;
                    case '?':
                        Serial.print("u ");Serial.println(m->hysterUp,6);
                        Serial.print("d ");Serial.println(m->hysterDown,6);
                        break;
                    default:
                        src->error(103,"hysteresis command unknown");
                        break;
                    }
                }
                break;
            case 't':// print ASR count
                src->success("%d",adc_isr_ct);
                break;
            default:
                src->error(999,"unknown command");
            }
        }
    }
};

/// the serial listener object, which responds to serial commands
static MySerialListener listener;

/// a listener for printing overcurrent messages to serial

class SerialOvercurrentListener : public MotorListener {
public:
    
    /// on an overcurrent print a message (may be removed in final code)
    /// and activate the appropriate LED
    
    virtual void overCurrentEvent(MotorController *src,int current){
        Serial.print("overcurrent in motor ");
        Serial.println(src->getID()+1);
    }
};

static SerialOvercurrentListener overcurrentListener;

/// a serial buffer which picks up characters from the serial interface
/// when polled and adds them to the internal buffer; when a line is
/// complete it sends it to a listener.
static SerialBuffer b(&listener);

void initSerialComms(){
    // start serial comms
    Serial.begin(9600);
    
    delay(200);
    Serial.println("Starting..");
    
    motors[0].addMotorListener(&overcurrentListener);
}    

void serialCommsLoop(){
    b.poll();
}
