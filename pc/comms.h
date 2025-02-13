/**
 * \file 
 * Handle low-level communications to the master, via a serial port.
 * 
 *
 */


#ifndef __COMMS_H
#define __COMMS_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdarg.h>

#include "status.h"

/// simulated serial device, in case you want to test stuff. Needs to be backed
/// by a real simulator object to fake the comms.

class Simulator {
public:
    /// returns how many chars read, ct is max amount
    virtual int read(char *buf,int ct) = 0;
    /// return -ve on error (which should be never in a simulator)
    virtual int write(const char *s,int ct)=0;
    virtual void update()=0; //!< update any simulation
    virtual void poll()=0; //!< poll sim for commands
};


/// serial communications class. Can handle both binary and text comms.
/// The protocol, however, always starts up in text mode and waits for
/// the string "Ready" on a line by itself (this is done in connect())


class SerialComms : public StatusObservable {
    int fd; //!< file descriptor
    timeval timeout; //!< timeout for reads
    FILE *log;
    Simulator *sim; //!< null if not simulated, or else a pointer to a serial simulator
    
    static int getBaudEnum(int baudRate){
        switch(baudRate){
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            return -1;
        }
    }
    
public:    
    
    /// status flag: connected
    static const StatusFlag CONNECTED=1;
    /// status flag: error occurred during connect
    static const StatusFlag ERROR=2;
    /// timeout in read
    static const StatusFlag TIMEOUT=4;
    /// waiting for connection
    static const StatusFlag CONNECTING=8;
    /// protocol error
    static const StatusFlag PROTOCOL_ERROR=64;
    
    static const StatusFlag ERRORFLAGS = (PROTOCOL_ERROR|TIMEOUT|ERROR);
    
    /// return true if the status is an error
    bool isError(){
        return (getStatus() & ERRORFLAGS)?true:false;
    }
    
    /// returns true if we're simulated
    bool isSim(){
        return sim!=NULL;
    }
    
    /// if there's a simulator, tick it
    void tickSim(){
        if(sim)sim->update();
    }
    /// check simulator for commands
    void pollSim(){
        if(sim)sim->poll();
    }
    
    /// connect to a simulation rather than a port.
    void simConnect(Simulator *s){
        sim = s;
    }
    
    
    
    /// connect to a serial port; disconnecting if required. Will set
    /// status flags to indicate success. Waits for the other end to
    /// reply with "Ready" on a line by itself - after that you're free
    /// to use text or binary.
    
    void connect(const char *dev, int baudRate){
        int br;
        struct timespec qqq={0,10000000};
        
        disconnect(); // first, disconnect.
        
        setTimeout(5,0); // initial 5 second timeout
        
        log = fopen("/tmp/comms.log","w");
        
        // read/write, not controlling terminal
        fd = open(dev,O_RDWR,O_NOCTTY);
        if(fd==-1){
            perror("unable to open device: ");
            goto error;
        }
        
        // drop DTR to reset the Arduino! Ugh.
        int st;
        ioctl(fd, TIOCMGET, &st);
        st &= ~TIOCM_DTR;
        ioctl(fd, TIOCMSET, &st);
        nanosleep(&qqq,NULL);
        st |= TIOCM_DTR;
        ioctl(fd, TIOCMSET, &st);
        
        if(fcntl(fd,F_SETFL,0)<0){
            perror("cannot fcntl: ");
            goto error;
        }
        
        struct termios options;
        if(tcgetattr(fd,&options)<0){
            perror("cannot get terminal options: ");
            goto error;
        }
        
        //
        // Input flags - Turn off input processing
        // convert break to null byte, no CR to NL translation,
        // no NL to CR translation, don't mark parity errors or breaks
        // no input parity check, don't strip high bit off,
        // no XON/XOFF software flow control
        //
        options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                             INLCR | PARMRK | INPCK | ISTRIP | IXON);            
        
        // Output flags - Turn off output processing
        // no CR to NL translation, no NL to CR-NL translation,
        // no NL to CR translation, no column 0 CR suppression,
        // no Ctrl-D suppression, no fill characters, no case mapping,
        // no local output processing
        //
        // options.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
        //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
        options.c_oflag = 0;            
        
        //
        // No line processing:
        // echo off, echo newline off, canonical mode off, 
        // extended input processing off, signal chars off
        //
        options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
        
        //
        // Turn off character processing
        // clear current char size mask, no parity checking,
        // no output processing, force 8 bit input
        //
        options.c_cflag &= ~(CSIZE | PARENB);
        options.c_cflag |= CS8|CREAD|CLOCAL|HUPCL;
        
        // Raw mode.
        // One input byte is enough to return from read()
        // Inter-character timer off
        //
        options.c_cc[VMIN]  = 1;
        options.c_cc[VTIME] = 0;
        
        
        //
        // Communication speed (simple version, using the predefined
        // constants)
        //
        br = getBaudEnum(baudRate);
        if(br<0){
            notifyMessage("unsupported baud rate: %d\n",baudRate);
            goto error;
        }
        
        if(cfsetispeed(&options, br) < 0 || cfsetospeed(&options, br) < 0) {
            notifyMessage("cannot set baud rate: ",strerror(errno));
            goto error;
        }
        //
        // Finally, apply the configuration
        //
        if(tcsetattr(fd, TCSAFLUSH, &options) < 0){
            notifyMessage("cannot set attrs: ",strerror(errno));
            goto error;
        }
        
