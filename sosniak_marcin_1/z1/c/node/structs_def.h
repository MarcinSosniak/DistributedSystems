#ifndef STRUCTS_DEF_H_INCLUDED
#define STRUCTS_DEF_H_INCLUDED


#define MESSAGE_STR_SIZE 256
#define NAME_LENGTH 8
#define MAX_ADDR_LEN 16 // inlcudes '/0'
// MAX_ADDR_LEN = sizeof("255.255.255.255")


#include <stdio.h>
#include <string.h>


/*
flags determinate type
no flags Depic regular message
TOKEN_DESTR destruction Token
TOKEN_CTRL_REQ asks for getting into ring
TOKEN_CTRL_ACK ackonwldges gettign itno ring, sens your next target data
TOKEN_CTRL_NACK denies getting into ring (as version 1.3 not used)
*/
#define TOKEN_DESTR 0b00000001
#define TOKEN_CTRL_ACK 0b00000100
#define TOKEN_CTRL_REQ 0b00001000
#define TOKEN_NEW_FRIEND 0b00010000




typedef struct
{
    char sTo[NAME_LENGTH+1];
    char sFrom[NAME_LENGTH+1];
    char message[MESSAGE_STR_SIZE];
} Message;

typedef struct
{
    char sAddr[MAX_ADDR_LEN];
    int iPortNr;
    char sPadding[sizeof(Message)- sizeof(int)-MAX_ADDR_LEN];
} ControlMessage;




typedef struct
{
    unsigned char  flags;
    unsigned short iMessageId;
    unsigned short iTokenSecurity;
    Message msg;
} Token;

void printMessage(Message &msg);

void clearMessage(Message &msg);

//Token generateCtrlToken(const char* addr, int iPort ,char TOKEN_FLAG);
void printToken(Token&);

void printControlToken(Token&);

void printControlMessage(ControlMessage&);

void bytePrint(void*,int);



#endif // STRUCTS_DEF_H_INCLUDED
