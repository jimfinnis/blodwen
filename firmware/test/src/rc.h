/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __RC_H
#define __RC_H

/// RC receiver - uses PCINT0.

class RCReceiver {
    struct {
        float mn,mx;
        float factor;
    } calib[3];
    int s[3];
    bool rdy;
    
public:
    void init();
    void update();
    
    RCReceiver &setCalib(int n,float mn,float mx){
        calib[n].mx =mx;
        calib[n].mn =mn;
        calib[n].factor = 2.0f/(mx-mn);
        return *this;
    }
    
    float get(int n){
        float f = s[n];
        f -= calib[n].mn;
        f *= calib[n].factor;
        if(f<0)f=0;
        if(f>2.0f)f=2.0f;
        f-=1.0f;
        return f;
    }
    int raw(int n){
        return s[n];
    }
    
    /// true if received data recently
    bool ready(){
        return rdy;
    }
          
        
    
};
#endif /* __RC_H */
