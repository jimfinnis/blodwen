/**
 * \file
 * Binary I2C PC->master->slave protocol, imposing a register
 * read/write paradigm onto the raw serial connection with the
 * master.
 */


#ifndef __SLAVE_H
#define __SLAVE_H

#include "comms.h"
#include "status.h"
#include "regconfig.h"
#include "regs.h"
#include "regsauto.h"

#include <stdint.h>
#include "roverexcept.h"

#define CMD_WRITE 2 //!< register changes
#define CMD_READ 3  //!< read registers in read set
#define CMD_SETREADSET 4  //!< change the read set

#define READSET_DRIVESTEER 0
#define READSET_LIFT	1
#define READSET_MASTER  2

#define READSET_SPARE 4

///an exception thrown when a slave communication generates
///an error - typically due to a protocol failure.
class SlaveException : public RoverException {
public:
    SlaveException(const char *s,...){
        va_list ap;
        va_start(ap,s);
        vsprintf(msg,s,ap);
        va_end(ap);
    }
};




/// encapsulates the low-level comms protocol by preceding blocks
/// with a byte giving their length so that the Arduino knows how
/// much to read, and with the command code and device ID.
/// This object should, in turn, be wrapped by a SlaveDevice
/// (or a number of them)

class SlaveProtocol {
    /// the buffer for commands. The count is in the first byte.
    /// This is 256 in size because the count is a byte wide.
    uint8_t buf[256];
    /// the buffer count is also kept here
    uint8_t ct;
    
    /// internal - asserts that we are actually inside a block
    /// (since starting a block will add size and command, count will
    /// be non-zero)
    void assertInBlock(){
        if(!ct)throw SlaveException("adding command data while not in a block");
    }
    
public:
    
    /// the comms system 
    SerialComms *comms;
    
    /// constructor - we still need to call init() after this
    SlaveProtocol(){
        comms = NULL;
        ct=0;
    }
    
    /// initialise the protocol, telling it which comms we're using
    void init(SerialComms *c){
        comms = c;
    }
    
    /// start a new command, putting the slave ID into the top four bits
    void start(int id,int cmd){
        ct=2; // size of message including the count byte
        
        cmd |= id << 4; // top four bits are optional ID change code
        // buf[0] will contain the size, buf[1] contains the command
        // and device ID
        buf[1] = cmd;
    }
    
    /// add a block of memory to the block to be sent
    void add(uint8_t *ptr,int size){
        assertInBlock();
        if(size+*buf > 255)
            throw SlaveException("message too long");
        
        memcpy(buf+ct,ptr,size);
        ct+=size;
    }
    
    /// add a single byte to the block to be sent
    void addByte(uint8_t b){
        assertInBlock();
        if(ct==255)
            throw SlaveException("message too long");
        buf[ct++]=b;
    }
    
    /// send the block if there is any block to be sent
    void send(){
        assertInBlock();
        if(ct>1){
            buf[0]=ct;
            int rv = comms->write((const char *)buf,ct);
            //            dump("Write",buf,ct);
            ct=0;
            if(rv<0)
                throw SlaveException("cannot write block: %d",rv);
        }
    }
    
    /// read a block of known length - will keep reading until the
    /// correct number of bytes has been read.
    void readBlock(uint8_t *ptr,int size){
        while(size){
            int rv = comms->read((char *)ptr,size);            
            if(rv<0){
                if(comms->getStatus()&SerialComms::TIMEOUT){
                    throw SlaveException("time out in read");
                } else {
                    throw SlaveException("error in read");
                }
            }
            //            dump("Partial read",ptr,rv);
            ptr+=rv;
            size-=rv;
        }
        //        dump("TOTAL READ",base,qqq);
    }
};

/// encapsulates a slave device via a binary interface.
/// Registers are read in a block read rather than one
/// by one. Registers to be read should be at the start
/// of the register block.

class SlaveDevice {
    /// the slave protocol we're using, which in turn contains
    /// the comms device
    SlaveProtocol *p;
    
    /// the ID of the slave on the I2C bus.
    int devID;
    /// the register table we're using
    const Register *regs;
    
    /// comms buffer - this is 256 in size because the maximum size 
    /// of a protocol block is 256 bytes (the count in the first byte
    /// of such a block being a byte wide.)
    uint8_t buf[256];
    /// current number of bytes in comms buffer
    int ct;
    /// register values - these are from the read set, so if the read
    /// set is 2,3,4 then regVals[0-2] will be values from registers 2,3,4
    uint16_t regVals[64];
    /// how many registers in the table
    int regCt;
    
    static uint8_t readSet[READSETS][READSETSIZE];
    static uint8_t readSetCt[READSETS];
    
    /// the set we have just read with readSet()
    int curSet;
    
public:
    
    int getAddr(){
        return devID;
    }
    
    /// construct - we still need to call init() after this
    SlaveDevice(){
        p = NULL;
        ct=0;
        for(int i=0;i<READSETS;i++)
            readSetCt[i]=0;
    }
    
    /// connect, setting up a status listener,
    /// telling us which comms device to use, which register set I'm
    /// using, and what my device number is. Returns a reference
    /// to the object itself, for use in fluent programming.
    
    SlaveDevice &init(SlaveProtocol *_p, const Register *table, int id){
        p = _p;
        printf("    initialising device %d\n",id);
        devID = id;
        regs = table;
        for(int i=0;;i++){
            if(regs[i].sizeAndFlags==32){
                regCt=i;
                break;
            }
        }
        return *this;
    }
    
