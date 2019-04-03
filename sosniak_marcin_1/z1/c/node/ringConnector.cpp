#include "structs_def.h"
#include "ringConnector.h"


Token generateCtrlToken(const char* addr, int iPort ,unsigned char TOKEN_FLAG)
{
    #ifdef DEBUG_PRINT
    printf("In generate CtrlToken arugments %s,%d,%d\n",addr,iPort,TOKEN_FLAG);
    #endif // DEBUG_PRINT
   if(strlen(addr)>=MAX_ADDR_LEN)
   {
       printf("tried to issude to long addr to control token");
       Token randomToken;
       //CRASH
       int* hue=nullptr;
       printf("%d",*hue);
       return randomToken;
   }


    Token control_token;
    control_token.iMessageId=0;
    control_token.iTokenSecurity=0;
    control_token.flags=TOKEN_FLAG;
    ControlMessage* ctrlMsg=(ControlMessage*)(&(control_token.msg));
    ctrlMsg->iPortNr=iPort;
    strcpy(ctrlMsg->sAddr,addr);
    return control_token;
}


void AbstractConnector::initControlSocket()
{
    memset((void*) &m_Token,0,sizeof(m_Token));
    if(!m_ready)
    {
        printf("\ncannot use AbstractConnector::initControlSocket without using derived class .init(...)\n");
        return;
    }
    m_ControlSocket= checkWithExit(socket(AF_INET,SOCK_DGRAM,0),"Control Socket creation");

    memset(&m_ControlSocketAddr,0,sizeof(m_ControlSocketAddr));
    m_ControlSocketAddr.sin_family = AF_INET;
    m_ControlSocketAddr.sin_addr.s_addr = inet_addr(m_sMyAddr.c_str());
    m_ControlSocketAddr.sin_port = htons(m_iMyPort+CONTROL_SOCKET_PORT_OFFSET);

    checkWithExit(bind(m_ControlSocket,(sockaddr*) &m_ControlSocketAddr,sizeof(m_ControlSocketAddr)),"bind control socket");
}

void AbstractConnector::setSuper()
{
    m_fSuper=true;
}

int AbstractConnector::checkToken()
{
    //check message id, if not laready seen
    if(m_Token.flags & TOKEN_CTRL_ACK)
    {
        #ifdef DEBUG_PRINT
        printf("flags=%d,TOKEN_CTRL_ACK=%d\n",m_Token.flags,TOKEN_CTRL_ACK);
        #endif
        return CHECK_TOKEN_CONTROL_TOKEN_ACK;
    }
    if(m_Token.flags & TOKEN_CTRL_REQ)
    {
        #ifdef DEBUG_PRINT
        printf("flags=%d,TOKEN_CTRL_REQ=%d\n",m_Token.flags,TOKEN_CTRL_REQ);
        #endif
        return CHECK_TOKEN_CONTROL_TOKEN_REQ;
    }


    int iRet=CHECK_TOKEN_POSITVE;
    if(m_iLastMessageId!=0 && m_iLastMessageId==m_Token.iMessageId)
    {
        m_Token.iMessageId=m_Token.iMessageId+1;
        clearMessage(m_Token.msg);
        iRet=CHECK_TOKEN_POSITVE;
    }
    if(m_iLastMessageId==0 && m_iLastMessageId!=m_Token.iMessageId)
    {
        m_iLastMessageId=m_Token.iMessageId;
    }

    if(m_fSuper)
    {
        if(m_iExpectedTokenSecurity==0 || m_iExpectedTokenSecurity==m_Token.iTokenSecurity)
        {
            m_iExpectedTokenSecurity=rand()%(MAX_SECURITY_RAND-1)+1;
            m_Token.iTokenSecurity=m_iExpectedTokenSecurity;
        }
        else
        {
            iRet=CHECK_TOKEN_SECURITY_ERROR;
        }
    }
    m_iLastMessageId=m_Token.iMessageId;
    return iRet;
}


int checkWithExit(int in,string  msg)
{
    if (in < 0)
    {
        printf("%s error code:%d \n",msg.c_str(),WSAGetLastError());
        exit(0);
    }
    return in;
}


