////////////////// -!- cmode; c++ -!- /////////////////////////

#include "config.h"
#include "speedmotor.h"
#include "posmotor.h"
#include "adc.h"
#include "chassis.h"

#include <avr/interrupt.h>
#include <avr/wdt.h>

#ifndef cbi
/// handy macro for clearing register bits (turning off pullups)
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

/// the global system state - exceptions etc.
State state;

/// ADC reader object
ADCReader adc;

/// address of I2C device, loaded from eeprom
char i2c_addr=0;

/// LED counters for REG_DEBUGLED
char ledCounts[2];

/// last time we got a message from the master
volatile unsigned long lastmsgtime=0;

/// add LED listener for exceptions - one light for overcurrents,
/// identifying the motor; two lights for other exceptions.

class LEDListener : public ExceptionListener {
public:
    char bits;
    LEDListener(){
        bits = 0;
    }
    
    virtual void onException(MotorController *m,char type){
        if(m)bits |= (1<<m->getID());
        else bits=3;
    }
    virtual void onExceptionReset(){
        bits=0;
    }
};

LEDListener ledListener;


#if DRIVESTEER
/// instance of the chassis listener
Chassis chassis;
/// the two motor controllers run by this unit
SpeedMotorController driveMotor;
PositionMotorController steerMotor;
MotorController *motors[] = {&driveMotor,&steerMotor};
#else
PositionMotorController lift1,lift2;
MotorController *motors[] = {&lift1,&lift2};
#endif

#if DRIVESTEER
// the encoder is on pins PD0 and PD1 (arduino numbers 0-1, RX/TX).
// We set up an interrupt to be triggered on PD0 changing.

void setupEncoderISR(){
    cli();
    PCMSK2 |= 1<<PCINT16; // PD-0.
    PCICR |= 1<<PCIE2; // turn on ints for port D
    sei();
}
ISR(PCINT2_vect){
    if(PIND & 1) // rising edge on pin 0
        driveMotor.tickEncoder();
}
#endif


/**
 * Divides a given PWM pin frequency by a divisor.
 *
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 *
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 *
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 *
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

// show a sequence of lights on reset
void startupLights(){
    digitalWrite(M1LED,LOW);
    digitalWrite(M2LED,LOW);
    /*
    // flash back/forth
    for(int i=0;i<5;i++){
        wdt_reset();
        delay(300);
        digitalWrite(M1LED,HIGH);
        digitalWrite(M2LED,LOW);
        wdt_reset();
        delay(300);
        digitalWrite(M1LED,LOW);
        digitalWrite(M2LED,HIGH);
    }
    // then both on
    wdt_reset();
    digitalWrite(M1LED,HIGH);
    digitalWrite(M2LED,HIGH);
    delay(300);
       // then show addr
    */
    wdt_reset();
    digitalWrite(M1LED, (i2c_addr & 1)?HIGH:LOW);
    digitalWrite(M2LED, (i2c_addr & 2)?HIGH:LOW);
    delay(300);
    wdt_reset();
    delay(300);
    // then clear
    wdt_reset();
    digitalWrite(M1LED,LOW);
    digitalWrite(M2LED,LOW);
}

void setup()
{
    cbi(PORTC,4); // disable internal pullups for I2C
    cbi(PORTC,5);
    
    cbi(PORTD,0); // disable internal pullups for encoder
    cbi(PORTD,1);
    
    // if we reset through abusing the watchdog, we need to
    // turn the watchdog off again immediately!
    
    MCUSR=0;
    wdt_enable(WDTO_1S);
    
#if DRIVESTEER
    pinMode(ENCA,INPUT);
    pinMode(ENCB,INPUT);
#endif
    
    pinMode(M1LED,OUTPUT);
    pinMode(M2LED,OUTPUT);
    pinMode(M1POS,OUTPUT);
    pinMode(M1NEG,OUTPUT);
    pinMode(M2POS,OUTPUT);
    pinMode(M2NEG,OUTPUT);
    pinMode(M1PWM,OUTPUT);
    pinMode(M2PWM,OUTPUT);
    
    extern void readEEProm();
    readEEProm(); // read parameters. Will halt with an error flash when bad!
    
    startupLights();
    
    // set PWM to 31kHz (divisor=1)
    
    setPwmFrequency(M1PWM,1); 
    setPwmFrequency(M2PWM,1);
    
#if DRIVESTEER
    // initialise the motor controllers
    driveMotor.init(0,M1PWM,M1POS,M1NEG,ENCA,ENCB);
    steerMotor.init(1,M2PWM,M2POS,M2NEG);
    // initialise ADC and add appropriate reads:
    // adc0 and 1 are current, adc 6 is steer motor position
    // 7 is chassis.
    adc.addRead(0,TYPE_CURRENT,&driveMotor);
    adc.addRead(1,TYPE_CURRENT,&steerMotor);
    adc.addRead(6,TYPE_POSITION,&steerMotor);
    adc.addRead(7,TYPE_POSITION,&chassis);
#else
    lift1.init(0,M1PWM,M1POS,M1NEG);
    lift2.init(1,M2PWM,M2POS,M2NEG);
    adc.addRead(0,TYPE_CURRENT,&lift1);
    adc.addRead(1,TYPE_CURRENT,&lift2);
    adc.addRead(6,TYPE_POSITION,&lift1);
    adc.addRead(7,TYPE_POSITION,&lift2);
#endif
    state.addExceptionListener(&ledListener);
    
    
    // set up interrupts and ADC
    cli(); // interrupts off
    adc.init();
#if DRIVESTEER
    setupEncoderISR();
#endif
    sei(); // interrupts on
    
    
    // setup I2C
    extern void initI2C(int);
    initI2C(i2c_addr);
}

unsigned long maxt=0;

void loop()
{
    extern void I2CLoop();
    I2CLoop();
    
    adc.poll();
/*    
    unsigned long t = millis();
    if(t>lastmsgtime && (t-lastmsgtime)>5000){
        for(;;){ // reset using WDT
            delay(10);
            digitalWrite(M1LED,HIGH);
            delay(10);
            digitalWrite(M1LED,LOW);
        }
    }
*/    
    // now handle the leds
    
    
    // internal heartbeat
    static int qqq=0;
    if(qqq++==100){
        ledCounts[1]=1;
        qqq=0;
    }
    
    for(int i=0;i<2;i++){
        int out;
        if(ledCounts[i] || (ledListener.bits & (1<<i)))
            out=HIGH;
        else
            out=LOW;
        if(ledCounts[i])ledCounts[i]--;
        digitalWrite(i==0?M1LED:M2LED,out);
    }
    
}
