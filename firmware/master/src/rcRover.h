/**
 * @file
 * Remote control code specific to this app.
 * 
 */

#ifndef __RCROVER_H
#define __RCROVER_H

#include "rc.h"

class RoverRCReceiver : public RCReceiver {
    float steerPos;
    bool calibrated;
    
    void calibrate(); //!< send default calibration data
public:
    
    RoverRCReceiver() : RCReceiver(){
        steerPos=0;
        calibrated=false;
    }
    
    /// call this if the link dropped in the last update
    virtual void onUp();
    
    
    /// call this if the link came up in the last update
    virtual void onDown();
    
    /// read values and act on them
    virtual void update();
};


#endif /* __RCROVER_H */