UdpConnector::UdpConnector()
{
    ;
}


void UdpConnector::myInit(string sMyAddr,int iMyPort)
{
    m_fMyInit=true;

    if(sMyAddr.compare("")==0)
        m_sMyAddr="127.0.0.1";
    else
        m_sMyAddr=sMyAddr;



    m_iMyPort=iMyPort;


    m_TokenRingSocket= checkWithExit(socket(AF_INET,SOCK_DGRAM,0),"TokenRing Socket creation");

    memset(&m_TokenRingMyAddr,0,sizeof(m_TokenRingMyAddr));
    m_TokenRingMyAddr.sin_family = AF_INET;
    m_TokenRingMyAddr.sin_addr.s_addr = inet_addr(m_sMyAddr.c_str());
    m_TokenRingMyAddr.sin_port = htons(m_iMyPort);

    checkWithExit(bind(m_TokenRingSocket,(sockaddr*) &m_TokenRingMyAddr,sizeof(m_TokenRingMyAddr)),"bind ring socket");

    #ifdef DEBUG_PRINT
    printf("finished myInit()\n");
    #endif // DEBUG_PRINT


}

void UdpConnector::init(string sMyAddr,int iMyPort, string sTargetAddr, int iTargetPort)
{
    m_ready=true;

    myInit(sMyAddr,iMyPort);

    if(sTargetAddr.compare("")==0)
        m_sTargetAddr="127.0.0.1";
    else
        m_sTargetAddr=sTargetAddr;

    m_iTargetPort=iTargetPort;
    //init target address;
    memset(&m_TokenRingTargetAddr,0,sizeof(m_TokenRingTargetAddr));
    m_TokenRingTargetAddr.sin_family = AF_INET;
    m_TokenRingTargetAddr.sin_addr.s_addr = inet_addr(m_sTargetAddr.c_str());
    m_TokenRingTargetAddr.sin_port = htons(m_iTargetPort);

    initControlSocket();
    printf("finished UDP init\n");
}

int UdpConnector::sendMSG(Message msg)
{

    m_Token.iMessageId=m_Token.iMessageId+1;
    memcpy((void*) &m_Token.msg,(void *) &msg,sizeof(msg));
    #ifdef DEBUG_PRINT
    printf("sending this token:\n");
    printToken(m_Token);
    #endif // DEBUG_PRINT
    int out= checkWithExit(sendto(m_TokenRingSocket,(char*) &m_Token,sizeof(m_Token),0,(struct sockaddr*) &m_TokenRingTargetAddr,sizeof(m_TokenRingTargetAddr)),"on send");
    memset((void*) &(m_Token.msg),0,sizeof(m_Token.msg));
    return out;
}

int UdpConnector::reSend()
{
    #ifdef DEBUG_PRINT
    printf("reSending this token:\n");
    printToken(m_Token);
    #endif // DEBUG_PRINT
    int out= checkWithExit(sendto(m_TokenRingSocket,(char*) &m_Token,sizeof(m_Token),0,(struct sockaddr*) &m_TokenRingTargetAddr,sizeof(m_TokenRingTargetAddr)),"on resend");
    memset((void*) &(m_Token.msg),0,sizeof(m_Token.msg));
    return out;
}

Message UdpConnector::recieve()
{
    int bTokenCheckState=CHECK_TOKEN_NAN;
    while(bTokenCheckState!=CHECK_TOKEN_POSITVE)
    {
        int iSize=0;
        while (iSize<(int)sizeof(m_Token))
        {
            iSize+=checkWithExit(recv(m_TokenRingSocket,(char*) (&m_Token+iSize), sizeof(m_Token),0),"on receive");
        }
        bTokenCheckState=checkToken();
        #ifdef DEBUG_PRINT
        printf("recieved this token withState(%d):\n",bTokenCheckState);
        printToken(m_Token);
        #endif // DEBUG_PRINT
        if(bTokenCheckState==CHECK_TOKEN_CONTROL_TOKEN_REQ)
            handleControlReq();
    }
    return m_Token.msg;

}


