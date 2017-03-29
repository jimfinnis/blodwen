/**
 * \file
 * Rover exceptions - this are thrown when there is an error;
 * there are several subclasses.
 * 
 */

#ifndef __ROVEREXCEPT_H
#define __ROVEREXCEPT_H

#include <exception>

/// an exception thrown when there is an error in rover code - typically
/// a subclass such as SlaveException is thrown.
class RoverException : public std::exception {
protected:
    char msg[256];
        
public:
    RoverException(){}
    RoverException(const char *m){
        strcpy(msg,m);
    }
    virtual const char *what(){
        return msg;
    }
};



#endif /* __ROVEREXCEPT_H */
