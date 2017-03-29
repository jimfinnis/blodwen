/**
 * \file 
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __ENCODER_H
#define __ENCODER_H

#include <stdint.h>

/// Quad encoder class which measures the speed
/// of a two-channel encoder;
/// assumes the encoder is in PD0-PD1 - if you change this,
/// you need to modify the ISR in sketch.ino

class QuadEncoder {
    
    /// value calculated in update()
    float tickFreq;
    
    volatile uint32_t negativeCt;
    volatile uint32_t ct;
    uint32_t prevTime; 
    uint32_t odoTicks;
    
public:    
    
    QuadEncoder(){
        negativeCt=0;
        ct=0;
        prevTime=0;
        odoTicks=0;
    }        
    
    
    // update the encoder. Every now and then, see how many ticks have
    // gone by. Returns true if a new value was calculated.
    
    bool update(){
        uint32_t now = micros();
        if(now<prevTime){
            // if time ran backwards (because of integer wraparound)
            // then skip a go
            prevTime = now;
            return false;
        }
        uint32_t t = now-prevTime;
        if(t>150000){ // update every N microseconds
            prevTime = now; // update previous time
            uint32_t ict = ct;   // atomic(ish) read of ct
            float count = ict; // convert to float
            tickFreq = (count*1e6)/(float)t; // and get tick frequency
            // negate frequency if we're going in reverse
            if(negativeCt>ct/3) 
                tickFreq = -tickFreq;
            ct=0;
            negativeCt=0;
            return true;
        } else
            return false; // no update occurred
    }
    
    
    /// called from interrupt on rising edge
    void tick(){
        ct++;
        odoTicks++;
        //check sign of other pin to get direction. If it's high,
        //we're going backwards.
        if(PIND&2)
            negativeCt++;
    }
    
    /// return the tick frequency
    float getTickFreq(){
        return tickFreq;
    }
    
    void resetOdometry(){
        odoTicks=0;
    }
    uint32_t getOdometry(){
        return odoTicks;
    }
          
};


#endif /* __ENCODER_H */