int UdpConnector::handleControlReq()
{

    #ifdef DEBUG_PRINT
        printf("in Handle ControlReq\n");
        printControlToken(m_Token);
    #endif // DEBUG_PRINT



    Token ResponseToken=generateCtrlToken(m_sTargetAddr.c_str(),m_iTargetPort,TOKEN_CTRL_ACK);

    struct sockaddr_in new_next_addr;
    ControlMessage* recvCotnrolMsg= (ControlMessage*) &(m_Token.msg);


    new_next_addr.sin_family = AF_INET;
    new_next_addr.sin_addr.s_addr = inet_addr(recvCotnrolMsg->sAddr);
    new_next_addr.sin_port = htons(recvCotnrolMsg->iPortNr);


    #ifdef DEBUG_PRINT
        printf("trying to send ACKTToken\n");
        printControlToken(ResponseToken);
    #endif // DEBUG_PRINT
    checkWithExit(sendto(m_TokenRingSocket,(char*) &ResponseToken,sizeof(ResponseToken),0,(struct sockaddr*) &new_next_addr,sizeof(new_next_addr)),"on send TOKEN_CTRL_ACK");

    m_TokenRingTargetAddr=new_next_addr;
    m_sTargetAddr=recvCotnrolMsg->sAddr;
    m_iTargetPort=recvCotnrolMsg->iPortNr;
    return 0;
}