    /// begin a block of register writes - these will be built up
    /// inside the buffer until endWrites(), when they are sent to
    /// the protocol. This is done because the number of writes must
    /// appear at the start of the message
    
    void startWrites(){
        if(!isConnected())return;
        p->start(devID,CMD_WRITE); // start the command
        buf[0]=0; // the number of writes is set to zero
        ct=1; // one byte in the buffer so far (the number of writes)
    }
    
    /// end a block, adding the buffer to the protocol output buffer 
    /// and sending it. We then wait for a response byte, which should
    /// be zero.
    
    void endWrites(){
        if(!isConnected())return;
        uint8_t readbuf[8];
        // add the writes to the main output buffer
        p->add(buf,ct);
        // send
        p->send();
        // and wait for a response - just one byte
        p->readBlock(readbuf,1);
        if(readbuf[0])
            throw SlaveException("error in reg write: %d",readbuf[0]);
    }
    
    /// add a register write to the buffer - must be between startWrites()
    /// and endWrites(). This is for 'unmapped' registers, which are
    /// raw 16-bit integer values.
    void writeInt(uint8_t reg,uint16_t val){
        if(!isConnected())return;
        if(ct<256){
            buf[0]++;
            buf[ct++]=reg;
            buf[ct++]=val&0xff;
            if(regs[reg].getSize() == 2) // extra byte if reqd.
                buf[ct++]=val>>8;
        } else {
            throw SlaveException("too many writes in one block");            
        }
    }
    
    /// add a register write to the buffer - must be between startWrites()
    /// and endWrites(). This is for 'mapped' registers, which are
    /// mapped from a floating point range onto unsigned 16-bit values 
    /// for transmission.
    
    void writeFloat(int r, float v){
        if(!isConnected())return;
        if(r>=regCt)
            throw SlaveException("%d is not a sensible register",r);
        if(!regs[r].isInRange(v)){
            throw SlaveException("%f out of range for register %d",v,r);
            p->comms->setStatus(SerialComms::PROTOCOL_ERROR);
        }
        uint16_t i = regs[r].map(v);
        writeInt(r,i);
    }
    
    /// send a write command to reset this slave's exceptions
    void resetExceptions(){
        if(!isConnected())return;
        startWrites();
        writeInt(REG_RESET,RESET_EXCEPTIONS);
        endWrites();
    }
    
    /// Set a read set. This consists of the set number, and alist of
    /// registers which the device should send when we request a read for that set.
    /// The list is terminated with -1.
    /// Note that the read set will change for ALL devices,
    /// not just this one!
    void setReadSet(int set, int first,...){
        va_list ptr;
        va_start(ptr,first);
        
        if(!isConnected())return;
        readSetCt[set]=0; // clear our copy of the read set
        p->start(devID,CMD_SETREADSET); // start the command
        p->addByte(set); // add the set index
        p->addByte(first); // add the first item to the command 
        readSet[set][readSetCt[set]++]=first; // and our copy
        
        for(;;){ // for remaining items
            int n = va_arg(ptr,int); // get next item
            if(n<0)break; // if it's -ve, break
            if(readSetCt[set]==READSETSIZE)
                throw SlaveException("read set too large");
            p->addByte((uint8_t)n); // add item to command
            readSet[set][readSetCt[set]++]=(uint8_t)n; // and our copy
        }
        p->send(); // send command
        // and wait for a response - just one byte
        uint8_t readbuf[8];
        p->readBlock(readbuf,1);
        if(readbuf[0]!=readSetCt[set]) // should be the count
            throw SlaveException("error in read set: %d, should be %d",readbuf[0],readSetCt[set]);
    }
    
    
    
    
    /// request a read of the current read set and await the response block.
    void readRegs(int set){
        curSet = set;
        
        /// calculate the size of the response
        int size=0;
        for(int i=0;i<readSetCt[set];i++){
            size+=regs[readSet[set][i]].getSize();
        }
        
        // start the command
        p->start(devID,CMD_READ);
        // send the read set
        p->addByte(set);
        // send
        p->send();
        // await the response
        p->readBlock(buf,size);
        
        // copy values into register holding area
        
        uint8_t *ptr = buf;
        
        for(int i=0;i<readSetCt[set];i++){
            uint16_t v=0;
            v=*ptr++;
            if(regs[readSet[set][i]].getSize()==2)
                v+=*ptr++ << 8;
            //            printf("%x: Reg %d = %x\n",(ptr-buf),readSet[i],v);
            regVals[i]=v;
        }
    }    
    
    /// after calling readRegs, this can be used to get register values;
    /// the index is the read set index, so if the read set is 2,3,4 then
    /// getRegInt(0..2) will get values for registers 2,3 and 4.
    uint16_t getRegInt(int n){
        return regVals[n];
    }
    
    /// after calling readRegs, this can be used to get register values;
    /// the index is the read set index, so if the read set is 2,3,4 then
    /// getRegInt(0..2) will get values for registers 2,3 and 4.
    float getRegFloat(int n){
        return regs[readSet[curSet][n]].unmap(getRegInt(n));
    }
    
    
    
    /// are we connected?
    bool isConnected(){
        return p && p->comms && p->comms->isReady();
    }
    
};




#endif /* __SLAVE_H */
