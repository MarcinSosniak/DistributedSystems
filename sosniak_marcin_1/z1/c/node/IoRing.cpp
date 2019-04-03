#include "IoRing.h"



TwoThreadSafeQue::TwoThreadSafeQue(int i)
{
    m_msgBuff = new Message[i];
    m_iSize=i;
    m_iReadFrom=0;
    m_iWriteTo=0;
}

TwoThreadSafeQue::~TwoThreadSafeQue()
{
    delete m_msgBuff;
}
bool TwoThreadSafeQue::push(Message msg)
{
    //printf("push write=%d ; read=%d\n",m_iWriteTo,m_iReadFrom);
    if(m_iWriteTo==m_iReadFrom-1 || (m_iWriteTo==m_iSize-1 &&m_iReadFrom==0))
    {

        return false;
    }


    m_msgBuff[m_iWriteTo]=msg;
    if(m_iWriteTo==m_iSize-1)
        m_iWriteTo=0;
    else
        m_iWriteTo++;
    return true;

}
bool TwoThreadSafeQue::pop(Message& msg)
{
    //printf("in POp\n");
    if(m_iWriteTo==m_iReadFrom)
        return false;
    //printf("in Pop Later\n, m_iReadFrom=");
    msg=m_msgBuff[m_iReadFrom];
    //printf("after read\n");
    if(m_iReadFrom==m_iSize-1)
        m_iReadFrom=0;
    else
        m_iReadFrom++;
    return true;
}


IoRing::IoRing(string sMyName, char type,bool fSuper)
{
    #ifdef DEBUG_PRINT
    printf("ioring created with %s,%d,%d\n",sMyName.c_str(),type,fSuper);
    #endif // DEBUG_PRINT

    m_que= new TwoThreadSafeQue(20);
    m_fSuper=fSuper;
    if(sMyName.length()<=NAME_LENGTH)
        strcpy(m_scMyName,sMyName.c_str());
    else
    {
        char strBuff[100];
        strcpy(strBuff,sMyName.c_str());
        strBuff[NAME_LENGTH]='\0';
        strcpy(m_scMyName,strBuff);
    }
    if(type== TYPE_TCP)
    {
        m_myConnector=(AbstractConnector*) new TcpConnector;
    }
    else
    {
        m_myConnector=(AbstractConnector*) new UdpConnector;
    }

    if(fSuper)
        m_myConnector->setSuper();
}
IoRing::~IoRing()
{
    delete m_myConnector;
    delete m_que;
}

int IoRing::initAsPartOfRing(int iMyPort,string sNextHopAddr, int iNextHopPort)
{
    printf("Initializing as part of ring\n");
    m_myConnector->init(string("127.0.0.1"),iMyPort, sNextHopAddr, iNextHopPort);
    memset((void*) &m_tmpMessage,0,sizeof(m_tmpMessage));
    //Sleep(1000);
    if(m_fSuper)
    {
        m_myConnector->sendMSG(m_tmpMessage);
    }

    runAsynchronous();
    return 0;
}

int IoRing::initFromOutSideOfRing(int iMyPort,string sRingConnectorAddr, int iRingConnectorPort)
{
    printf("Initializing from outside of ring of ring\n");
       m_myConnector->connectToExisting(string("127.0.0.1"),iMyPort,sRingConnectorAddr,iRingConnectorPort);
       runAsynchronous();
}
void IoRing::runAsynchronous()
{
    CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            function_toStartAsynchrnousIoRing,       // thread function name
            this,          // argument to thread function
            0,                      // use default creation flags
            &m_asynchrThreadId);   // returns the thread identifier
}

DWORD WINAPI function_toStartAsynchrnousIoRing(LPVOID lpParam)
{
    IoRing* ring=(IoRing*) lpParam;
    ring->run();
}


bool fFreeToken(Message &msg)
{
    if(strcmp(msg.sTo,"")==0)
        return true;
    else
        return false;
}



bool IoRing::fToMeWithPrint(Message &msg)
{
    if(strcmp(m_tmpMessage.sTo,m_scMyName)==0)
    {
        printMessage(msg);
        return true;
    }
    else
    {
        return false;
    }
}





void IoRing::run()
{
    Sleep(2000);
    printf("IoRing asynchronous run Ready\n");
    bool lastSend=false;
    while(true)
    {
        //printf("-------before recieve------------\n");
        m_tmpMessage=m_myConnector->recieve();
        //printf("-------got message------------\n");
        m_logger.log(string(m_scMyName));

        Sleep(1000);
        //printf("-------after sleep-------\n");
        if(fFreeToken(m_tmpMessage) || fToMeWithPrint(m_tmpMessage))
        {
            //printf("noticed free token");
            if(lastSend || !m_que->pop(m_tmpMessage))
            {
                //printf("in standard reSend()\n");
                m_myConnector->reSend();
                lastSend=false;
            }
            else
            {
                //printf("in standard reSend() the pother option\n");
                m_myConnector->sendMSG(m_tmpMessage);
                lastSend=true;
            }
        }
        else // token is not for us nor free, we just pass it further
        {
            m_myConnector->reSend();
        }
        //printf("-------after ifs-------\n");
    }//end while
    printf("IoRing asynchronous run finished\n");
}

void IoRing::queSend(string strTo,string sMsg)
{
    Message toEnque;
    memset((void*)&toEnque,0,sizeof(toEnque));

    if(strTo.length()<=NAME_LENGTH)
        strcpy(toEnque.sTo,strTo.c_str());
    else
    {
        char strBuff[100];
        strcpy(strBuff,strTo.c_str());
        strBuff[NAME_LENGTH]='\0';
        strcpy(toEnque.sTo,strBuff);
    }

    strcpy(toEnque.sFrom,m_scMyName);


    if(sMsg.length()<MESSAGE_STR_SIZE-1)
        strcpy(toEnque.message,sMsg.c_str());
    else
    {
        char strBuff[1024];
        strcpy(strBuff,sMsg.c_str());
        strBuff[MESSAGE_STR_SIZE-1]='\0';
        strcpy(toEnque.message,strBuff);
    }
    if(!m_que->push(toEnque))
    {
        #ifdef DEBUG_PRINT
            printf("que push failed\n");
            #endif // DEBUG_PRINT
        while(!m_que->push(toEnque))
        {
            Sleep(200);
        }
    }




}






