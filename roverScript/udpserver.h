/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __UDPSERVER_H
#define __UDPSERVER_H

class UDPServerListener {
public:
    /// override this to parse key/value pairs in messages
    virtual void onKeyValuePair(const char *s,float v)=0;
};

class UDPServer {
private:
    int fd;
    UDPServerListener *listener;
    void parseMessage(const char *s);
public:
    UDPServer();
    int start(int port);
    void stop();
    
    void setListener(UDPServerListener *l){
        listener = l;
    }
    
    void poll();
};

#endif /* __UDPSERVER_H */
