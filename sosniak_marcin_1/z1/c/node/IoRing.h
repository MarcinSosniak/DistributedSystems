#ifndef IORING_H_INCLUDED
#define IORING_H_INCLUDED
#include "ringConnector.h"
#include "log.h"
#define TYPE_TCP 1
#define TYPE_UDP 0






class TwoThreadSafeQue
{
private:
    Message* m_msgBuff;
    int m_iReadFrom;
    int m_iWriteTo;
    int m_iSize;
public:
    TwoThreadSafeQue(int iSize);
    ~TwoThreadSafeQue();
    bool push(Message);
    bool pop(Message& msg);
};




class IoRing
{
private:
    AbstractConnector* m_myConnector;
    udpLogger m_logger;
    TwoThreadSafeQue* m_que;
    char m_scMyName[NAME_LENGTH+1];
    Message m_tmpMessage;
    bool m_fSuper=false;
    DWORD m_asynchrThreadId;
public:
    IoRing(string sMyName, char type,bool fSuper);
    ~IoRing();
    int initAsPartOfRing(int iMyPort,string sNextHopAddr, int iNextHopPort);
    int initFromOutSideOfRing(int iMyPort,string sRingConnectorAddr, int iRingConnectorPort);
    friend DWORD WINAPI function_toStartAsynchrnousIoRing(LPVOID lpParam);
    void queSend(string strTo,string sMsg);
private:
    bool fToMeWithPrint(Message &msg);
    void runAsynchronous();
    void run();
};

DWORD WINAPI function_toStartAsynchrnousIoRing(LPVOID lpParam);




#endif // IORING_H_INCLUDED
