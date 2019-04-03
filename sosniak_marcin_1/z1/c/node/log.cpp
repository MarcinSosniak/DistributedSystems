#include "log.h"

udpLogger::udpLogger(string sGroup, int iPort)
{
    if(sGroup.length() <= 0 )
    {
        m_sGroup=LOG_DEFAULT_GROUP;
        m_iPort=LOG_DEFAULT_PORT;
    }
    else
    {
        m_sGroup=sGroup;
        m_iPort=iPort;
    }
    init();
}


udpLogger::udpLogger()
{
    m_sGroup=LOG_DEFAULT_GROUP;
    m_iPort=LOG_DEFAULT_PORT;
    init();
}


void udpLogger::init()
{
    //WSADATAInit();
    m_SocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_SocketFd < 0) {
        printf(" Listener socket creation error code:%d \n",WSAGetLastError());
        exit(1);
    }

    memset(&m_Addr, 0, sizeof(m_Addr));
    m_Addr.sin_family = AF_INET;
    m_Addr.sin_addr.s_addr = inet_addr(m_sGroup.c_str());
    m_Addr.sin_port = htons(m_iPort);
}

int udpLogger::log(string msg)
{
    string sOutMsg=getTime();
    sOutMsg+=msg;

    int iSendBytes=sendto(m_SocketFd,sOutMsg.c_str(),strlen(sOutMsg.c_str()),0,(struct sockaddr*) &m_Addr,sizeof(m_Addr));
    if (iSendBytes<0)
    {
        printf(" log error code:%d \n",WSAGetLastError());
    }
    return iSendBytes;
}

string udpLogger::getTime()
{
    char buff[10];
    memset(buff,0,10);
    sprintf(buff,"%ld ",time(NULL));
    return buff;
}


void WSADATAInit()
{
   WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        printf("WSAStartup ");
        printf("error code:%d\n",WSAGetLastError());
        exit(1);
    }
}
