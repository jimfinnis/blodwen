/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __UDPCLIENT_H
#define __UDPCLIENT_H

#define DEFAULT_HOSTNAME "192.168.0.100"
#define PORT 13231

extern const char *hostName;
bool udpSend(const char *msg);

#endif /* __UDPCLIENT_H */
