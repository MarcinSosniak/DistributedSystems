#include "log.h"
#include "ringConnector.h"
#include "IoRing.h"

#define TEST_SUPER_PORT 2001
#define TEST_NOT_SUPER_PORT 2002
#define TEST_LATE_TO_PARTY_PORT 2003


testConnect(char* argv[],udpLogger logger)
{
    printf("in Test connect\n");
    Message msg;
    clearMessage(msg);
    UdpConnector conn= UdpConnector();
    printf("Trying confunction with args (%s,%d,%s,%d)\n",string("127.0.0.1").c_str(),atoi(argv[1]),string(argv[2]).c_str(),atoi(argv[3]));
    printf("connfunction :%d\n",conn.connectToExisting(string("127.0.0.1"),atoi(argv[1]),string(argv[2]),atoi(argv[3])));
    string sMyName="C";
    while( true)
    {
        msg=conn.recieve();
        logger.log(string("Paulinka jest Super from:")+sMyName);
        Sleep(4000);

        sprintf((msg.sFrom),"C");
        sprintf((msg.sTo),"B");
        sprintf((msg.message),"pong");
        conn.sendMSG(msg);
    }

}


testTcp(char* argv[],udpLogger logger)
{
    bool fSuper=false;
    string strMyName="";
    Message toSend;
    printf("in tcp test as:");
    if(strcmp(argv[1],"Y")==0)
    {
        printf("SUPER!\n");
        fSuper=true;
        sprintf((toSend.sFrom),"A");
        sprintf((toSend.sTo),"B");
        sprintf((toSend.message),"ping");
        strMyName="A";
    }
    else
    {
        sprintf((toSend.sFrom),"B");
        sprintf((toSend.sTo),"A");
        sprintf((toSend.message),"pong");
        strMyName="B";
        printf("SADFACE\n");
    }
    TcpConnector tpcC;
    if(fSuper)
    {
        tpcC.setSuper();
        tpcC.init("127.0.0.1",TEST_SUPER_PORT,"127.0.0.1",TEST_NOT_SUPER_PORT);
        tpcC.sendMSG(toSend);
    }
    else
    {
        tpcC.init("127.0.0.1",TEST_NOT_SUPER_PORT,"127.0.0.1",TEST_SUPER_PORT);
    }

    while(true)
    {
        tpcC.recieve();
        logger.log(string("Paulinka jest Super from:")+strMyName);
        tpcC.sendMSG(toSend);
        Sleep(3000);
    }


}


void testTcpConnectToExisting(char* argv[],udpLogger logger)
{
    string strMyName="C";
    Message toSend;
    printf("in tcp test as:%s\n",strMyName.c_str());
    sprintf((toSend.sFrom),"C");
    sprintf((toSend.sTo),"B");
    sprintf((toSend.message),"IT WORKS HAHAHAHHAHAHA!");

    TcpConnector tcpC;
    tcpC.connectToExisting(string("127.0.0.1"),TEST_LATE_TO_PARTY_PORT,string("127.0.0.1"),TEST_SUPER_PORT);
    while(true)
    {
        tcpC.recieve();
        logger.log(string("Paulinka jest Super from:")+strMyName);
        tcpC.sendMSG(toSend);
        Sleep(3000);
    }

    return;

}



/*

void testUDP5Args(char*argv,udpLogger logger)
{
    Message msg;
    clearMessage(msg);
    UdpConnector conn= UdpConnector();
    conn.init(string("127.0.0.1"),atoi(argv[1]),string(argv[2]),atoi(argv[3]));
    printf("init succesfull\n");

    string sMyName="";
    if(strcmp(argv[4],"Y")==0)
    {
        conn.setSuper();
        printf("super TRying to write to msg\n");
        sprintf((msg.sFrom),"A");
        sprintf((msg.sTo),"B");
        sprintf((msg.message),"ping");
        printf("super TRying to send\n");
        conn.sendMSG(msg);
        sMyName="A";
    }
    else
    {
        printf("not so super");
        sMyName="B";
    }
    while( true)
    {
        msg=conn.recieve();
        logger.log(string("Paulinka jest Super from:")+sMyName);


        if(strcmp(argv[4],"Y")==0)
        {
            Sleep(3000);
            sprintf((msg.sFrom),"A");
            sprintf((msg.sTo),"B");
            sprintf((msg.message),"ping");
            conn.sendMSG(msg);
        }
        else
        {
            Sleep(6000);
            sprintf((msg.sFrom),"B");
            sprintf((msg.sTo),"A");
            sprintf((msg.message),"pong");
            conn.sendMSG(msg);
        }
    }
}

*/

// node.exe A 2000 127.0.0.1 2001 U T
/*
ostantie moze byc T/N/C, T-ma token N- nie ma C- dolacza sie do atywnego reingu
przed ostanite moze nbuc T/U
*/
void mainRun(IoRing ring)
{
    Sleep(3000);
    while(true)
    {
        char buffName[1024];
        char buffMsg[2048];
        printf("to who you want to send a message?\n>");
        gets(buffName);
        printf("what do  you want to send?\n>");
        gets(buffMsg);
        ring.queSend(string(buffName),string(buffMsg));
    }
}






int main(int args,char* argv[])
{
    WSADATAInit();
    printf("args=%d\n",args);
    char type=TYPE_UDP;
    bool fSuper=false;
    bool fAddingToExistingRing=false;


    if(args<7)
    {
        printf("insufficent arguments, should look like 'node.exe A 2000 127.0.0.1 2001 Y T'\n");
        return 0;
    }
    if(strcmp(argv[6],"C")==0)
    {
        fAddingToExistingRing=true;
    }
    if(strcmp(argv[6],"T")==0)
    {
        fSuper=true;
    }

    if(strcmp(argv[5],"T")==0)
    {
        type=TYPE_TCP;
    }

    IoRing ring=IoRing(string(argv[1]),  type, fSuper);

    if(fAddingToExistingRing)
    {
        ring.initFromOutSideOfRing(atoi(argv[2]),string(argv[3]), atoi(argv[4]));
    }
    else
    {
        ring.initAsPartOfRing(atoi(argv[2]),string(argv[3]), atoi(argv[4]));
    }

    mainRun(ring);


    printf("somehow main runned out o.0\n");

}
