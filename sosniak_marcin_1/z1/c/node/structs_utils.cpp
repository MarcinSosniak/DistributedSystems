#include "structs_def.h"

void printMessage(Message &msg)
{
    printf("From:%s\nTo:  %s\n%s\n\n",msg.sFrom,msg.sTo,msg.message);
}

void clearMessage(Message &msg)
{
    memset((void*) &msg,0,sizeof(msg) );
}


/*
Token generateCtrlToken(const char* addr, int iPort ,unsigned char TOKEN_FLAG)
{
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
*/
void printToken(Token& t)
{
    if(t.flags==TOKEN_DESTR)
        printf("Desturciton Token(%d)\n",t.flags);
    else if(t.flags==TOKEN_CTRL_ACK)
        printf("Control ACK Token(%d)\n",t.flags);
    else if(t.flags==TOKEN_CTRL_REQ)
        printf("Control REQ Token(%d)\n",t.flags);
    else if(t.flags==0)
        printf("Message Token(%d)\n",t.flags);
    else
        printf("Uknown Token with flags=%d\n",t.flags);
    printf("messageId=%d\nSecurityId=%d\n",t.iMessageId,t.iTokenSecurity);
    printMessage(t.msg);
    printf("\n\n");
}


void printControlToken(Token&t)
{
    if(t.flags==TOKEN_DESTR)
        printf("Desturciton Token(%d)\n",t.flags);
    else if(t.flags==TOKEN_CTRL_ACK)
        printf("Control ACK Token(%d)\n",t.flags);
    else if(t.flags==TOKEN_CTRL_REQ)
        printf("Control REQ Token(%d)\n",t.flags);
    else if(t.flags==0)
        printf("Message Token(%d)\n",t.flags);
    else
        printf("Uknown Token with flags=%d\n",t.flags);
    printControlMessage(*((ControlMessage*)(&(t.msg))));
}


void printControlMessage(ControlMessage& cm)
{
    printf("Addr=%s, Port=%d\n",cm.sAddr,cm.iPortNr);
}



void bytePrint(void* target,int iCount)
{
    char* c=(char*)target;
    for (int i=0; i< iCount;i++)
    {
        printf("%3d/",*(c+i));
        if((i+1)% 10 == 0)
            printf("\n");
    }
    printf("\n");
}