int UdpConnector::connectToExisting(string sMyAddr,int iMyPort,string sTargetAddr, int iTargetPort)
{
    if(m_ready)
    {
        #ifdef DEBUG_PRINT
        printf("tried to connectToExisting() after init()\n");
        #endif // DEBUG_PRINT
        return -1;
    }
    if(!m_fMyInit)
    {
        myInit( sMyAddr, iMyPort);
    }

    #ifdef DEBUG_PRINT
    printf("connecting after myInit()\n");
    #endif // DEBUG_PRINT



    if(sTargetAddr.compare("")==0)
        m_sTargetAddr="127.0.0.1";
    else
        m_sTargetAddr=sTargetAddr;

    m_iTargetPort=iTargetPort;
    //init target address;
    memset(&m_TokenRingTargetAddr,0,sizeof(m_TokenRingTargetAddr));
    m_TokenRingTargetAddr.sin_family = AF_INET;
    m_TokenRingTargetAddr.sin_addr.s_addr = inet_addr(m_sTargetAddr.c_str());
    m_TokenRingTargetAddr.sin_port = htons(m_iTargetPort);


    Token askToken=generateCtrlToken(m_sMyAddr.c_str(),m_iMyPort,TOKEN_CTRL_REQ);

    #ifdef DEBUG_PRINT
    printf("trying to send ask token\n");
    printControlToken(askToken);
    #endif // DEBUG_PRINT



    checkWithExit(sendto(m_TokenRingSocket,(char*) &askToken,sizeof(askToken),0,(struct sockaddr*) &m_TokenRingTargetAddr,sizeof(m_TokenRingTargetAddr)),"on send ctrl token REQ");

    int iSize=0;
    while ( iSize< (int)sizeof(m_Token))
    {
        iSize+=checkWithExit(recv(m_TokenRingSocket,(char*) (&m_Token+iSize), sizeof(m_Token),0),"on control token back rcv");
    }

    #ifdef DEBUG_PRINT
    printf("got back from ctrl_req:\n");
    printControlToken(m_Token);
    #endif // DEBUG_PRINT

    if(checkToken()!=CHECK_TOKEN_CONTROL_TOKEN_ACK)
    {
        #ifdef DEBUG_PRINT
        printf("go invalid response got%d\n",checkToken());
        #endif // DEBUG_PRINT
        return -1;
    }
    ControlMessage* ctrlMsg=(ControlMessage*) &(m_Token.msg);

    string sNewTargetAddr=ctrlMsg->sAddr;
    int iNewTargetAddr=ctrlMsg->iPortNr;
    if(sNewTargetAddr.compare("")==0)
        m_sTargetAddr="127.0.0.1";
    else
        m_sTargetAddr=sNewTargetAddr;

    m_iTargetPort=iNewTargetAddr;

    memset(&m_TokenRingTargetAddr,0,sizeof(m_TokenRingTargetAddr));
    m_TokenRingTargetAddr.sin_family = AF_INET;
    m_TokenRingTargetAddr.sin_addr.s_addr = inet_addr(m_sTargetAddr.c_str());
    m_TokenRingTargetAddr.sin_port = htons(m_iTargetPort);

    m_ready=true;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TcpConnector::init(string sMyAddr,int iMyPort, string sTargetAddr, int iTargetPort)
{
    if(m_fSuper)
    {
        targetInit(sTargetAddr, iTargetPort);
        myInit(sMyAddr, iMyPort);
    }
    else
    {
        myInit(sMyAddr, iMyPort);
        targetInit(sTargetAddr, iTargetPort);
    }
    m_ready=true;
    printf("TPC init succesfull\n");
    runMonitorListening();
}
int TcpConnector::sendMSG(Message msg)
{
    m_Token.iMessageId=m_Token.iMessageId+1;
    memcpy((void*) &m_Token.msg,(void *) &msg,sizeof(msg));
    #ifdef DEBUG_PRINT
    printf("sending this token from socket(%d):\n",m_TokenRingSendingSocket);
    printToken(m_Token);
    #endif // DEBUG_PRINT
    getSem("sendMSG");
    int out= checkWithExit(send(m_TokenRingSendingSocket,(char*) &m_Token,sizeof(m_Token),0),"on send");
    releaseSem();
    memset((void*) &(m_Token.msg),0,sizeof(m_Token.msg));
    return out;

}
int TcpConnector::reSend()
{
    #ifdef DEBUG_PRINT
    printf("sending this token:\n");
    printToken(m_Token);
    #endif // DEBUG_PRINT
    getSem("reSend");
    int out= checkWithExit(send(m_TokenRingSendingSocket,(char*) &m_Token,sizeof(m_Token),0),"on resend");
    releaseSem();
    memset((void*) &(m_Token.msg),0,sizeof(m_Token.msg));
    return out;
}
Message TcpConnector::recieve()
{
    bool gotNormalMessage=false;
    while(!gotNormalMessage)
    {
        int iRet=recv(m_TokenRingListeningSocket,(char*)&m_Token,sizeof(m_Token),0);
        if (iRet<0)
        {
            int iErrno= WSAGetLastError();
            if(iErrno==WSAECONNRESET || iErrno==WSAESHUTDOWN )
            {
                reAccept();
            }
            else
            {
                checkWithExit(-1,"error on standrad rcv");
            }

        }
        else
        {
            gotNormalMessage=true;
        }
    }
    int iCk=checkToken();
    #ifdef DEBUG_PRINT
    printf("\n\n\n\nReceived (CheckToken()=%d):\n",iCk);
    printToken(m_Token);
    #endif
    return m_Token.msg;
}

void TcpConnector::myInit(string sMyAddr,int iMyPort)
{

    m_fMyInit=true;

    if(sMyAddr.compare("")==0)
        m_sMyAddr="127.0.0.1";
    else
        m_sMyAddr=sMyAddr;

    m_iMyPort=iMyPort;


    m_TokenRingSerwer= checkWithExit(socket(AF_INET,SOCK_STREAM,0),"TokenRing Socket to be serwer creation");

    memset(&m_TokenRingMyAddr,0,sizeof(m_TokenRingMyAddr));
    m_TokenRingMyAddr.sin_family = AF_INET;
    m_TokenRingMyAddr.sin_addr.s_addr = inet_addr(m_sMyAddr.c_str());
    m_TokenRingMyAddr.sin_port = htons(m_iMyPort);
    checkWithExit(bind(m_TokenRingSerwer,(sockaddr*) &m_TokenRingMyAddr,sizeof(m_TokenRingMyAddr)),"bind ring socket");
    checkWithExit(listen(m_TokenRingSerwer,0),"listen ring socket");
    m_TokenRingListeningSocket=checkWithExit(accept(m_TokenRingSerwer,nullptr,0),"on accept");

    bListeningSocketConnected=true;
}

void TcpConnector::targetInit(string sTargetAddr,int iTargetPort)
{
    m_Sem = CreateSemaphore(
    NULL,           // default security attributes
    1,  // initial count
    1,  // maximum count
    NULL);          // unnamed semaphore

    if(m_Sem==NULL)
    {
        checkWithExit(-1,"Failed to create semaphore");
    }

    if(sTargetAddr.compare("")==0)
        m_sTargetAddr="127.0.0.1";
    else
        m_sTargetAddr=sTargetAddr;

    m_iTargetPort=iTargetPort;
    //init target address;
    memset(&m_TokenRingTargetAddr,0,sizeof(m_TokenRingTargetAddr));
    m_TokenRingTargetAddr.sin_family = AF_INET;
    m_TokenRingTargetAddr.sin_addr.s_addr = inet_addr(m_sTargetAddr.c_str());
    m_TokenRingTargetAddr.sin_port = htons(m_iTargetPort);

    m_TokenRingSendingSocket=checkWithExit(socket(AF_INET,SOCK_STREAM,0),"TokenRing SocketSending creation");
    checkWithExit(connect(m_TokenRingSendingSocket,(sockaddr*) &m_TokenRingTargetAddr,sizeof(m_TokenRingTargetAddr)),"on connect");
}












int TcpConnector::reAccept()
{
    bListeningSocketConnected=false;
    #ifdef DEBUG_PRINT
    printf("in reaccpet, connection lost\n");
    #endif // DEBUG_PRINT
    while(!bListeningSocketConnected)
    {
        Sleep(10);
    }
    return 0;
}



void TcpConnector::getSem(string where)
{
    #ifdef DEBUG_PRINT
    printf("-----taking semaphore %s-----\n",where.c_str());
    #endif // DEBUG_PRINT
    DWORD dwRet=WaitForSingleObject(m_Sem,0);
    if(dwRet==WAIT_TIMEOUT)
    {
        printf("failed on semaphore get (in %s) with error%d\n",where.c_str(),GetLastError());
        exit(1);
    }
}

void TcpConnector::releaseSem()
{
    #ifdef DEBUG_PRINT
    printf("-----releasing semaphore-----\n");
    #endif // DEBUG_PRINT
    DWORD dwRet=ReleaseSemaphore(m_Sem,1,NULL);
    if(dwRet==0)
    {
        printf("failed on releasing semaphore error:%d",(int)GetLastError());
    }
}



void TcpConnector::monitorListening()
{
    #ifdef DEBUG_PRINT
    printf("monitoring lsitening socket active\n");
    #endif // DEBUG_PRINT
    int newConnectionSocket;
    while(true)
    {
        #ifdef DEBUG_PRINT
        printf("monitorListening awaiting connection\n");
        #endif // DEBUG_PRINT
        newConnectionSocket=checkWithExit(accept(m_TokenRingSerwer,nullptr,0),"on accept in monitor thread");
        getSem("monitorListening");

        #ifdef DEBUG_PRINT
        printf("monitor listening got connection\n");
        #endif // DEBUG_PRINT
        Token recvToken;
        checkWithExit(recv(newConnectionSocket,(char*)&recvToken,sizeof(recvToken),0),"monotor listening recive token error");
        if(!((recvToken.flags & TOKEN_CTRL_REQ)||(recvToken.flags & TOKEN_NEW_FRIEND)))
        {
                printf("\n\nToken Type=%d, excpected type =%d\n",recvToken.flags,TOKEN_CTRL_REQ);
                checkWithExit(-1,"recived as control token not Cotnrol Token REQ");
        }


        if(recvToken.flags & TOKEN_NEW_FRIEND)
        {
            #ifdef DEBUG_PRINT
            printf("restoring lost connection, got token:\n");
            printControlToken(m_Token);
            #endif // DEBUG_PRINT
            releaseSem();
            while(bListeningSocketConnected)
            {
                ;
            }
            getSem("after waitign fo bListeningSocketConnected");
            checkWithExit(closesocket(m_TokenRingListeningSocket),"on closing socket "); // shutdown not need since connection lost
            m_TokenRingListeningSocket=newConnectionSocket; // restore new transmission as
            bListeningSocketConnected=true;
        }
        else // asummuing token is TOKEN_CTRL_REQ
        {
            #ifdef DEBUG_PRINT
            printf("perparing to reply to CTRL_REQ, got token:\n");
            printControlToken(m_Token);
            #endif // DEBUG_PRINT
            Token ResponseToken=generateCtrlToken(m_sTargetAddr.c_str(),m_iTargetPort,TOKEN_CTRL_ACK);


            struct sockaddr_in new_next_addr;
            ControlMessage* recvCotnrolMsg= (ControlMessage*) &(recvToken.msg);


            new_next_addr.sin_family = AF_INET;
            new_next_addr.sin_addr.s_addr = inet_addr(recvCotnrolMsg->sAddr);
            new_next_addr.sin_port = htons(recvCotnrolMsg->iPortNr);


            #ifdef DEBUG_PRINT
                printf("trying to send ACKTToken\n");
                printControlToken(ResponseToken);
            #endif // DEBUG_PRINT
            checkWithExit(send(newConnectionSocket,(char*) &ResponseToken,sizeof(ResponseToken),0),"on send TOKEN_CTRL_ACK");

            checkWithExit(shutdown(m_TokenRingSendingSocket,SD_BOTH),"on shutdown sedning socket");
            closesocket(m_TokenRingSendingSocket);

            m_TokenRingSendingSocket=newConnectionSocket;
            // we do a switcheru, and change conotrl connection into our enxt outgoing connection
            m_TokenRingTargetAddr=new_next_addr;
            m_sTargetAddr=recvCotnrolMsg->sAddr;
            m_iTargetPort=recvCotnrolMsg->iPortNr;
        }
        releaseSem();
        #ifdef DEBUG_PRINT
        printf("after release sem\n");
        #endif // DEBUG_PRINT
    } // end while
}


int TcpConnector::connectToExisting(string sMyAddr,int iMyPort,string sRingConnectorAddr, int iRingConnectorPort)
{

    #ifdef DEBUG_PRINT
    printf("in connectToExisitng\n with args %s,%d,%s,%d\n",sMyAddr.c_str(),iMyPort,sRingConnectorAddr.c_str(),iRingConnectorPort);
    #endif // DEBUG_PRINT
    m_Sem = CreateSemaphore(
    NULL,           // default security attributes
    1,  // initial count
    1,  // maximum count
    NULL);          // unnamed semaphore

    if(m_Sem==NULL)
    {
        checkWithExit(-1,"Failed to create semaphore");
    }

    if(sMyAddr.compare("")==0)
        m_sMyAddr="127.0.0.1";
    else
        m_sMyAddr=sMyAddr;

    m_iMyPort=iMyPort;


    ///////////// Init SERWER SOCKET
    m_TokenRingSerwer= checkWithExit(socket(AF_INET,SOCK_STREAM,0),"TokenRing Socket to be serwer creation");
    memset(&m_TokenRingMyAddr,0,sizeof(m_TokenRingMyAddr));
    m_TokenRingMyAddr.sin_family = AF_INET;
    m_TokenRingMyAddr.sin_addr.s_addr = inet_addr(m_sMyAddr.c_str());
    m_TokenRingMyAddr.sin_port = htons(m_iMyPort);
    checkWithExit(bind(m_TokenRingSerwer,(sockaddr*) &m_TokenRingMyAddr,sizeof(m_TokenRingMyAddr)),"bind ring socket");
    checkWithExit(listen(m_TokenRingSerwer,0),"listen ring socket");
    ///////////// INIT AND CONNECT LISTENING SOCKET
    m_TokenRingListeningSocket= checkWithExit(socket(AF_INET,SOCK_STREAM,0),"TokenRing Socket to be listening creation");

    struct sockaddr_in ringConnectorAddres;
    memset(&ringConnectorAddres,0,sizeof(ringConnectorAddres));
    ringConnectorAddres.sin_family = AF_INET;
    ringConnectorAddres.sin_addr.s_addr = inet_addr(sRingConnectorAddr.c_str());
    ringConnectorAddres.sin_port = htons(iRingConnectorPort);


    int iRet=-1;
    while(iRet<0)
    {
        #ifdef DEBUG_PRINT
        printf("trying to connect to target server socket");
        #endif // DEBUG_PRINT
        iRet=connect(m_TokenRingListeningSocket,(sockaddr*) &ringConnectorAddres,sizeof(ringConnectorAddres));
        if(iRet<0 && WSAETIMEDOUT!=WSAGetLastError())
        {
            checkWithExit(-1,"connecting to target Serwer socket");
        }
    }

    m_Token=generateCtrlToken(sMyAddr.c_str(),iMyPort,TOKEN_CTRL_REQ);
    #ifdef DEBUG_PRINT
    printf("\n\nTrying to send this CTRL_REQ token:\n");
    printControlToken(m_Token);
    #endif // DEBUG_PRINT

    checkWithExit(send(m_TokenRingListeningSocket,(char*)&m_Token,sizeof(m_Token),0),"on sending CTRL_REQ token");
    checkWithExit(recv(m_TokenRingListeningSocket,(char*)&m_Token,sizeof(m_Token),0),"on receving CTRL_ACK token");
    #ifdef DEBUG_PRINT
    printf("\nRecived this  CTRL_ACK token:\n");
    printControlToken(m_Token);
    #endif // DEBUG_PRINT
    if(!(m_Token.flags & TOKEN_CTRL_ACK))
    {
        checkWithExit(-1,"Recivied From CTRL_REQ not CTRL_ACK token");
    }

    ControlMessage*  ctrlMsg= (ControlMessage*) &(m_Token.msg);
    m_sTargetAddr=ctrlMsg->sAddr;
    m_iTargetPort=ctrlMsg->iPortNr;

    memset(&m_TokenRingTargetAddr,0,sizeof(m_TokenRingTargetAddr));
    m_TokenRingTargetAddr.sin_family = AF_INET;
    m_TokenRingTargetAddr.sin_addr.s_addr = inet_addr(m_sTargetAddr.c_str());
    m_TokenRingTargetAddr.sin_port = htons(m_iTargetPort);

    m_TokenRingSendingSocket=checkWithExit(socket(AF_INET,SOCK_STREAM,0),"m_TokenRingSendingSocket creation");


    iRet=-1;
    while(iRet<0)
    {
        #ifdef DEBUG_PRINT
        printf("trying to connect to actual next hop Listening  socket");
        #endif // DEBUG_PRINT
        iRet=connect(m_TokenRingSendingSocket,(sockaddr*) &m_TokenRingTargetAddr,sizeof(m_TokenRingTargetAddr));
        if(iRet<0 && WSAETIMEDOUT!=WSAGetLastError())
        {
            checkWithExit(-1,"connecting to target Serwer socket");
        }
    }

    m_Token= generateCtrlToken("",0,TOKEN_NEW_FRIEND);

    checkWithExit(send(m_TokenRingSendingSocket,(char*) &m_Token,sizeof(m_Token),0),"sending inforamtion of being new neighbour");


    bListeningSocketConnected=true;
    m_ready=true;
    m_fMyInit=true;
    printf("succesfully connected to ring \n");


    runMonitorListening();
    return 0;
}




void TcpConnector::runMonitorListening()
{
    CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            runMonitorListening_Func,       // thread function name
            this,          // argument to thread function
            0,                      // use default creation flags
            &m_MonitorThreadId);   // returns the thread identifier
}

DWORD WINAPI runMonitorListening_Func(LPVOID lpParam)
{
    TcpConnector* conn=(TcpConnector*) lpParam;
    conn->monitorListening();
    return 0;
}



















