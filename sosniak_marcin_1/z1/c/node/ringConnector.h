#ifndef RINGCONNECTOR_H_INCLUDED
#define RINGCONNECTOR_H_INCLUDED

#define CONTROL_SOCKET_PORT_OFFSET 1000

/*
-- checkToken results:
    0 - positve, token passed
    MESSSAGE_SEEN  = -1 , token already seen, messege to be cleared, checked by everyone
    informational only return.
    INVALID_TOKEN_SECURITY = -2 checked by super node only, messegae to be terminated, destruction token to be send.
*/

#define CHECK_TOKEN_POSITVE 0
#define CHECK_TOKEN_MSG_SEEN -1
#define CHECK_TOKEN_SECURITY_ERROR -2
#define CHECK_TOKEN_CONTROL_TOKEN_REQ 1
#define CHECK_TOKEN_CONTROL_TOKEN_ACK 2
#define CHECK_TOKEN_UNEXPECTED_ACK -3
#define CHECK_TOKEN_NAN 255

#define MAX_SECURITY_RAND 255



//#define DEBUG_PRINT 1



#include "structs_def.h"

#include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
#include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
#include <Windows.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;




class AbstractConnector
{
    protected:
        unsigned short m_iLastMessageId=0;
        unsigned short m_iExpectedTokenSecurity=0;
        bool m_fSuper=false;
        Token m_Token;
        bool m_ready=false;
        bool m_fMyInit=false;
        string m_sMyAddr;
        int m_iMyPort;
        string m_sTargetAddr;
        int m_iTargetPort;

        int m_ControlSocket=-1;
        struct sockaddr_in m_ControlSocketAddr;

    public:
        void setSuper();
        virtual void init(string sMyAddr,int iMyPort, string sTargetAddr, int iTargetPort) =0;
        virtual int sendMSG(Message msg)=0;
        virtual int reSend()=0;
        virtual Message recieve()=0;
        virtual int connectToExisting(string sMyAddr,int iMyPort,string sTargetAddr, int iTargetPort)=0; // init not rquired
    protected:
        virtual void myInit(string sMyAddr,int iMyPort)=0;
        int checkToken(); // also clears outdated messages
        void initControlSocket(); // uses My address, and my Port + CONTROL_SOCKET_PORT_OFFSET
        //requeres derived class "init to be succesful"
};

class UdpConnector : public AbstractConnector
{
private:
    int m_TokenRingSocket=-1;
    struct sockaddr_in m_TokenRingMyAddr;
    struct sockaddr_in m_TokenRingTargetAddr;

public:
    UdpConnector();
    virtual void init(string sMyAddr,int iMyPort, string sTargetAddr, int iTargetPort) override;
    virtual int sendMSG(Message msg) override;
    virtual int reSend() override;
    virtual Message recieve() override;
    virtual int connectToExisting(string sMyAddr,int iMyPort,string sTargetAddr, int iTargetPort) override;
protected:
    int handleControlReq();
    virtual void myInit(string sMyAddr,int iMyPort);
};

int checkWithExit(int , string);


class TcpConnector : public AbstractConnector
{
private:
    int m_TokenRingSerwer=-1;
    int m_TokenRingListeningSocket=-1;
    int m_TokenRingSendingSocket=-1;
    struct sockaddr_in m_TokenRingMyAddr;
    struct sockaddr_in m_TokenRingTargetAddr;
    bool bListeningSocketConnected=false;
    DWORD m_MonitorThreadId=0;
    HANDLE m_Sem;
public:
    virtual void init(string sMyAddr,int iMyPort, string sTargetAddr, int iTargetPort) override;
    virtual int sendMSG(Message msg) override;
    virtual int reSend() override;
    virtual Message recieve() override;
    virtual int connectToExisting(string sMyAddr,int iMyPort,string sTargetAddr, int iTargetPort) override;
    friend DWORD WINAPI runMonitorListening_Func(LPVOID lpParam);
protected:
    void getSem(string);
    void releaseSem();
    int reAccept();
    int handleControlReq();
    virtual void myInit(string sMyAddr,int iMyPort);
    void targetInit(string sTargetAddr,int iTargetPort);
    void monitorListening();
    void runMonitorListening();
};

DWORD WINAPI runMonitorListening_Func(LPVOID lpParam);

#endif // RINGCONNECTOR_H_INCLUDED
