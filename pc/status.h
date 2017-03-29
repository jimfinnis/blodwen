/**
 * \file
 * Observer/observable pattern for status changes and
 * error messages.
 */


#ifndef __STATUS_H
#define __STATUS_H

#include <stdint.h>

/// statuses are 32-bit integers

typedef int32_t StatusFlag;

/// interface you can specify to be used when status changes.

class StatusListener {
public:
    /// called when a status update message occurs.
    virtual void onMessage(const char *str){}
    
    /// called when the system's status changes; takes a new set
    /// of status flags (which are system dependent)
    virtual void onStatusChange(StatusFlag flags){}
};

/// classes which can have the above listener attached to them
/// need to inherit this.

class StatusObservable {
private:
    /// the listener
    StatusListener *statusListener;
    /// the current status (starts at 0)
    StatusFlag status;
    

    void notifyStatusChange(){
        if(statusListener)
            statusListener->onStatusChange(status);
    }

public:
    StatusObservable() {
        status=0;
        statusListener = NULL;
    }
    
    /// set an event listener of some kind
    void setStatusListener(StatusListener *l){
        statusListener = l;
    }
    
    /// get the current status
    StatusFlag getStatus(){
        return status;
    }
    
    /// tell the listeners about a new message
    void notifyMessage(const char *s,...){
        if(statusListener){
            char buf[256];
            va_list ap;
            va_start(ap,s);
            vsprintf(buf,s,ap);
        
            statusListener->onMessage(buf);
        
            va_end(ap);
        }
    }

    /// set some status flags, maybe clear some other flags, and notify
    void setStatus(StatusFlag set,StatusFlag clr=0){
        status |= set;
        status &= ~clr;
        notifyStatusChange();
    }
    
    
    /// just clear some status flags and notify
    void clrStatus(StatusFlag clr){
        status &= ~clr;
        notifyStatusChange();
    }        
    
};
    
    
#endif /* __STATUS_H */
