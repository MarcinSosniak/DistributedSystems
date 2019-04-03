#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
#include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
#include <Windows.h>
#include <iostream>
#include <time.h>
#include <stdio.h>
using namespace std;

#define LOG_DEFAULT_GROUP "224.1.1.1"
#define LOG_DEFAULT_PORT 5007


class udpLogger
{
    private:
        int m_SocketFd=-1;
        string m_sGroup;
        int m_iPort;
        struct sockaddr_in m_Addr;
    public:
        udpLogger(string sGroup, int iPort);
        udpLogger();
        int log(string sMsg);
    private:
        string getTime();
        void init();
};


void WSADATAInit();


#endif // LOG_H_INCLUDED