        setStatus(CONNECTING);
        notifyMessage("core connected");
        
        /// wait for "Ready" line
        
        if(fd>=0){
            // wait for ready
            int i;
            for(i=0;i<10;i++){
                char buf[256];
                int ct = readLine(buf,256);
                if(ct>0){
                    notifyMessage("recvd : %s",buf);
                    if(!strcmp(buf,"Ready"))
                        break;
                }
            }
            if(i==10){
                setStatus(TIMEOUT,CONNECTING);
                notifyMessage("Timeout waiting for ready");
            }else{
                setStatus(CONNECTED,CONNECTING); // set, clear
                notifyMessage("Arduino ready");
            }
        }
        
        if(log)fprintf(log,"connected\n");
        
        // set timeout to shorter
        //        setTimeout(0,100*1000);
        setTimeout(1,0); // 1 second 
        sleep(2);
        
        return; // all done OK
error:
        // exception
        close(fd); 
        fd=-1;
        setStatus(ERROR);
    }
    
    /// set the timeout
    void setTimeout(int sec,int usec){
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;
    }
    
    
    /// disconnect and clear status
    void disconnect(){
        if(sim){
            sim = NULL;
        }
        if(fd>=0){
            clrStatus(CONNECTING|CONNECTED|TIMEOUT|ERROR|PROTOCOL_ERROR);
            close(fd);
            fd=-1;
        }
        if(log){
            fclose(log);
            log=NULL;
        }
    }        
    
    SerialComms() {
        log=NULL;
        fd=-1;
        sim=NULL;
    }
    
    ~SerialComms(){
        disconnect();
    }
    
    /// raw write
    int write(const char *s,int ct){
        if(sim){
            sim->write(s,ct);
            return 0;
        }
        if(isReady()){
            int rv = ::write(fd,s,ct);
            if(rv!=ct){
                notifyMessage("not enough bytes in write (%d!=%d)",rv,ct);
                return -1;
            }
        } else {
            notifyMessage("cannot write, not ready");
            return -2;
        }
        return 0;
    }
    
    /// clear a timeout status so we can retry. If TIMEOUT occurs, this
    /// must be cleared before we do anything!
    void clearTimeout(){
        clrStatus(TIMEOUT);
    }
    
    /// is the comms module in a timeout?
    bool isTimeout(){
        return 
              (getStatus()&TIMEOUT) != 0;
    }
    
    /// raw read - returns -1 on timeout, and sets the TIMEOUT status. Also returns -1 if called
    /// when disconnected, and if an error occurs. Does not check for connected status, just for valid fd.
    
    int read(char *buf,int ct){
        if(sim){
            return sim->read(buf,ct);
        }
        if(fd>=0){
            fd_set set;
            FD_ZERO(&set);
            FD_SET(fd,&set);
            
            // don't clear timeout - if a timeout occurred we'll be out of sync.            
            //            clrStatus(TIMEOUT); 
            clrStatus(ERROR);
            
            timeval to = timeout; // have to copy this!
            int rv = select(fd+1,&set,NULL,NULL,&to);
            
            if(!rv){
                setStatus(TIMEOUT);
                notifyMessage("timeout in read");
                return -2;
            } else if(rv<0) {
                setStatus(ERROR);
                notifyMessage("-ve value from select in read");
                return -3;
            }
            else 
                return ::read(fd,buf,ct);
        }
        else {
            setStatus(ERROR);
            return -1;
        }
    }
    
    /// return true if we are able to read and write
    bool isReady(){
        return sim || ( fd>=0 && !isError() && !(getStatus()&CONNECTING));
    }
    
    
    /// formatted string write, but not if we're in TIMEOUT or error
    int writef(const char *s,...){
        if(isReady()){
            char buf[256];
            va_list ap;
            va_start(ap,s);
            vsprintf(buf,s,ap);
            if(log)fprintf(log,"Written: %s\n",buf);
            int rv = write(buf,strlen(buf));
            va_end(ap);
            return rv;
        } else return -1;
        
    }
    
    /// read a line of text; returns -1 on error otherwise returns count.
    /// In the case of an error will reset the internal buffer. Does not check for connected
    /// status, just that there is a valid fd - this is because it might be 
    /// used to establish connected status.
    int readLine(char *buf,int maxlen){
        int n=0;
        char c;
        
        if(fd>=0){
            
            while(n<maxlen){
                int ct = read(&c,1);
                if(ct<0){
                    // if a timeout occurs, return -1 and reset the count
                    n=0;
                    return -1;
                }
                if(ct>0){
                    if(c==10){
                        buf[n++]=0;
                        break;
                    } else if(c!=13){
                        buf[n++]=c;
                    }
                }
            }
            if(log)fprintf(log,"read line *%s*\n",buf);
            return n;
        }
        return -1;
    }
    
};


inline void dump(const char *name,const uint8_t *s,int n){
    int i;
    char buf[128];
    strcpy(buf,"-----------------------------------------------------------------");
    sprintf(buf,"%s: %d bytes",name,n);
    buf[strlen(buf)]=' ';
    puts(buf);
    for(i=0;i<n;i++){
        if(!(i%8))printf("%04x : ",i);
        printf("%02x ",s[i]);
        if((i%8)==7)printf("\n");
    }
    if(i%8)printf("\n");
}


#endif /* __COMMS_H */
