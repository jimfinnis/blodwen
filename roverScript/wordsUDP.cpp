/**
 * @file
 * Handles the UDP communications
 * 
 */

#include "../pc/rover.h"
#include "angort.h"
#include "roverexceptions.h"
extern void udpwrite(const char *s,...);
extern void handleUDP();

/// head of a linked list of UDP properties
static class UDPProperty *headUDPPropList=NULL;

class UDPProperty : public Property {
public:
    /// linkage field
    UDPProperty *next;
    
    /// actual value; gets copied to and from
    /// field v for Angort access
    float val;
    
    /// name of property
    const char *name;
    
    UDPProperty(const char *n){
        val=0;
        // add to list
        next=headUDPPropList;
        headUDPPropList=this;
        name = strdup(n);
    }
    
    ~UDPProperty(){
        free((void *)name);
    }
                   
    
    void set(float f){
        val = f;
    }
    
    virtual void postSet(){
        val = v.toFloat();
    }
    
    virtual void preGet(){
        Types::tFloat->set(&v,val);
    }
};

void setUDPProperty(const char *name,float val){
    for(UDPProperty *p=headUDPPropList;p;p=p->next){
        if(!strcmp(name,p->name)){
            p->val = val;
            return;
        }
    }
    throw Exception(EX_NOTFOUND).set("cannot find UDP property %s",name);
}

void sendUDPProperties(){
    for(UDPProperty *p=headUDPPropList;p;p=p->next){
        udpwrite("%s=%f",p->name,p->val);
    }
}    


%name udp



%word handleudp (--) send and receive queued UDP data
{
    handleUDP();
}


%word addudpvar (name --) create a new global which mirrors the monitor
{
    const StringBuffer& b = a->popString();
    a->registerProperty(b.get(),new UDPProperty(b.get()));
}

%word udpwrite (string --) write a string to the UDP port for the monitor
{
    const StringBuffer& b = a->popString();
    udpwrite(b.get());
}    

